#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QString>
#include <QList>

class QAction;
class QComboBox;
class QLabel;
class Game;
class GameSwitcher;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow();
	void update();
	void tellError(QString type, QString description);
	void restoreSettings();
	void saveSettings();
public:
	void make();
	void load();
	void store(bool on);
	void toggle(bool on);
	void drop();
	void whitePlayerType(int type);
	void blackPlayerType(int type);
	void registerNewGame();
	void flip();
	void cut();
private:
	QAction *_make;
	QAction *_load;
	QAction *_store;
	QAction *_toggle;
	QAction *_drop;
	QComboBox *_white;
	QComboBox *_black;
	GameSwitcher *_switcher;
	QAction *_flip;
	QAction *_cut;
	QLabel *_count;
	QAction *_quit;
private:
	QList<Game*> _games;
};

#endif