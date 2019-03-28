#ifndef ROLE_H
#define ROLE_H

// Цвет шашки, цвет игрока. Пассивные данные.

class Role {
public:
	Role ();
	Role (int index);
	operator int () const;
	bool valid () const;
	Role opposite () const;
	Role next () const;
public:
	static const int White = 0;
	static const int Black = 1;
	static const int None = 2;
private:
	int _value;
};

#endif
