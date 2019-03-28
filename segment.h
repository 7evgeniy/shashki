#ifndef SEGMENT_H
#define SEGMENT_H

#include "cell.h"
#include "direction.h"

class Position;

class Segment {
public:
	class Iterator {
	public:
		Cell operator* () const;
		Iterator& operator++ ();
	private:
		friend class Segment;
		friend bool operator== (Iterator it1, Iterator it2);
		friend bool operator!= (Iterator it1, Iterator it2);
		Iterator (const Position &position, Cell addr, Direction path);
	private:
		const Position &_position;
		Cell _addr;
		Direction _path;
		Role _color;
		bool _king;
	};
public:
	Segment (const Position& position, Cell start, Direction path);
	Iterator begin () const;
	Iterator end () const;
private:
	Iterator _begin;
	Iterator _end;
};

bool operator== (Segment::Iterator it1, Segment::Iterator it2);
bool operator!= (Segment::Iterator it1, Segment::Iterator it2);

#endif
