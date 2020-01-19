#include "gui_player.h"
#include "board_widget.h"
#include "game.h"

GuiPlayer::GuiPlayer(Game *game, BoardWidget *widget) :
	Player(game), _widget(widget), _navigator(&_tree) {     // is not yet fully constructed!
	connect(_widget, &BoardWidget::click, this, &GuiPlayer::click);
	connect(_widget, &BoardWidget::move, this, &GuiPlayer::move);
	connect(_widget, &BoardWidget::reset, this, &GuiPlayer::reset);
}

void GuiPlayer::activate(const BoardState &board) {
	_board = board;
	_active = true;
}

void GuiPlayer::deactivate() {
	_active = false;
	_navigator.clear();
	_tree.clear();
	_widget->unmark();
}

void GuiPlayer::click(Cell cell) {
	if (!_active)
		return;
	if (_navigator.empty()) {      // ход не начат
		if (_tree.init(_board, cell)) {
			_navigator.init(ActionTree::StartIndex);
			_widget->markBorder(cell);
		}
	}
	else {   // надо либо продолжить ход, либо прекратить набор хода
		bool repeat = (_navigator.past().isEmpty() && cell == _navigator.seed());
		if (!repeat && _navigator.navigate(cell))
			evolve(_navigator.future());
		else
			reset();
	}
}

void GuiPlayer::move(Cell cell) {
	if (!_active || _navigator.empty())
		return;
	if (_navigator.past().isEmpty() && cell == _navigator.seed())
		_widget->markBlue({});
	else {
		_navigator.navigate(cell);
		_widget->markBlue(toCells(_navigator.future()));
	}
}

void GuiPlayer::reset() {
	if (!_active)
		return;
	_navigator.clear();
	_tree.clear();
	_widget->unmark();
}

void GuiPlayer::evolve(QList<ActionTree::Index> part) {
	_navigator.append(part);
	QList<Cell> action = toCells(_navigator.action());
	_widget->markGreen(action);
	if (!action.last().valid())
		_game->move(action);
}

QList<Cell> GuiPlayer::toCells(QList<ActionTree::Index> action) {
	QList<Cell> result;
	for (ActionTree::Index index : action)
		result.push_back(_tree.cell(index));
	return result;
}
