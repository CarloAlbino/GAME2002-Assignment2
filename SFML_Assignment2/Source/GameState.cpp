#include <Book/GameState.hpp>
#include <Book/MusicPlayer.hpp>

#include <SFML/Graphics/RenderWindow.hpp>


GameState::GameState(StateStack& stack, Context context)
: State(stack, context)
, mWorld2(*context.window, *context.fonts, *context.sounds, *context.player, *context.window)
, mWorld(*context.window, *context.fonts, *context.sounds, *context.player, *context.window)
, mPlayer(*context.player)
, currentLevelNum(0)
{
	mPlayer.setMissionStatus(Player::MissionRunning);

	// Play game theme
	context.music->play(Music::MissionTheme);
}

void GameState::draw()
{
	switch (currentLevelNum)
	{
	case 0:
		mWorld.draw();
		break;
	case 1:
		mWorld2.draw();
		break;
	default:
		break;
	}
}

bool GameState::update(sf::Time dt)
{
	switch (currentLevelNum)
	{
	case 0:
		mWorld.update(dt);

		if (!mWorld.hasAlivePlayer())
		{
			mPlayer.setMissionStatus(Player::MissionFailure);
			requestStackPush(States::GameOver);
		}
		else if (mWorld.hasPlayerReachedEnd())
		{
			//mPlayer.setMissionStatus(Player::MissionSuccess);
			//requestStackPush(States::GameOver);
			currentLevelNum++;
		}

		break;
	case 1:
		mWorld2.update(dt);

		if (!mWorld2.hasAlivePlayer())
		{
			mPlayer.setMissionStatus(Player::MissionFailure);
			requestStackPush(States::GameOver);
		}
		else if (mWorld2.hasPlayerReachedEnd())
		{
			mPlayer.setMissionStatus(Player::MissionSuccess);
			requestStackPush(States::GameOver);
		}

		break;
	default:
		break;
	}

	if (currentLevelNum = 0)
	{
		CommandQueue& commands = mWorld.getCommandQueue();
		mPlayer.handleRealtimeInput(commands);
	}
	else if (currentLevelNum = 1)
	{
		CommandQueue& commands2 = mWorld2.getCommandQueue();
		mPlayer.handleRealtimeInput(commands2);
	}

	return true;
}

bool GameState::handleEvent(const sf::Event& event)
{
	// Game input handling
	CommandQueue& commands = mWorld.getCommandQueue();
	mPlayer.handleEvent(event, commands);

	// Escape pressed, trigger the pause screen
	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
		requestStackPush(States::Pause);

	return true;
}