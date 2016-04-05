#include <Book/PauseState.hpp>
#include <Book/Button.hpp>
#include <Book/Utility.hpp>
#include <Book/MusicPlayer.hpp>
#include <Book/ResourceHolder.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

#include <iostream>

PauseState::PauseState(StateStack& stack, Context context)
: State(stack, context)
, mBackgroundSprite()
, mPausedText()
, mGUIContainer()
, mWindow(*context.window)
{
	sf::Font& font = context.fonts->get(Fonts::Main);
	sf::Vector2f windowSize(context.window->getSize());

	mPausedText.setFont(font);
	mPausedText.setString("Game Paused");	
	mPausedText.setCharacterSize(70);
	centerOrigin(mPausedText);
	mPausedText.setPosition(0.5f * windowSize.x, 0.4f * windowSize.y);

	auto returnButton = std::make_shared<GUI::Button>(context);
	returnButton->setPosition(0.5f * windowSize.x - 100, 0.4f * windowSize.y + 75);
	returnButton->setText("Return");
	returnButton->setCallback([this] ()
	{
		pauseGame();
		//requestStackPop();
	});
	
	auto backToMenuButton = std::make_shared<GUI::Button>(context);
	backToMenuButton->setPosition(0.5f * windowSize.x - 100, 0.4f * windowSize.y + 125);
	backToMenuButton->setText("Back to menu");
	backToMenuButton->setCallback([this] ()
	{
		returnToMainMenu();
		//requestStateClear();
		//requestStackPush(States::Menu);
	});

	mGUIContainer.pack(returnButton);
	mGUIContainer.pack(backToMenuButton);

	getContext().music->setPaused(true);
}

PauseState::~PauseState()
{
	getContext().music->setPaused(false);
}

void PauseState::draw()
{
	sf::RenderWindow& window = *getContext().window;
	window.setView(window.getDefaultView());

	sf::RectangleShape backgroundShape;
	backgroundShape.setFillColor(sf::Color(0, 0, 0, 150));
	backgroundShape.setSize(window.getView().getSize());

	window.draw(backgroundShape);
	window.draw(mPausedText);
	window.draw(mGUIContainer);
}

bool PauseState::update(sf::Time)
{
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	{
		// Return to game
		if (checkForButton(0))
			pauseGame();
		// Return to menu
		if (checkForButton(1))
			returnToMainMenu();
	}
	else
	{
		checkForButton(0);
		checkForButton(1);
	}

	return false;
}

bool PauseState::handleEvent(const sf::Event& event)
{
	mGUIContainer.handleEvent(event);
	return false;
}

//[Carlo]

bool PauseState::checkForButton(int buttonNum)
{
	int mouseX = sf::Mouse::getPosition(mWindow).x;
	int mouseY = sf::Mouse::getPosition(mWindow).y;

	if (mouseX > mGUIContainer.getChild(buttonNum)->getPosition().x &&
		mouseX < mGUIContainer.getChild(buttonNum)->getPosition().x + 200.f &&
		mouseY > mGUIContainer.getChild(buttonNum)->getPosition().y &&
		mouseY < mGUIContainer.getChild(buttonNum)->getPosition().y + 50.f)
	{
		mGUIContainer.select(buttonNum);
		return true;
	}
	else
	{
		return false;
	}

}

void PauseState::pauseGame()
{
	requestStackPop();
}

void PauseState::returnToMainMenu()
{
	requestStateClear();
	requestStackPush(States::Menu);
}
