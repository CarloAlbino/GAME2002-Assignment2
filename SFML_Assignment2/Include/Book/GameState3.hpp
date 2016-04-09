#ifndef BOOK_GAMESTATE3_HPP
#define BOOK_GAMESTATE3_HPP

#include <Book/State.hpp>
#include <Book/World3.hpp>
#include <Book/Player.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>


class GameState3 : public State
{
public:
	GameState3(StateStack& stack, Context context);

	virtual void		draw();
	virtual bool		update(sf::Time dt);
	virtual bool		handleEvent(const sf::Event& event);


private:
	World3				mWorld;
	Player&				mPlayer;
};

#endif // BOOK_GAMESTATE2_HPP