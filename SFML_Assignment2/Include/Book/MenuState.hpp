#ifndef BOOK_MENUSTATE_HPP
#define BOOK_MENUSTATE_HPP

#include <Book/State.hpp>
#include <Book/Container.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>


class MenuState : public State
{
	public:
								MenuState(StateStack& stack, Context context);

		virtual void			draw();
		virtual bool			update(sf::Time dt);
		virtual bool			handleEvent(const sf::Event& event);

	//[Carlo]
	private:
		bool					checkForButton(int buttonNum);
		void					startGame();
		void					goToSettings();
		void					quitGame();

	private:
		sf::Sprite				mBackgroundSprite;
		GUI::Container			mGUIContainer;

		//[Carlo]
		sf::RenderWindow&		mWindow;
};

#endif // BOOK_MENUSTATE_HPP
