#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include "../board/board_state.h"
#include "../board/cell.h"
#include "game.h"
#include <QList>
#include <QFutureWatcher>
#include <QFuture>

class QComboBox;
class QSpinBox;
class QAction;
class QLabel;
class BoardWidget;
class BoardController;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow();
	void updateInputState();
	void receiveAction(std::vector<Cell> action);
	void playHuman(BoardState board);
	void playAutomatic(BoardState board);
public:
	void drop();
	void goBack();
	void goForth();
	void goStart();
	void goFinish();
	void flip();
	void cut();
	void saveSettings();
	void restoreSettings();
	void automaticDone();
private:
	static const int HumanVsHuman = 0;
	static const int HumanVsAutomatic = 1;
	static const int AutomaticVsHuman = 2;
private:
	QAction *_toggle;
	QAction *_drop;
	QComboBox *_modes;
	QSpinBox *_heads;
	QAction *_flip;
	QAction *_cut;
	QAction *_first;
	QAction *_prev;
	QLabel *_score;
	QAction *_next;
	QAction *_last;
	QLabel *_count;
	QAction *_quit;
private:
	BoardWidget *_central;
	BoardController *_control;
	QFutureWatcher<std::vector<Cell>> _watcher;
	QFuture<std::vector<Cell>> _future;

private:
	Game _game;    // дерево игры: позиции после каждого полного полухода.
	int _head;     // текущая вершина в дереве позиций, то есть текущая игра.
	int _depth;    // номер текущей позиции в игре, считая от вершины дерева.
	std::vector<Cell> _buffer;
	bool _automatic[2];
};

#endif
