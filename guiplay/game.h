#ifndef GAME_H
#define GAME_H

#include "../board/cell.h"
#include "../board/board_state.h"
#include "root.h"

class Game {
public:
	Game();
	bool empty() const;
	bool inner(int head, int depth) const;
	int length(int head) const;
	BoardState at(int head, int depth) const;
	void fork(int head, int depth);
	BoardState cut(int head, int depth);
	BoardState evolve(int head, std::vector<Cell> action);
private:
	Root<BoardState>::Iterator find(int head, int depth) const;
	bool doesReach(Root<BoardState>::Iterator from, Root<BoardState>::Iterator to) const;
private:
	Root<BoardState> _game;
};

#endif
