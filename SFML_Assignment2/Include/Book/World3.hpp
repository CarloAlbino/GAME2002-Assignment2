#ifndef BOOK_WORLD3_HPP
#define BOOK_WORLD3_HPP

#include <Book/ResourceHolder.hpp>
#include <Book/ResourceIdentifiers.hpp>
#include <Book/SceneNode.hpp>
#include <Book/SpriteNode.hpp>
#include <Book/Aircraft.hpp>
#include <Book/CommandQueue.hpp>
#include <Book/Command.hpp>
#include <Book/BloomEffect.hpp>
#include <Book/SoundPlayer.hpp>
#include <Book/Player.hpp>

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <array>
#include <queue>


// Forward declaration
namespace sf
{
	class RenderTarget;
}

class World3 : private sf::NonCopyable
{
public:
	World3(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, Player& player, sf::Window& window);
	void								update(sf::Time dt);
	void								draw();

	CommandQueue&						getCommandQueue();

	bool 								hasAlivePlayer() const;
	bool 								hasPlayerReachedEnd() const;

	//[Carlo]
	void								moveToMousePos();


private:
	void								loadTextures();
	void								adaptPlayerPosition();
	void								adaptPlayerVelocity();
	void								handleCollisions();
	void								updateSounds();

	void								buildScene();
	void								addEnemies();
	void								addEnemy(Aircraft::Type type, float relX, float relY);
	void								spawnEnemies();
	void								destroyEntitiesOutsideView();
	void								guideMissiles();
	sf::FloatRect						getViewBounds() const;
	sf::FloatRect						getBattlefieldBounds() const;


private:
	enum Layer
	{
		Background,
		LowerAir,
		UpperAir,
		LayerCount
	};

	struct SpawnPoint
	{
		SpawnPoint(Aircraft::Type type, float x, float y)
			: type(type)
			, x(x)
			, y(y)
		{
		}

		Aircraft::Type type;
		float x;
		float y;
	};


private:
	sf::RenderTarget&					mTarget;
	sf::RenderTexture					mSceneTexture;
	sf::View							mWorldView;
	TextureHolder						mTextures;
	FontHolder&							mFonts;
	SoundPlayer&						mSounds;

	SceneNode							mSceneGraph;
	std::array<SceneNode*, LayerCount>	mSceneLayers2;
	CommandQueue						mCommandQueue;

	sf::FloatRect						mWorldBounds;
	sf::Vector2f						mSpawnPosition;
	float								mScrollSpeed;
	Aircraft*							mPlayerAircraft;

	std::vector<SpawnPoint>				mEnemySpawnPoints;
	std::vector<Aircraft*>				mActiveEnemies;

	BloomEffect							mBloomEffect;

	//[Carlo]
	Player&								mPlayer;
	bool								mArrivedAtPosition;
	bool								mCanGetMousePos;
	sf::Vector2f						mNextTarget;
	sf::Window&							mWindow;
};

#endif // BOOK_WORLD3_HPP
