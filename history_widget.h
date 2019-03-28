#ifndef HISTORY_WIDGET_H
#define HISTORY_WIDGET_H

#include <QWidget>
#include <QString>
#include "role.h"

class Game;
class QPushButton;
class QLabel;

// Виджет, выделяющий в истории игры одну позицию из многих пройденных.
class HistoryWidget : public QWidget {
	Q_OBJECT
public:
	HistoryWidget(QWidget *parent = nullptr);
	void setGame(Game *game);
	void update();
public:
	void goBack();
	void goForth();
	void goStart();
	void goFinish();
private:
	static QString text(Role color, bool lost);
	Game *_game;
	QPushButton *_prev;
	QPushButton *_next;
	QPushButton *_first;
	QPushButton *_last;
	QLabel *_score;
};

#endif
