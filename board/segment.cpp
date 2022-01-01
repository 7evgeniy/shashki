#include "segment.h"
#include "position.h"

Segment::Segment (const Position& position, Cell start, Direction path)
	: _begin(position, start, path), _end(position, Cell(), path) {}

Segment::Iterator::Iterator (const Position& position, Cell addr, Direction path)
	: _position(position), _addr(addr), _path(path) {
	_color = _position.color(_addr);
	_king = _position.king(_addr);
}

Segment::Iterator Segment::begin () const {return _begin;}
Segment::Iterator Segment::end () const {return _end;}
bool operator== (Segment::Iterator it1, Segment::Iterator it2) {return it1._addr == it2._addr;}
bool operator!= (Segment::Iterator it1, Segment::Iterator it2) {return it1._addr != it2._addr;}
Cell Segment::Iterator::operator* () const {return _addr;}

Segment::Iterator& Segment::Iterator::operator++ () {
	if (!_addr.valid())
		return *this;
	if (_addr.capture()) {
		_addr = _addr.neighbour(_path);
		return *this;
	}
	switch (_position.accepts(_addr, _path, _color, _king)) {
	case Position::Block: _addr = Cell(); break;
	case Position::Capture: _addr = _addr.neighbour(_path); _addr.setCapture(true); break;
	case Position::Slow: _addr = _addr.neighbour(_path); break;
	}
	return *this;
}
