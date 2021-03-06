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
#include "ai_player.h"

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
	_ability = new QSlider(Qt::Horizontal);
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
	only->addWidget(_ability);
	only->addWidget(_switcher);
	only->addAction(_flip);
	only->addAction(_cut);
	only->addSeparator();
	only->addWidget(_count);
	only->addWidget(stretch);
	only->addAction(_quit);
	only->setObjectName("ToolBarControl");

	connect(_make, &QAction::triggered, this, &MainWindow::make);
	connect(_load, &QAction::triggered, this, &MainWindow::load);
	connect(_store, &QAction::triggered, this, &MainWindow::store);
	connect(_toggle, &QAction::triggered, this, &MainWindow::toggle);
	connect(_drop, &QAction::triggered, this, &MainWindow::drop);
	connect(_switcher, &GameSwitcher::notifyGui, this, &MainWindow::update);
	connect(_switcher, &GameSwitcher::gameCreated, this, &MainWindow::registerNewGame);
	connect(_flip, &QAction::triggered, this, &MainWindow::flip);
	connect(_cut, &QAction::triggered, this, &MainWindow::cut);
	connect(_quit, &QAction::triggered, qApp, &QApplication::quit);
	connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::saveSettings);

	_ability->setMinimum(1);
	_ability->setMaximum(MaxAbility);
	_ability->setValue(MaxAbility);
	_ability->setToolTip("Сила машинной игры");
	_ability->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QApplication::setOrganizationName("Evgeniy");
	QApplication::setApplicationName("Shashki");
	qApp->setWindowIcon(QIcon(":/app.png"));
	qApp->setApplicationDisplayName("Шашки");
	QDir::setCurrent(QDir::homePath());
	restoreSettings();
	registerNewGame();
	update();
}

void MainWindow::registerNewGame() {
	connect(_switcher->game(), &Game::notifyGui, this, &MainWindow::update);
	connect(_switcher->game(), &Game::notifyError, this, &MainWindow::tellError);
	setupGame();          // настройка, нужная и в начале партии, и при её продолжении
	_central->addWidget(_switcher->game()->gameWidget());
	_switcher->game()->setFlipped(_black->currentData().toInt() > _white->currentData().toInt());
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
	_white->setEnabled(_switcher->game()->enabled() && _switcher->game()->frozen());
	_black->setEnabled(_switcher->game()->enabled() && _switcher->game()->frozen());
	_ability->setEnabled(_switcher->game()->enabled() && _switcher->game()->frozen());
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

void MainWindow::make() {setupGame(); _switcher->game()->postCommand(Game::Command::Make);}
void MainWindow::toggle(bool on) {if (on) setupGame(); _switcher->game()->setFrozen(!on);}
void MainWindow::drop() {_switcher->game()->postCommand(Game::Command::Drop);}
void MainWindow::flip() {_switcher->game()->setFlipped(!_switcher->game()->flipped());}
void MainWindow::cut() {_switcher->game()->postCommand(Game::Command::Cut);}

void MainWindow::setupGame() {
	_switcher->game()->setPlayer(Role::White, _white->currentData().toInt());
	_switcher->game()->setPlayer(Role::Black, _black->currentData().toInt());
	if (_ability->value() == MaxAbility)
		_switcher->game()->setAiAbility(1.0);
	else {
		double a = _ability->value(), b = MaxAbility;
		_switcher->game()->setAiAbility(0.8 + 0.2*a/b);
	}
}
