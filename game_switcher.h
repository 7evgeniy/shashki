#ifndef GAME_SWITCHER_H
#define GAME_SWITCHER_H

#include <QToolBar>
#include <QList>

class QActionGroup;
class QAction;
class Game;

class GameSwitcher : public QToolBar {
	Q_OBJECT
public:
	GameSwitcher();
	Game* game() const;
	void addButton();
	void removeButton();
signals:
	void notifyGui();
	void gameCreated();
	void aboutToDrop();
private:
	QAction *_add;
	QAction *_remove;
	QActionGroup *_group;
	QList<QAction*> _actions;
	QList<Game*> _games;
};

#endif
