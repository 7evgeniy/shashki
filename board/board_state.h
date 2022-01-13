#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include "position.h"
#include "direction.h"
#include "cell.h"
#include "role.h"

/*
 * Автомат по изменению состояния доски, действующий по правилам русских шашек.
 *
 * Есть одна процедура, меняющая состояние автомата, и несколько процедур,
 * возвращающих только сведения о его текущем состоянии. Возможные действия
 * над автоматом:
 *   1) выбор шашки, которая будет ходить на следующем полуходу;
 *   2) перенос этой шашки на соседнее поле;
 *   3) останов (пустое поле).
 * Автомат сам решает, когда эти действия надо сопровождать уничтожением
 * шашек противоположного цвета (взятиями) или возведением шашки в дамки.
 */

class BoardState {
public:
	static bool apply (BoardState& board, std::vector<Cell> action);
	static BoardState initialBoard();
public:
	BoardState ();
	BoardState (const Position &position, Role start);
	bool control (Cell location);    // Совершить единичное действие над автоматом.
	bool lost () const;              // Игрок, который должен ходить, не может ходить.
	bool quiet () const;             // Истина, если никакая шашка не может бить.
	bool capture () const;           // Истина, если совершённое действие — взятие.
	bool finished () const;          // Никакая шашка не начала движение в полуходе.
	Cell place () const;             // Здесь находится шашка, начавшая движение.
	Role color () const;             // Таков цвет шашки, которая должна ходить.
	const Position& position () const;
private:    // Самое общее состояние доски:
	Position _position;
	Role _color;           // Вариант правил игры: для белых или для чёрных.
	bool _hungry;                     // Истина, если какая-нибудь шашка может бить.
	void forage ();                   // Проверить, может ли бить какая-нибудь шашка.
	bool hungry (Cell stone) const;   // Истина, если заданная шашка может бить.
	bool free (Cell stone) const;     // Истина, если заданная шашка может ходить.
private:    // Что шашка может делать, пока движется по диагонали:
	struct Location {
		Cell cell;                // Поле, на котором шашка находится.
		bool left;                // Шашка может отсюда брать влево.
		bool right;               // Шашка может отсюда брать вправо.
		bool stop;                // Можно здесь и завершить ход.
		Location(Cell c, bool l, bool r) : cell(c), left(l), right(r), stop(true) {}
	};
	std::vector<Location> _view;    // Распорядок движения по диагонали.
	Direction _direction;      // Направление, по которому движется шашка.
	Cell _start;               // Поле, с которого шашка начала ход.
	int _location;             // Текущее положение на диагонали.
private:    // Единичные действия, изменяющие состояние доски:
	bool advanceLocation ();
	bool installDirection (Direction direction);
	bool startStone (Cell start);
	bool stopStone ();
private:    // Функции, возвращающие true при некорректности единичного действия:
	bool imprudentStart (Cell location) const;         // Эта шашка не может вообще ходить.
	bool wrongDigression (Direction direction) const;  // Шашка не должна сходить с диагонали.
	bool crazyAdvance () const;        // Шашка не может идти дальше по диагонали.
	bool untimelyStop () const;        // Пока рано завершать ход.
private:
	std::vector<Location> browse (Cell start, Direction path) const;
};

#endif
