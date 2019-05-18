#include "main_window.h"
#include <QAction>
#include <QComboBox>
#include <QLabel>
#include "game.h"
#include <QToolBar>
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include "game_switcher.h"
#include <QStackedWidget>

MainWindow::MainWindow() {
	_central = new QStackedWidget;
	setCentralWidget(_central);
	_make = new QAction(QIcon(":/make.png"), "Запуск игры", this);
	_load = new QAction(QIcon(":/load.png"), "Загрузка файла", this);
	_store = new QAction(QIcon(":/store.png"), "Запись в файл", this);
	_toggle = new QAction(QIcon(":/toggle.png"), "Приём ходов вкл/выкл", this);
	_drop = new QAction(QIcon(":/drop.png"), "Сброс игры", this);
	_white = Game::makePlayerList(Role::White);
	_black = Game::makePlayerList(Role::Black);
	_switcher = new GameSwitcher;
	_flip = new QAction(QIcon(":/flip.png"), "Перевернуть доску", this);
	_cut = new QAction(QIcon(":/cut.png"), "Обрезать игру", this);
	_count = new QLabel;
	_quit = new QAction(QIcon(":/quit.png"), "Выход", this);
	_store->setCheckable(true);
	_toggle->setCheckable(true);

	QToolBar *only = addToolBar("Управление");
	QWidget *stretch = new QWidget;
	stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	only->addAction(_make);
	only->addAction(_load);
	only->addAction(_store);
	only->addAction(_toggle);
	only->addAction(_drop);
	only->addWidget(_white);
	only->addWidget(_black);
	only->addWidget(_switcher);
	only->addAction(_flip);
	only->addAction(_cut);
	only->addSeparator();
	only->addWidget(_count);
	only->addWidget(stretch);
	only->addAction(_quit);
	only->setObjectName("ToolBarControl");

	typedef void (QComboBox::*PtrToCB_Int)(int);
	PtrToCB_Int cic = &QComboBox::currentIndexChanged;
	connect(_make, &QAction::triggered, this, &MainWindow::make);
	connect(_load, &QAction::triggered, this, &MainWindow::load);
	connect(_store, &QAction::triggered, this, &MainWindow::store);
	connect(_toggle, &QAction::triggered, this, &MainWindow::toggle);
	connect(_drop, &QAction::triggered, this, &MainWindow::drop);
	connect(_white, cic, this, &MainWindow::whitePlayerType);
	connect(_black, cic, this, &MainWindow::blackPlayerType);
	connect(_switcher, &GameSwitcher::notifyGui, this, &MainWindow::update);
	connect(_switcher, &GameSwitcher::gameCreated, this, &MainWindow::registerNewGame);
	connect(_flip, &QAction::triggered, this, &MainWindow::flip);
	connect(_cut, &QAction::triggered, this, &MainWindow::cut);
	connect(_quit, &QAction::triggered, qApp, &QApplication::quit);
	connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::saveSettings);

	whitePlayerType(_white->currentData().toInt());
	blackPlayerType(_black->currentData().toInt());
	QApplication::setOrganizationName("Evgeniy");
	QApplication::setApplicationName("Shashki");
	qApp->setWindowIcon(QIcon(":/app.png"));
	setWindowTitle("Шашки");
	QDir::setCurrent(QDir::homePath());
	restoreSettings();
	registerNewGame();
	update();
}

void MainWindow::registerNewGame() {
	connect(_switcher->game(), &Game::notifyGui, this, &MainWindow::update);
	connect(_switcher->game(), &Game::notifyError, this, &MainWindow::tellError);
	_central->addWidget(_switcher->game()->gameWidget());
}

void MainWindow::dropOldGame() {
	_central->removeWidget(_switcher->game()->gameWidget());
}

void MainWindow::update() {
	_central->setCurrentWidget(_switcher->game()->gameWidget());
	if (_switcher->game()->enabled()) {
		int white = _switcher->game()->stoneCount(Role::White);
		int black = _switcher->game()->stoneCount(Role::Black);
		_count->setText(QString("Число шашек %1:%2").arg(white).arg(black));
	}
	else
		_count->setText("");
	_make->setEnabled(!_switcher->game()->enabled());
	_load->setEnabled(!_switcher->game()->enabled());
	_store->setEnabled(_switcher->game()->enabled() && _switcher->game()->frozen());
	_toggle->setEnabled(_switcher->game()->enabled() && !_switcher->game()->lost());
	_drop->setEnabled(_switcher->game()->enabled());
	_cut->setEnabled(_switcher->game()->enabled());
	_toggle->setChecked(_switcher->game()->enabled() && !_switcher->game()->frozen());
	if (_switcher->game()->isWriting()) {
		_store->setToolTip(QString("Запись в файл: %1").arg(_switcher->game()->filename()));
		_store->setChecked(true);
	}
	else {
		_store->setToolTip(QString("Запись в файл"));
		_store->setChecked(false);
	}
}

void MainWindow::tellError(QString type, QString description) {
	QMessageBox msgBox;
	msgBox.setWindowTitle(windowTitle());
	msgBox.setText(type);
	msgBox.setInformativeText(description);
	msgBox.setIcon(QMessageBox::Warning);
	msgBox.exec();
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

void MainWindow::load() {
	QFileDialog dialog(this, "Загрузить игру", QDir::currentPath());
	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setDefaultSuffix("sha");
	if (dialog.exec() == QFileDialog::Accepted) {
		_switcher->game()->setFilename(dialog.selectedFiles().first());
		_switcher->game()->postCommand(Game::Command::Read);
	}
	QDir::setCurrent(dialog.directory().absolutePath());
}

void MainWindow::store(bool on) {
	if (on) {
		QFileDialog dialog(this, "Повести запись игры в файл", QDir::currentPath());
		dialog.setFileMode(QFileDialog::AnyFile);
		dialog.setAcceptMode(QFileDialog::AcceptSave);
		dialog.setDefaultSuffix("sha");
		if (dialog.exec() == QFileDialog::Accepted) {
			_switcher->game()->setFilename(dialog.selectedFiles().first());
			_switcher->game()->postCommand(Game::Command::Write);
		}
		QDir::setCurrent(dialog.directory().absolutePath());
	}
	else
		_switcher->game()->stopWriting();
}

void MainWindow::make() {_switcher->game()->postCommand(Game::Command::Make);}
void MainWindow::toggle(bool on) {_switcher->game()->setFrozen(!on);}
void MainWindow::drop() {_switcher->game()->postCommand(Game::Command::Drop);}
void MainWindow::whitePlayerType(int type) {_switcher->game()->setPlayerType(Role::White, type);}
void MainWindow::blackPlayerType(int type) {_switcher->game()->setPlayerType(Role::Black, type);}
void MainWindow::flip() {_switcher->game()->postCommand(Game::Command::Flip);}
void MainWindow::cut() {_switcher->game()->postCommand(Game::Command::Cut);}
