#ifndef GAME_H
#define GAME_H

#include "../board/cell.h"
#include "../board/board_state.h"
#include "root.h"

class Game {
public:
	Game();
	int length(int head) const;
	BoardState at(int head, int depth) const;
	void cut(int head, int depth);
	BoardState evolve(int head, std::vector<Cell> action);
private:
	Root<BoardState> _game;
};

#endif
