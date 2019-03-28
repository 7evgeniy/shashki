#include "history_widget.h"
#include "game.h"
#include "role.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

HistoryWidget::HistoryWidget(QWidget *parent) : QWidget(parent), _game(nullptr) {
	_first = new QPushButton("<<");
	_last = new QPushButton(">>");
	_prev = new QPushButton("<");
	_next = new QPushButton(">");
	_score = new QLabel;
	QHBoxLayout *everything = new QHBoxLayout;
	everything->addWidget(_first);
	everything->addWidget(_prev);
	everything->addWidget(_score);
	everything->addWidget(_next);
	everything->addWidget(_last);
	setLayout(everything);

	_prev->setToolTip("Предыдущая позиция");
	_next->setToolTip("Следующая позиция");
	_first->setToolTip("Начальная позиция");
	_last->setToolTip("Последняя позиция");
	_score->setAlignment(Qt::AlignHCenter);

	_first->setFocusPolicy(Qt::NoFocus);
	_last->setFocusPolicy(Qt::NoFocus);
	_prev->setFocusPolicy(Qt::NoFocus);
	_next->setFocusPolicy(Qt::NoFocus);
	_first->setShortcut(QKeySequence("Alt+Home"));
	_last->setShortcut(QKeySequence("Alt+End"));
	_prev->setShortcut(QKeySequence("Alt+Left"));
	_next->setShortcut(QKeySequence("Alt+Right"));

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void HistoryWidget::setGame(Game *game) {
	_game = game;
	disconnect();
	connect(_first, &QPushButton::clicked, this, &HistoryWidget::goStart);
	connect(_last, &QPushButton::clicked, this, &HistoryWidget::goFinish);
	connect(_prev, &QPushButton::clicked, this, &HistoryWidget::goBack);
	connect(_next, &QPushButton::clicked, this, &HistoryWidget::goForth);
}

void HistoryWidget::update() {
	bool enabled = _game && _game->enabled();
	if (enabled) {
		_first->setEnabled(_game->reportHistory(Game::Travel::Start));
		_prev->setEnabled(_game->reportHistory(Game::Travel::Back));
		_last->setEnabled(_game->reportHistory(Game::Travel::Finish));
		_next->setEnabled(_game->reportHistory(Game::Travel::Forth));
		_score->setText(text(_game->nowColor(), _game->nowLost()));
	}
	setVisible(enabled);
	QWidget::update();
}

void HistoryWidget::goBack() {_game->travelHistory(Game::Travel::Back);}
void HistoryWidget::goForth() {_game->travelHistory(Game::Travel::Forth);}
void HistoryWidget::goStart() {_game->travelHistory(Game::Travel::Start);}
void HistoryWidget::goFinish() {_game->travelHistory(Game::Travel::Finish);}

QString HistoryWidget::text(Role color, bool lost) {
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
