#ifndef GUI_PLAYER_H
#define GUI_PLAYER_H

#include "player.h"
#include "action_tree.h"
#include "action_builder.h"

class BoardWidget;

class GuiPlayer : public Player {
	Q_OBJECT
public:
	GuiPlayer(Game *game, BoardWidget *widget);
	void activate(const BoardState &board);
	void deactivate();
public:
	void click(Cell cell);     // получено поле, где пользователь щёлкнул
	void move(Cell cell);      // получено поле, на которое пользователь навёл курсор
	void reset();              // пользователь отказался от завершения выбранного хода
private:
	void evolve(QList<ActionTree::Index> part);
	QList<Cell> toCells(QList<ActionTree::Index> action);
private:
	BoardState _board;
	BoardWidget *_widget;
	ActionTree _tree;
	ActionBuilder _navigator;
	bool _active;        // «этот игрок вправе управлять виджетом доски и игрой»
};

#endif
