#include "cell.h"
#include <cstdlib>

const unsigned int Cell::None = 80;
const Cell::StaticTable Cell::_table;

Cell::StaticTable::StaticTable() {
	int offset[4] = {+7, +9, -7, -9};
	for (int i = 0; i < 64; ++ i) {
		Cell from = fromValue(i);
		if (from.valid()) {
			for (int j = 0; j < 4; ++ j) {
				Cell to = fromValue(i+offset[j]);
				if (to.valid())
					neighbour[from.index()].push_back(to);
			}
			neighbour[from.index()].push_back(Cell());
		}
	}
	for (int i = 0; i < 33; ++ i)
		everything.push_back(fromIndex(i));
}

Cell Cell::fromValue(unsigned int value) {return Cell(value%8, value/8);}
unsigned int Cell::index() const {return _value/2;}
unsigned int Cell::value() const {return _value;}
unsigned int Cell::file() const {return _value%8;}
unsigned int Cell::rank() const {return _value/8;}

Cell Cell::fromIndex(unsigned int index) {
	Cell cell;
	if (index < 32)
		cell._value = index*2 + (index/4)%2;
	return cell;
}

Cell Cell::fromString(std::string str) {
	if (str.size() != 2)
		return Cell();
	int file = -1, rank = -1;
	if (std::isupper(str[0]))
		file = str[0]-'A';
	if (std::islower(str[0]))
		file = str[0]-'a';
	if (std::isdigit(str[1]))
		rank = str[1]-'1';
	return Cell(file, rank);
}

std::string Cell::str() const {
	if (!valid())
		return "None";
	std::string result;
	result.push_back(file()+'A');
	result.push_back(rank()+'1');
	return result;
}

Cell::Cell() : _value(None), _capture(false) {}

Cell::Cell(unsigned int file, unsigned int rank) : _capture(false) {
	_value = rank*8 + file;
	if (file >= 8)
		_value = None;
	if (rank >= 8)
		_value = None;
	if ((file+rank)%2)
		_value = None;
}

bool Cell::valid() const {return _value != None;}
bool Cell::capture() const {return _capture;}
void Cell::setCapture(bool flag) {_capture = flag;}
bool operator==(Cell fst, Cell snd) {return fst._value == snd._value;}
bool operator!=(Cell fst, Cell snd) {return fst._value != snd._value;}

bool Cell::promotion(Role color) const {
	if (!valid())
		return false;
	switch (color) {
	case Role::White: return _value >= 56;
	case Role::Black: return _value < 8;
	default: return false;
	}
}

Cell Cell::neighbour(Direction direction) const {
	switch (direction) {
	case Direction::LeftForward: return Cell::fromValue(_value+7);
	case Direction::RightForward: return Cell::fromValue(_value+9);
	case Direction::LeftBackward: return Cell::fromValue(_value-9);
	case Direction::RightBackward: return Cell::fromValue(_value-7);
	default: return Cell::fromValue(None);
	}
}

Direction Cell::connection(Cell other) const {
    switch (static_cast<int>(other._value) - static_cast<int>( _value)) {
	case +7: return Direction::LeftForward;
	case +9: return Direction::RightForward;
	case -9: return Direction::LeftBackward;
	case -7: return Direction::RightBackward;
	default: return Direction::None;
	}
}

const std::vector<Cell>& Cell::neighbours(Cell from) {
	return from.valid() ? _table.neighbour[from.index()] : _table.everything;
}
