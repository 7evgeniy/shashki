#ifndef POSITION_H
#define POSITION_H

#include <vector>
#include "role.h"
#include "cell.h"

// Позиция на шашечной доске. Пассивные данные.

class Position {
public:
	struct Stone {
		Role color; bool king; bool ghost;
		Stone(Role c = Role::None, bool k = false, bool g = false) : color(c), king(k), ghost(g) {}
	};
	enum Motion {Block = 0, Slow, Capture};
	Position (Stone *cells = nullptr);

	Role color (Cell cell) const;
	bool king (Cell cell) const;
	bool ghost (Cell cell) const;
	Stone at (Cell cell) const;
	std::vector<Cell> stock (Role role) const;

	bool move (Cell from, Cell to);
	bool promote (Cell cell);
	bool kill (Cell cell);
	Stone* access (Cell cell);
	void removeGhosts ();

	Motion accepts (Cell thru, Direction direction) const;  // Тип прохода через поле.
	bool captures (Cell thru, Direction direction) const;   // Дорога содержит взятия.
	Motion accepts (Cell thru, Direction direction, Role color, bool king) const;
	bool captures (Cell thru, Direction direction, Role color, bool king) const;
private:
	Stone _cells[32];
};

#endif
