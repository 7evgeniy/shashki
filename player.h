#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>

class Game;
class BoardState;

class Player : public QObject {
	Q_OBJECT
public:
	Player(Game *game) : _game(game) {}
	virtual ~Player() = default;
	virtual void activate(const BoardState &board) = 0;
	virtual void deactivate() {}
protected:
	Game *_game;
};

#endif
