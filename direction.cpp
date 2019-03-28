#include "direction.h"

const int Direction::None;
const int Direction::LeftForward;
const int Direction::RightForward;
const int Direction::RightBackward;
const int Direction::LeftBackward;

const int Direction::Turn::None;
const int Direction::Turn::GoStraight;
const int Direction::Turn::GoLeft;
const int Direction::Turn::GoRight;
const int Direction::Turn::GoOpposite;

const Direction::StaticTable Direction::_table;

Direction::StaticTable::StaticTable () {
	everything.assign({LeftForward, RightForward, RightBackward, LeftBackward});
}

Direction::Direction () {
	_value = None;
}

Direction::Direction (int index) {
	if (index >= 4 && index < 8)
		_value = index;
	else
		_value = None;
}

Direction Direction::opposite() const {
	switch (_value) {
	case LeftBackward: return RightForward;
	case LeftForward: return RightBackward;
	case RightBackward: return LeftForward;
	case RightForward: return LeftBackward;
	default: return None;
	}
}

Direction::Turn Direction::derivator(Direction target) const {
	if (_value == None || target._value == None)
		return Turn::None;
	if (_value == target._value)
		return Turn::GoStraight;
	if (4+(_value+1)%4 == target._value)
		return Turn::GoRight;
	if (_value == 4+(target._value+1)%4)
		return Turn::GoLeft;
	return Turn::GoOpposite;
}

Direction Direction::derive(Direction::Turn turn) const {
	if (_value == None || turn == Turn::None || turn == Turn::GoOpposite)
		return None;
	return 4+(_value+turn)%4;
}

bool Direction::valid(Role color) const {
	switch (color) {
	case Role::White: return *this == LeftForward || *this == RightForward;
	case Role::Black: return *this == LeftBackward || *this == RightBackward;
	default: return *this != None;
	}
}

Direction::operator int() const {return _value;}
const std::vector<Direction>& Direction::enumerate() {return _table.everything;}
