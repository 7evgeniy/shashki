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
	connect(_control, &BoardController::moved, this, &MainWindow::receiveAction);
	auto finished = &QFutureWatcher<std::vector<Cell>>::finished;
	connect(&_watcher, finished, this, &MainWindow::automaticDone);

	_toggle = new QAction(QIcon(":/board.png"), "Игра вкл/выкл", this);
	_drop = new QAction(QIcon(":/drop.png"), "Сбросить игру", this);
	_modes = new QComboBox;
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

	_toggle->setCheckable(true);
	_modes->addItem("Человек vs. машина", HumanVsAutomatic);
	_modes->addItem("Машина vs. человек", AutomaticVsHuman);
	_modes->addItem("Человек vs. человек", HumanVsHuman);
	_modes->setFocusPolicy(Qt::NoFocus);
	_heads->setRange(1, 1);
	_heads->setSuffix(" из 1");
	_heads->setToolTip("№ партии");
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
	only->addAction(_toggle);
	only->addAction(_drop);
	only->addWidget(_modes);
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

	using ComboSignalInt = void (QComboBox::*)(int);
	using SpinSignalInt = void (QSpinBox::*)(int);
	ComboSignalInt cic = &QComboBox::currentIndexChanged;
	SpinSignalInt vc = &QSpinBox::valueChanged;
	connect(_toggle, &QAction::triggered, this, &MainWindow::toggle);
	connect(_drop, &QAction::triggered, this, &MainWindow::drop);
	connect(_modes, cic, this, &MainWindow::updateInputState);
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

	_depth = _head = 0;
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
	_count->setEnabled(_toggle->isChecked());
	_score->setEnabled(_toggle->isChecked());
	_modes->setEnabled(!_toggle->isChecked());
	_drop->setEnabled(!_game.empty());
	_cut->setEnabled(_toggle->isChecked() && _depth != 0);
	_first->setEnabled(_game.length(_head) != _depth+1);
	_prev->setEnabled(_game.length(_head) != _depth+1);
	_next->setEnabled(_depth != 0);
	_last->setEnabled(_depth != 0);
	BoardState board = _game.at(_head, _depth);
	int white = board.position().stock(Role::White).size();
	int black = board.position().stock(Role::Black).size();
	_count->setText(QString("%1:%2").arg(white).arg(black));
	_score->setText(score(board.color(), board.lost()));
	_automatic[Role::White] = AutomaticVsHuman & _modes->currentData().toInt();
	_automatic[Role::Black] = HumanVsAutomatic & _modes->currentData().toInt();
	bool active = _depth == 0 && !_game.at(_head, 0).lost() && _toggle->isChecked();
	bool controlled = active && !_automatic[board.color()];
	_central->setActive(active);
	_central->setController(controlled ? _control : nullptr);
	if (active && !_buffer.empty()) {
		board = _game.evolve(_head, _buffer);
		_buffer.clear();
		if (_automatic[board.color()])
			playAutomatic(board);
	}
	_central->setPosition(board.position());
}

void MainWindow::receiveAction(std::vector<Cell> action) {
	_buffer = action;
	updateInputState();
}

void MainWindow::playAutomatic(BoardState board) {
	_future = QtConcurrent::run(minimax, board);
	_watcher.setFuture(_future);
}

void MainWindow::automaticDone() {receiveAction(_future.result());}

void MainWindow::goBack() {++ _depth; updateInputState();}
void MainWindow::goForth() {-- _depth; updateInputState();}
void MainWindow::goStart() {_depth = _game.length(_head); updateInputState();}
void MainWindow::goFinish() {_depth = 0; updateInputState();}

void MainWindow::flip() {_central->setFlipped(!_central->flipped());}

void MainWindow::toggle(bool on) {
	if (on && _game.empty() && _automatic[Role::White])
		playAutomatic(BoardState::initialBoard());
	updateInputState();
}

void MainWindow::drop() {
	_game = Game();
	_head = _depth = 0;
	_buffer.clear();
	_toggle->setChecked(false);
	updateInputState();
}

void MainWindow::cut() {
	_game.cut(_head, _depth);
	_heads->setMaximum(_heads->maximum()+1);
	_heads->setSuffix(QString(" из %1").arg(_heads->maximum()));
	_head = _heads->maximum()-1;
	_depth = 0;
	_buffer.clear();
	if (_automatic[_game.at(_head, _depth).color()])
		playAutomatic(_game.at(_head, _depth));
	_heads->setValue(_heads->maximum()-1);
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
