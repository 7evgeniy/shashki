#include "role.h"

const int Role::None;
const int Role::White;
const int Role::Black;

Role::Role() {
	_value = None;
}

Role::Role(int index) {
	if (index == White || index == Black)
		_value = index;
	else
		_value = None;
}

Role::operator int() const {
	return _value;
}

bool Role::valid() const {
	return _value != None;
}

Role Role::opposite() const {
	switch (_value) {
	case White: return Black;
	case Black: return White;
	default: return None;
	}
}

Role Role::next() const {
	switch (_value) {
	case White: return Black;
	case Black: return None;
	default: return White;
	}
}
