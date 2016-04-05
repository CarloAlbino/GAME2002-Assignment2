#ifndef BOOK_PAUSESTATE_HPP
#define BOOK_PAUSESTATE_HPP

#include <Book/State.hpp>
#include <Book/Container.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window.hpp>


class PauseState : public State
{
	public:
							PauseState(StateStack& stack, Context context);
							~PauseState();

		virtual void		draw();
		virtual bool		update(sf::Time dt);
		virtual bool		handleEvent(const sf::Event& event);

	//[Carlo]
	private:
		bool				checkForButton(int buttonNum);
		void				pauseGame();
		void				returnToMainMenu();

	private:
		sf::Sprite			mBackgroundSprite;
		sf::Text			mPausedText;
		GUI::Container 		mGUIContainer;

		//[Carlo]
		sf::RenderWindow&			mWindow;
		sf::Sprite			mButton1;
		sf::Sprite			mButton2;
};

#endif // BOOK_PAUSESTATE_HPP