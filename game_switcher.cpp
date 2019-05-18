#include "game_switcher.h"
#include <QActionGroup>
#include <QAction>
#include "game.h"

GameSwitcher::GameSwitcher() {
	QAction *add = addAction(QIcon(":/add.png"), QString("Скопировать доску"));
	QAction *remove = addAction(QIcon(":/remove.png"), QString("Убрать игру"));
	_group = new QActionGroup(this);

	connect(add, &QAction::triggered, this, &GameSwitcher::addButton);
	connect(remove, &QAction::triggered, this, &GameSwitcher::removeButton);
	connect(_group, &QActionGroup::triggered, this, &GameSwitcher::notifyGui);

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setIconSize(QSize(16, 16));
	addButton();
}

Game* GameSwitcher::game() const {
	return _games[_group->checkedAction()->data().toInt()];
}

void GameSwitcher::addButton() {
	int i = _games.count();
	_actions.append(addAction(QIcon(":/board.png"), QString("Партия №%1").arg(i+1)));
	_games.append(_games.isEmpty() ? new Game : game()->clone());
	_games.last()->setParent(this);
	_actions.last()->setData(i);
	_actions.last()->setCheckable(true);
	_actions.last()->setActionGroup(_group);
	_actions.last()->setChecked(true);
	emit gameCreated();
	emit notifyGui();
}

void GameSwitcher::removeButton() {
	emit aboutToDrop();
	int index = _group->checkedAction()->data().toInt();
	Game *game = _games.takeAt(index);
	_group->removeAction(_actions.takeAt(index));
	_actions[index]->setChecked(true);
	game->deleteLater();
	emit notifyGui();
}
