#include "position.h"
#include <algorithm>

Position::Position(Stone *cells) {
	if (cells) for (int i = 0; i < 32; ++ i)
		_cells[i] = cells[i];
}

Role Position::color(Cell cell) const {
	if (!cell.valid())
		return Role::None;
	return _cells[cell.index()].color;
}

bool Position::king(Cell cell) const {
	if (!cell.valid())
		return false;
	return _cells[cell.index()].king;
}

bool Position::ghost(Cell cell) const {
	if (!cell.valid())
		return false;
	return _cells[cell.index()].ghost;
}

std::vector<Cell> Position::stock(Role role) const {
	std::vector<Cell> answer;
	for (int i = 0; i < 32; ++ i)
		if (!role.valid() || _cells[i].color == role)
			answer.push_back(Cell::fromIndex(i));
	return answer;
}

Position::Stone Position::at(Cell cell) const {
	if (!cell.valid())
		return Stone();
	return _cells[cell.index()];
}

Position::Stone* Position::access(Cell cell) {
	if (!cell.valid())
		return nullptr;
	return &_cells[cell.index()];
}

bool Position::move(Cell from, Cell to) {
	if (!_cells[from.index()].color.valid() || _cells[to.index()].color.valid())
		return false;
	_cells[to.index()].color = _cells[from.index()].color;
	_cells[to.index()].king = _cells[from.index()].king;
	_cells[from.index()].color = Role::None;
	return true;
}

bool Position::promote(Cell cell) {
	if (!_cells[cell.index()].color.valid())
		return false;
	_cells[cell.index()].king = true;
	return true;
}

bool Position::kill(Cell cell) {
	if (!_cells[cell.index()].color.valid())
		return false;
	_cells[cell.index()].ghost = true;
	return true;
}

void Position::removeGhosts() {
	for (int i = 0; i < 32; ++ i)
		if (_cells[i].ghost) {
			_cells[i].ghost = false;
			_cells[i].color = Role::None;
		}
}

Position::Motion Position::accepts(Cell thru, Direction direction) const {
	if (!thru.valid())
		return Block;
	Role color = _cells[thru.index()].color;
	bool king = _cells[thru.index()].king;
	return accepts(thru, direction, color, king);
}

bool Position::captures(Cell thru, Direction direction) const {
	if (!thru.valid())
		return false;
	Role color = _cells[thru.index()].color;
	bool king = _cells[thru.index()].king;
	return captures(thru, direction, color, king);
}

Position::Motion Position::accepts(Cell thru, Direction direction, Role color, bool king) const {
	if (!thru.valid() || !direction.valid() || !color.valid())
		return Block;
	Cell next = thru.neighbour(direction);
	if (!next.valid())
		return Block;
	Stone neighbour = _cells[next.index()];
	if (neighbour.color == color || neighbour.ghost)
		return Block;
	if (!neighbour.color.valid())
		return king ? Slow : (direction.valid(color) ? Slow : Block);
	if (neighbour.color == color.opposite()) {
		next = next.neighbour(direction);
		if (next.valid() && !_cells[next.index()].color.valid())
			return Capture;
	}
	return Block;
}

bool Position::captures(Cell thru, Direction direction, Role color, bool king) const {
	if (thru.promotion(color))
		king = true;
	while (true)
		switch (accepts(thru, direction, color, king)) {
		case Slow: if (king) {thru = thru.neighbour(direction); break;}
		case Block: return false;
		case Capture: return true;
		}
}
