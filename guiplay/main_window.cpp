#include "main_window.h"
#include "board_widget.h"
#include "board_controller.h"
#include <QAction>
#include <QIcon>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QToolBar>
#include <QApplication>
#include <QSettings>
#include <QtConcurrent/QtConcurrentRun>
#include "../board/minimax.h"

MainWindow::MainWindow() {
	_central = new BoardWidget;
	_control = new BoardController(this);
	setCentralWidget(_central);
	auto f = [this](std::vector<Cell> v){receiveAction(v, ActionType::Gui);};
	connect(_control, &BoardController::moved, this, f);
	auto finished = &QFutureWatcher<std::vector<Cell>>::finished;
	connect(&_watcher, finished, this, &MainWindow::automaticDone);

	_fork = new QAction(QIcon(":/make.png"), "Переходить", this);
	_white = new QAction(QIcon(":/board.png"), "Белые — автоматически", this);
	_black = new QAction(QIcon(":/board.png"), "Чёрные — автоматически", this);
	_white->setCheckable(true);
	_black->setCheckable(true);
	_heads = new QSpinBox;
	_flip = new QAction(QIcon(":/flip.png"), "Перевернуть доску", this);
	_cut = new QAction(QIcon(":/cut.png"), "Обрезать игру", this);
	_first = new QAction("<<", this);
	_prev = new QAction("<", this);
	_score = new QLabel;
	_next = new QAction(">", this);
	_last = new QAction(">>", this);
	_count = new QLabel;
	_quit = new QAction(QIcon(":/quit.png"), "Выход", this);

	_heads->setRange(1, 1);
	_heads->setSuffix(" из 1");
	_heads->setToolTip("№ игровой ветви");
	_heads->setFocusPolicy(Qt::NoFocus);
	_prev->setToolTip("Предыдущая позиция");
	_next->setToolTip("Следующая позиция");
	_first->setToolTip("Начальная позиция");
	_last->setToolTip("Последняя позиция");
	_count->setToolTip("Число шашек");

	QToolBar *only = addToolBar("tb");
	only->setObjectName("tb");
	QWidget *stretch = new QWidget;
	stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	only->addAction(_fork);
	only->addAction(_white);
	only->addAction(_black);
	only->addWidget(_heads);
	only->addAction(_flip);
	only->addAction(_cut);
	only->addWidget(_score);
	only->addAction(_first);
	only->addAction(_prev);
	only->addAction(_next);
	only->addAction(_last);
	only->addWidget(_count);
	only->addWidget(stretch);
	only->addAction(_quit);

	using SpinSignalInt = void (QSpinBox::*)(int);
	SpinSignalInt vc = &QSpinBox::valueChanged;
	connect(_fork, &QAction::triggered, this, &MainWindow::fork);
	connect(_white, &QAction::triggered, this, &MainWindow::white);
	connect(_black, &QAction::triggered, this, &MainWindow::black);
	connect(_heads, vc, this, &MainWindow::updateInputState);
	connect(_flip, &QAction::triggered, this, &MainWindow::flip);
	connect(_cut, &QAction::triggered, this, &MainWindow::cut);
	connect(_first, &QAction::triggered, this, &MainWindow::goStart);
	connect(_prev, &QAction::triggered, this, &MainWindow::goBack);
	connect(_next, &QAction::triggered, this, &MainWindow::goForth);
	connect(_last, &QAction::triggered, this, &MainWindow::goFinish);
	connect(_quit, &QAction::triggered, qApp, &QApplication::quit);
	connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::saveSettings);

	QApplication::setOrganizationName("Evgeniy");
	QApplication::setApplicationName("Shashki");
	qApp->setWindowIcon(QIcon(":/app.png"));
	setWindowTitle("Шашки");
	restoreSettings();

	_automatic[Role::White] = _automatic[Role::Black] = false;
	_depth = _head = 0;
	requestAction(_game.at(0, 0));
	updateInputState();
}

QString score(Role color, bool lost) {
	QString role;
	if (lost) {
		switch (color) {
		case Role::White: role = "чёрных"; break;
		case Role::Black: role = "белых"; break;
		default: return "";
		}
		return QString("Победа %1").arg(role);
	}
	else {
		switch (color) {
		case Role::White: role = "белых"; break;
		case Role::Black: role = "чёрных"; break;
		default: return "";
		}
		return QString("Ход %1").arg(role);
	}
}

void MainWindow::updateInputState() {
	_fork->setEnabled(_depth != 0);
	_cut->setEnabled(_game.inner(_head, _depth));
	_first->setEnabled(_game.length(_head) != _depth+1);
	_prev->setEnabled(_game.length(_head) != _depth+1);
	_next->setEnabled(_depth != 0);
	_last->setEnabled(_depth != 0);
	BoardState board = _game.at(_head, _depth);
	int white = board.position().stock(Role::White).size();
	int black = board.position().stock(Role::Black).size();
	_count->setText(QString("%1:%2").arg(white).arg(black));
	_score->setText(score(board.color(), board.lost()));
	bool active = (_depth == 0 && !board.lost());
	if (active && !_buffer.empty()) {    // получен ход
		board = _game.evolve(_head, _buffer);
		requestAction(board);
	}
	_central->setActive(active);
	_central->setPosition(board.position());
}

void MainWindow::receiveAction(std::vector<Cell> action, ActionType type) {
	if (type != _receiver)
		return;
	_receiver = ActionType::None;
	_buffer = action;
	updateInputState();
}

// подготовка контекста к получению хода для игры
void MainWindow::requestAction(BoardState board) {
	_buffer.clear();
	if (board.lost())
		return;
	if (_automatic[board.color()]) {
		_receiver = ActionType::Automatic;
		_central->setController(nullptr);
		_future = QtConcurrent::run(minimax, board);
		_watcher.setFuture(_future);
	}
	else {
		_receiver = ActionType::Gui;
		_central->setController(_control);
	}
}

void MainWindow::automaticDone() {receiveAction(_future.result(), ActionType::Automatic);}

void MainWindow::goBack() {++ _depth; updateInputState();}
void MainWindow::goForth() {-- _depth; updateInputState();}
void MainWindow::goStart() {_depth = _game.length(_head); updateInputState();}
void MainWindow::goFinish() {_depth = 0; updateInputState();}

void MainWindow::flip() {_central->setFlipped(!_central->flipped());}

void MainWindow::fork() {
	_game.fork(_head, _depth);
	_heads->setMaximum(_heads->maximum()+1);
	_heads->setSuffix(QString(" из %1").arg(_heads->maximum()));
	_head = _heads->maximum()-1;
	_depth = 0;
	_heads->setValue(_heads->maximum()-1);
	requestAction(_game.at(_head, _depth));
	updateInputState();
}

void MainWindow::white(bool on) {
	BoardState board = _game.at(_head, 0);
	_automatic[Role::White] = on;
	if (board.color() == Role::White)
		requestAction(board);
}

void MainWindow::black(bool on) {
	BoardState board = _game.at(_head, 0);
	_automatic[Role::Black] = on;
	if (board.color() == Role::Black)
		requestAction(board);
}

void MainWindow::cut() {
	requestAction(_game.cut(_head, _depth));
	_head = _depth = 0;
	updateInputState();
}

void MainWindow::restoreSettings() {
	QSettings settings;

	settings.beginGroup("mainwindow");
	QVariant state = settings.value("state");
	QVariant rect = settings.value("geometry");
	settings.endGroup();

	setWindowState(Qt::WindowMaximized);
	if (rect.isValid())
		restoreGeometry(rect.toByteArray());
	if (state.isValid())
		restoreState(state.toByteArray());
}

void MainWindow::saveSettings() {
	QSettings settings;
	settings.beginGroup("mainwindow");
	settings.setValue("state", QVariant(saveState()));
	settings.setValue("geometry", QVariant(saveGeometry()));
	settings.endGroup();
}
