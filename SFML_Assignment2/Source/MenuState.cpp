#include <Book/MenuState.hpp>
#include <Book/Button.hpp>
#include <Book/Utility.hpp>
#include <Book/MusicPlayer.hpp>
#include <Book/ResourceHolder.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>


MenuState::MenuState(StateStack& stack, Context context)
: State(stack, context)
, mGUIContainer()
, mWindow(*context.window)
{
	sf::Texture& texture = context.textures->get(Textures::TitleScreen);
	mBackgroundSprite.setTexture(texture);

	auto playButton = std::make_shared<GUI::Button>(context);
	playButton->setPosition(100, 300);
	playButton->setText("Play");
	playButton->setCallback([this] ()
	{
		startGame();
		//requestStackPop();
		//requestStackPush(States::Game);
	});

	auto settingsButton = std::make_shared<GUI::Button>(context);
	settingsButton->setPosition(100, 350);
	settingsButton->setText("Settings");
	settingsButton->setCallback([this] ()
	{
		goToSettings();
		//requestStackPush(States::Settings);
	});

	auto exitButton = std::make_shared<GUI::Button>(context);
	exitButton->setPosition(100, 400);
	exitButton->setText("Exit");
	exitButton->setCallback([this] ()
	{
		quitGame();
		//requestStackPop();
	});

	mGUIContainer.pack(playButton);
	mGUIContainer.pack(settingsButton);
	mGUIContainer.pack(exitButton);

	// Play menu theme
	context.music->play(Music::MenuTheme);
}

void MenuState::draw()
{
	sf::RenderWindow& window = *getContext().window;

	window.setView(window.getDefaultView());

	window.draw(mBackgroundSprite);
	window.draw(mGUIContainer);
}

bool MenuState::update(sf::Time)
{
	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	{
		// Return to game
		if (checkForButton(0))
			startGame();
		// Return to menu
		if (checkForButton(1))
			goToSettings();
		// Quit game
		if (checkForButton(2))
			quitGame();
	}
	else
	{
		for (int i = 0; i < 3; i++)
			checkForButton(i);
	}

	return true;
}

bool MenuState::handleEvent(const sf::Event& event)
{
	mGUIContainer.handleEvent(event);
	return false;
}

//[Carlo]

bool MenuState::checkForButton(int buttonNum)
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

void MenuState::startGame()
{
	requestStackPop();
	requestStackPush(States::Game);
}

void MenuState::goToSettings()
{
	requestStackPush(States::Settings);
}

void MenuState::quitGame()
{
	requestStackPop();
}
