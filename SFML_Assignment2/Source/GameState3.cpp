#include <Book/GameState3.hpp>
#include <Book/MusicPlayer.hpp>

#include <SFML/Graphics/RenderWindow.hpp>


GameState3::GameState3(StateStack& stack, Context context)
	: State(stack, context)
	, mWorld(*context.window, *context.fonts, *context.sounds, *context.player, *context.window)
	, mPlayer(*context.player)
{
	mPlayer.setMissionStatus(Player::MissionRunning);

	// Play game theme
	context.music->play(Music::MissionTheme);
}

void GameState3::draw()
{
	mWorld.draw();
}

bool GameState3::update(sf::Time dt)
{
	mWorld.update(dt);

	if (!mWorld.hasAlivePlayer())
	{
		mPlayer.setMissionStatus(Player::MissionFailure);
		requestStackPush(States::GameOver);
	}
	else if (mWorld.hasPlayerReachedEnd())
	{
		mPlayer.setMissionStatus(Player::MissionSuccess);
		requestStackPush(States::GameOver);
	}

	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleRealtimeInput(commands);

	return true;
}

bool GameState3::handleEvent(const sf::Event& event)
{
	// Game input handling
	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleEvent(event, commands);

	// Escape pressed, trigger the pause screen
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
		requestStackPush(States::Pause);

	return true;
}