#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include "role.h"
#include "game.h"

class BoardState;

class Player : public QObject {
	Q_OBJECT
public:
	Player() : _game(nullptr) {}
	virtual ~Player() = default;
	virtual void initialise(Game *game, Role color) {_game = game; _game->setPlayer(this, color);}
	virtual void activate(const BoardState &board) = 0;
	virtual void deactivate() {}
protected:
	Game *_game;
};

#endif
