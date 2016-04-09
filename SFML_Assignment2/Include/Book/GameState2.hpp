#ifndef BOOK_GAMESTATE2_HPP
#define BOOK_GAMESTATE2_HPP

#include <Book/State.hpp>
#include <Book/World2.hpp>
#include <Book/Player.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>


class GameState2 : public State
{
public:
	GameState2(StateStack& stack, Context context);

	virtual void		draw();
	virtual bool		update(sf::Time dt);
	virtual bool		handleEvent(const sf::Event& event);


private:
	World2				mWorld;
	Player&				mPlayer;
};

#endif // BOOK_GAMESTATE2_HPP