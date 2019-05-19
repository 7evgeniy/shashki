#include "game_switcher.h"
#include <QActionGroup>
#include <QAction>
#include "game.h"

GameSwitcher::GameSwitcher() {
	_add = addAction(QIcon(":/add.png"), QString("Скопировать доску"));
	_remove = addAction(QIcon(":/remove.png"), QString("Убрать игру"));
	_remove->setEnabled(false);
	_group = new QActionGroup(this);

	connect(_add, &QAction::triggered, this, &GameSwitcher::addButton);
	connect(_remove, &QAction::triggered, this, &GameSwitcher::removeButton);
	connect(_group, &QActionGroup::triggered, this, &GameSwitcher::notifyGui);

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setIconSize(QSize(16, 16));
	addButton();
}

Game* GameSwitcher::game() const {
	return _games[_actions.indexOf(_group->checkedAction())];
}

void GameSwitcher::addButton() {
	int i = _games.count();
	_games.append(_games.isEmpty() ? new Game : game()->clone());
	_games.last()->setParent(this);
	_actions.append(addAction(QIcon(":/board.png"), QString("Партия №%1").arg(i+1)));
	_actions.last()->setCheckable(true);
	_actions.last()->setActionGroup(_group);
	_actions.last()->setChecked(true);
	_remove->setEnabled(true);
	emit gameCreated();
	emit notifyGui();
}

void GameSwitcher::removeButton() {
	emit aboutToDrop();
	int index = _actions.indexOf(_group->checkedAction());
	Game *game = _games.takeAt(index);
	QAction *action = _actions.takeAt(index);
	action->setActionGroup(nullptr);
	if (_group->actions().count() == 1) {
		_remove->setEnabled(false);
		index = 0;
	}
	_actions[index]->setChecked(true);
	game->deleteLater();
	action->deleteLater();
	emit notifyGui();
}
