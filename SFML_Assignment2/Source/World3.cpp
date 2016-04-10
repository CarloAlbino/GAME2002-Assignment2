#include <Book/World3.hpp>
#include <Book/Projectile.hpp>
#include <Book/Pickup.hpp>
#include <Book/Foreach.hpp>
#include <Book/TextNode.hpp>
#include <Book/ParticleNode.hpp>
#include <Book/SoundNode.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>
#include <stdlib.h>


World3::World3(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, Player& player, sf::Window& window)
	: mTarget(outputTarget)
	, mSceneTexture()
	, mWorldView(outputTarget.getDefaultView())
	, mTextures()
	, mFonts(fonts)
	, mSounds(sounds)
	, mSceneGraph()
	, mSceneLayers2()
	, mWorldBounds(0.f, 0.f, mWorldView.getSize().x, 5000.f)
	, mSpawnPosition(mWorldView.getSize().x / 2.f, mWorldBounds.height - mWorldView.getSize().y / 2.f)
	, mScrollSpeed(-100.f)
	, mPlayerAircraft(nullptr)
	, mEnemySpawnPoints()
	, mActiveEnemies()
	, mPlayer(player)
	, mArrivedAtPosition(false)
	, mCanGetMousePos(true)
	, mWindow(window)
{
	mSceneTexture.create(mTarget.getSize().x, mTarget.getSize().y);

	loadTextures();
	buildScene();

	// Prepare the view
	mWorldView.setCenter(mSpawnPosition);
}

void World3::update(sf::Time dt)
{
	// Scroll the world, reset player velocity
	mWorldView.move(0.f, mScrollSpeed * dt.asSeconds());
	mPlayerAircraft->setVelocity(0.f, 0.f);

	// Setup commands to destroy entities, and guide missiles
	destroyEntitiesOutsideView();
	guideMissiles();

	// Forward commands to scene graph, adapt velocity (scrolling, diagonal correction)
	while (!mCommandQueue.isEmpty())
		mSceneGraph.onCommand(mCommandQueue.pop(), dt);
	adaptPlayerVelocity();

	// Collision detection and response (may destroy entities)
	handleCollisions();

	// Remove all destroyed entities, create new ones
	mSceneGraph.removeWrecks();
	spawnEnemies();

	if (mPlayer.canMoveToMouse())
	{
		if (mCanGetMousePos)
		{
			mNextTarget = mTarget.mapPixelToCoords(sf::Mouse::getPosition(mWindow), mWorldView);
			mArrivedAtPosition = false;
			mCanGetMousePos = false;
		}
		//std::cout << "Moving to mouse" << std::endl;
		if (!mArrivedAtPosition)
		{
			// Moving to mouse position
			moveToMousePos();
		}
		else
		{
			mPlayer.setMoveToMouse(false);
			mCanGetMousePos = true;
		}
	}

	/*system("cls");
	std::cout << "mPlayer canMoveToMouse " << mPlayer.canMoveToMouse() << std::endl;
	std::cout << "World mArrivedAtPosition " << mArrivedAtPosition << std::endl;
	std::cout << "World mCanGetMousePos " << mCanGetMousePos << std::endl;*/

	// Regular update step, adapt position (correct if outside view)
	mSceneGraph.update(dt, mCommandQueue);
	adaptPlayerPosition();

	updateSounds();
}

void World3::draw()
{
	if (PostEffect::isSupported())
	{
		mSceneTexture.clear();
		mSceneTexture.setView(mWorldView);
		mSceneTexture.draw(mSceneGraph);
		mSceneTexture.display();
		mBloomEffect.apply(mSceneTexture, mTarget);
	}
	else
	{
		mTarget.setView(mWorldView);
		mTarget.draw(mSceneGraph);
	}
}

CommandQueue& World3::getCommandQueue()
{
	return mCommandQueue;
}

bool World3::hasAlivePlayer() const
{
	return !mPlayerAircraft->isMarkedForRemoval();
}

bool World3::hasPlayerReachedEnd() const
{
	return !mWorldBounds.contains(mPlayerAircraft->getPosition());
}

void World3::loadTextures()
{
	mTextures.load(Textures::Entities, "Media/Textures/Entities.png");
	mTextures.load(Textures::FireArc, "Media/Textures/FireArc.png");
	mTextures.load(Textures::Moss, "Media/Textures/Moss.png");
	mTextures.load(Textures::Explosion, "Media/Textures/Explosion.png");
	mTextures.load(Textures::Particle, "Media/Textures/Particle.png");
	mTextures.load(Textures::FinishLine, "Media/Textures/FinishLine.png");
}

void World3::adaptPlayerPosition()
{
	// Keep player's position inside the screen bounds, at least borderDistance units from the border
	sf::FloatRect viewBounds = getViewBounds();
	const float borderDistance = 40.f;

	sf::Vector2f position = mPlayerAircraft->getPosition();
	position.x = std::max(position.x, viewBounds.left + borderDistance);
	position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
	position.y = std::max(position.y, viewBounds.top + borderDistance);
	position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
	mPlayerAircraft->setPosition(position);
}

void World3::adaptPlayerVelocity()
{
	sf::Vector2f velocity = mPlayerAircraft->getVelocity();

	// If moving diagonally, reduce velocity (to have always same velocity)
	if (velocity.x != 0.f && velocity.y != 0.f)
		mPlayerAircraft->setVelocity(velocity / std::sqrt(2.f));

	// Add scrolling velocity
	mPlayerAircraft->accelerate(0.f, mScrollSpeed);
}

bool matchesCategories3(SceneNode::Pair& colliders, Category::Type type1, Category::Type type2)
{
	unsigned int category1 = colliders.first->getCategory();
	unsigned int category2 = colliders.second->getCategory();

	// Make sure first pair entry has category type1 and second has type2
	if (type1 & category1 && type2 & category2)
	{
		return true;
	}
	else if (type1 & category2 && type2 & category1)
	{
		std::swap(colliders.first, colliders.second);
		return true;
	}
	else
	{
		return false;
	}
}

void World3::handleCollisions()
{
	std::set<SceneNode::Pair> collisionPairs;
	mSceneGraph.checkSceneCollision(mSceneGraph, collisionPairs);

	FOREACH(SceneNode::Pair pair, collisionPairs)
	{
		if (matchesCategories3(pair, Category::PlayerAircraft, Category::EnemyAircraft))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& enemy = static_cast<Aircraft&>(*pair.second);

			// Collision: Player damage = enemy's remaining HP
			player.damage(enemy.getHitpoints());
			enemy.destroy();
		}

		else if (matchesCategories3(pair, Category::PlayerAircraft, Category::Pickup))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& pickup = static_cast<Pickup&>(*pair.second);

			// Apply pickup effect to player, destroy projectile
			pickup.apply(player);
			pickup.destroy();
			player.playLocalSound(mCommandQueue, SoundEffect::CollectPickup);
		}

		else if (matchesCategories3(pair, Category::EnemyAircraft, Category::AlliedProjectile)
			|| matchesCategories3(pair, Category::PlayerAircraft, Category::EnemyProjectile))
		{
			auto& aircraft = static_cast<Aircraft&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);

			// Apply projectile damage to aircraft, destroy projectile
			aircraft.damage(projectile.getDamage());
			projectile.destroy();
		}
	}
}

void World3::updateSounds()
{
	// Set listener's position to player position
	mSounds.setListenerPosition(mPlayerAircraft->getWorldPosition());

	// Remove unused sounds
	mSounds.removeStoppedSounds();
}

void World3::buildScene()
{
	// Initialize the different layers
	for (std::size_t i = 0; i < LayerCount; ++i)
	{
		Category::Type category = (i == LowerAir) ? Category::SceneAirLayer : Category::None;

		SceneNode::Ptr layer(new SceneNode(category));
		mSceneLayers2[i] = layer.get();

		mSceneGraph.attachChild(std::move(layer));
	}

	// Prepare the tiled background
	sf::Texture& mossTexture = mTextures.get(Textures::Moss);
	mossTexture.setRepeated(true);

	float viewHeight = mWorldView.getSize().y;
	sf::IntRect textureRect(mWorldBounds);
	textureRect.height += static_cast<int>(viewHeight);

	// Add the background sprite to the scene
	std::unique_ptr<SpriteNode> jungleSprite(new SpriteNode(mossTexture, textureRect));
	jungleSprite->setPosition(mWorldBounds.left, mWorldBounds.top - viewHeight);
	mSceneLayers2[Background]->attachChild(std::move(jungleSprite));

	// Add the finish line to the scene
	sf::Texture& finishTexture = mTextures.get(Textures::FinishLine);
	std::unique_ptr<SpriteNode> finishSprite(new SpriteNode(finishTexture));
	finishSprite->setPosition(0.f, -76.f);
	mSceneLayers2[Background]->attachChild(std::move(finishSprite));

	// Add particle node to the scene
	std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(Particle::Smoke, mTextures));
	mSceneLayers2[LowerAir]->attachChild(std::move(smokeNode));

	// Add propellant particle node to the scene
	std::unique_ptr<ParticleNode> propellantNode(new ParticleNode(Particle::Propellant, mTextures));
	mSceneLayers2[LowerAir]->attachChild(std::move(propellantNode));

	// Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(mSounds));
	mSceneGraph.attachChild(std::move(soundNode));

	// Add player's aircraft
	std::unique_ptr<Aircraft> player(new Aircraft(Aircraft::Eagle, mTextures, mFonts));
	mPlayerAircraft = player.get();
	mPlayerAircraft->setPosition(mSpawnPosition);
	mSceneLayers2[UpperAir]->attachChild(std::move(player));

	// Add enemy aircraft
	addEnemies();
}

void World3::addEnemies()
{
	// Add enemies to the spawn point container
	addEnemy(Aircraft::Raptor, -200.f, 1000.f);
	addEnemy(Aircraft::Raptor, 0.f, 1000.f);
	addEnemy(Aircraft::Raptor, 200.f, 1000.f);
	addEnemy(Aircraft::Raptor, 0.f, 1150.f);
	addEnemy(Aircraft::Avenger, 100.f, 1150.f);
	addEnemy(Aircraft::Avenger, -100.f, 1150.f);
	addEnemy(Aircraft::Avenger, -70.f, 1710.f);
	addEnemy(Aircraft::Avenger, 70.f, 1700.f);
	addEnemy(Aircraft::Avenger, 30.f, 1850.f);
	addEnemy(Aircraft::Raptor, 400.f, 2200.f);
	addEnemy(Aircraft::Raptor, -400.f, 2200.f);
	addEnemy(Aircraft::Raptor, 0.f, 2200.f);
	addEnemy(Aircraft::Raptor, 0.f, 2500.f);
	addEnemy(Aircraft::Avenger, 0.f, 2600.f);
	addEnemy(Aircraft::Avenger, 0.f, 2700.f);
	addEnemy(Aircraft::Raptor, 0.f, 3000.f);
	addEnemy(Aircraft::Raptor, 250.f, 3100.f);
	addEnemy(Aircraft::Raptor, -250.f, 3100.f);
	addEnemy(Aircraft::Avenger, 350.f, 3200.f);
	addEnemy(Aircraft::Avenger, -350.f, 3200.f);
	addEnemy(Aircraft::Raptor, 0.f, 3800.f);
	addEnemy(Aircraft::Avenger, -400.f, 4000.f);
	addEnemy(Aircraft::Avenger, 400.f, 4200.f);
	addEnemy(Aircraft::Raptor, 200.f, 4200.f);
	addEnemy(Aircraft::Raptor, 0.f, 4200.f);
	addEnemy(Aircraft::Raptor, 150.f, 4400.f);
	addEnemy(Aircraft::Raptor, 300.f, 4400.f);
	addEnemy(Aircraft::Raptor, 450.f, 4400.f);
	addEnemy(Aircraft::Raptor, -150.f, 4400.f);
	addEnemy(Aircraft::Raptor, -300.f, 4400.f);
	addEnemy(Aircraft::Raptor, -450.f, 4400.f);


	// Sort all enemies according to their y value, such that lower enemies are checked first for spawning
	std::sort(mEnemySpawnPoints.begin(), mEnemySpawnPoints.end(), [](SpawnPoint lhs, SpawnPoint rhs)
	{
		return lhs.y < rhs.y;
	});
}

void World3::addEnemy(Aircraft::Type type, float relX, float relY)
{
	SpawnPoint spawn(type, mSpawnPosition.x + relX, mSpawnPosition.y - relY);
	mEnemySpawnPoints.push_back(spawn);
}

void World3::spawnEnemies()
{
	// Spawn all enemies entering the view area (including distance) this frame
	while (!mEnemySpawnPoints.empty()
		&& mEnemySpawnPoints.back().y > getBattlefieldBounds().top)
	{
		SpawnPoint spawn = mEnemySpawnPoints.back();

		std::unique_ptr<Aircraft> enemy(new Aircraft(spawn.type, mTextures, mFonts));
		enemy->setPosition(spawn.x, spawn.y);
		enemy->setRotation(180.f);

		mSceneLayers2[UpperAir]->attachChild(std::move(enemy));

		// Enemy is spawned, remove from the list to spawn
		mEnemySpawnPoints.pop_back();
	}
}

void World3::destroyEntitiesOutsideView()
{
	Command command;
	command.category = Category::Projectile | Category::EnemyAircraft;
	command.action = derivedAction<Entity>([this](Entity& e, sf::Time)
	{
		if (!getBattlefieldBounds().intersects(e.getBoundingRect()))
			e.remove();
	});

	mCommandQueue.push(command);
}

void World3::guideMissiles()
{
	// Setup command that stores all enemies in mActiveEnemies
	Command enemyCollector;
	enemyCollector.category = Category::EnemyAircraft;
	enemyCollector.action = derivedAction<Aircraft>([this](Aircraft& enemy, sf::Time)
	{
		if (!enemy.isDestroyed())
			mActiveEnemies.push_back(&enemy);
	});

	// Setup command that guides all missiles to the enemy which is currently closest to the player
	Command missileGuider;
	missileGuider.category = Category::AlliedProjectile;
	missileGuider.action = derivedAction<Projectile>([this](Projectile& missile, sf::Time)
	{
		// Ignore unguided bullets
		if (!missile.isGuided())
			return;

		float minDistance = std::numeric_limits<float>::max();
		Aircraft* closestEnemy = nullptr;

		// Find closest enemy
		FOREACH(Aircraft* enemy, mActiveEnemies)
		{
			float enemyDistance = distance(missile, *enemy);

			if (enemyDistance < minDistance)
			{
				closestEnemy = enemy;
				minDistance = enemyDistance;
			}
		}

		if (closestEnemy)
			missile.guideTowards(closestEnemy->getWorldPosition());
	});

	// Push commands, reset active enemies
	mCommandQueue.push(enemyCollector);
	mCommandQueue.push(missileGuider);
	mActiveEnemies.clear();
}

sf::FloatRect World3::getViewBounds() const
{
	return sf::FloatRect(mWorldView.getCenter() - mWorldView.getSize() / 2.f, mWorldView.getSize());
}

sf::FloatRect World3::getBattlefieldBounds() const
{
	// Return view bounds + some area at top, where enemies spawn
	sf::FloatRect bounds = getViewBounds();
	bounds.top -= 100.f;
	bounds.height += 100.f;

	return bounds;
}

void World3::moveToMousePos()
{
	sf::Vector2f newPlayerPosition;
	newPlayerPosition = mNextTarget - mPlayerAircraft->getPosition();	// Find the vector from the player to the mouse position
	float newMagnitude = sqrt(pow(newPlayerPosition.x, 2) + pow(newPlayerPosition.y, 2));	// Find the length of that vector
	if (newMagnitude > 15.f){						// if the vector length is greater than 15 continue
		newPlayerPosition.x /= newMagnitude;		//
		newPlayerPosition.y /= newMagnitude;		// Normalize the vector
		// Set speed to 15
		newPlayerPosition *= 20.f;
		// Send new position to the airplane
		newPlayerPosition += mPlayerAircraft->getPosition();	// Add the new position to the player's current position
		mPlayerAircraft->setPosition(newPlayerPosition);		// Set the player's new position
	}
	else
	{
		mArrivedAtPosition = true;
	}
}
