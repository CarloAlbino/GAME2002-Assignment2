#include <Book/GameState2.hpp>
#include <Book/MusicPlayer.hpp>

#include <SFML/Graphics/RenderWindow.hpp>


GameState2::GameState2(StateStack& stack, Context context)
	: State(stack, context)
	, mWorld(*context.window, *context.fonts, *context.sounds, *context.player, *context.window)
	, mPlayer(*context.player)
{
	mPlayer.setMissionStatus(Player::MissionRunning);

	// Play game theme
	context.music->play(Music::MissionTheme);
}

void GameState2::draw()
{
	mWorld.draw();
}

bool GameState2::update(sf::Time dt)
{
	mWorld.update(dt);

	if (!mWorld.hasAlivePlayer())
	{
		mPlayer.setMissionStatus(Player::MissionFailure);
		requestStackPush(States::GameOver);
	}
	else if (mWorld.hasPlayerReachedEnd())
	{
		//mPlayer.setMissionStatus(Player::MissionSuccess);
		requestStackPop();
		requestStackPush(States::Game3);
	}

	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleRealtimeInput(commands);

	return true;
}

bool GameState2::handleEvent(const sf::Event& event)
{
	// Game input handling
	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleEvent(event, commands);

	// Escape pressed, trigger the pause screen
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
		requestStackPush(States::Pause);

	return true;
}