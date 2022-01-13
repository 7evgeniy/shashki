#ifndef BOARD_CONTROLLER_H
#define BOARD_CONTROLLER_H

#include <QObject>
#include <QList>
#include <memory>
#include "../board/cell.h"
#include "../board/board_state.h"
#include "board_widget.h"
#include "root.h"

class BoardController : public QObject {
	Q_OBJECT
public:
	BoardController(QObject *parent = nullptr);
	void setControl(BoardWidget *control);
	void setBoard(BoardState board);
public:
	void click(Cell at);
	void hover(Cell at) const;
	void reset();
signals:
	void moved(QList<Cell> move);
private:
	std::shared_ptr<Root<Cell>> makeRoot(Cell at) const;
	QList<Cell> readAction(Root<Cell>::Iterator head) const;
	QList<Root<Cell>::Iterator> restrictHeads(Cell at) const;
	QList<Cell> commonPrefix(QList<Root<Cell>::Iterator> heads, Cell at) const;
private:
	BoardWidget *_control;
	BoardState _board;
	std::shared_ptr<Root<Cell>> _move;
	QList<Root<Cell>::Iterator> _heads;
	int _count;
};

#endif
