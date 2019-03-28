#ifndef CELL_H
#define CELL_H

// Поле на шашечной доске.

#include "direction.h"
#include <string>

class Cell {
public:
	Cell ();
	Cell (unsigned int file, unsigned int rank);
	static Cell fromIndex (unsigned int index);
	static Cell fromValue (unsigned int value);
	static Cell fromString (std::string str);
	unsigned int index () const;
	unsigned int value () const;
	unsigned int file () const;
	unsigned int rank () const;
	std::string str() const;
public:
	bool valid () const;
	bool capture () const;
	bool promotion (Role color) const;
	Cell neighbour (Direction direction) const;
	Direction connection (Cell other) const;
	void setCapture (bool flag);
	static const std::vector<Cell>& neighbours (Cell from = Cell());
public:
	static const unsigned int None;
	friend bool operator== (Cell fst, Cell snd);
	friend bool operator!= (Cell fst, Cell snd);
private:
	unsigned int _value;
	bool _capture;
private:
	struct StaticTable {
		std::vector<Cell> neighbour[32];
		std::vector<Cell> everything;
		StaticTable ();
	};
	static const StaticTable _table;
};

#endif
