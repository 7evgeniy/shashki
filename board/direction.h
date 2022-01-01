#ifndef DIRECTION_H
#define DIRECTION_H

#include <vector>
#include "role.h"

// Направление движения по шашечной доске. Пассивные данные.

class Direction {
public:
	class Turn {
	public:
		static const int None = -2;
		static const int GoLeft = -1;
		static const int GoStraight = 0;
		static const int GoRight = 1;
		static const int GoOpposite = 2;
		Turn (int v) : _value((v >= -2 && v <=2) ? v : None) {}
		operator int () {return _value;}
	private:
		int _value;
	};
public:
	Direction ();
	Direction (int index);
	operator int () const;
	Direction opposite () const;
	Turn derivator (Direction target) const;
	Direction derive (Turn turn) const;
	bool valid (Role color = Role::None) const;
public:
	static const std::vector<Direction>& enumerate ();
	static const int LeftForward = 4;
	static const int RightForward = 5;
	static const int RightBackward = 6;
	static const int LeftBackward = 7;
	static const int None = -1;
private:
	int _value;
private:
	struct StaticTable {
		std::vector<Direction> everything;
		StaticTable ();
	};
	static const StaticTable _table;
};

#endif
