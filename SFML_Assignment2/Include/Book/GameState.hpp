#ifndef BOOK_GAMESTATE_HPP
#define BOOK_GAMESTATE_HPP

#include <Book/State.hpp>
#include <Book/World.hpp>
#include <Book/World2.hpp>
#include <Book/Player.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>


class GameState : public State
{
	public:
							GameState(StateStack& stack, Context context);

		virtual void		draw();
		virtual bool		update(sf::Time dt);
		virtual bool		handleEvent(const sf::Event& event);


	private:
		World				mWorld;
		World2				mWorld2;
		Player&				mPlayer;

		int					currentLevelNum;
};

#endif // BOOK_GAMESTATE_HPP