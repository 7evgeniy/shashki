#include "game.h"
#include "board_widget.h"
#include "history_widget.h"
#include <QVBoxLayout>
#include <QComboBox>
#include "ai_player.h"
#include "gui_player.h"
#include <QDataStream>
#include <QFile>

const uint8_t Game::FileHeader[4] = {0x53, 0x48, 0x00, 0x00};

const QString Game::AtLoadError = "Ошибка при загрузке файла";
const QString Game::AtStoreError = "Ошибка при сохранении файла";
const QString Game::CouldntOpenError = "Не получается открыть файл";
const QString Game::WrongFormatError = "Файл имеет неверный формат";
const QString Game::CouldntReadError = "Не получается прочесть файл";
const QString Game::CouldntWriteError = "Не получается записать данные в файл";

Game::Game(QObject *parent) : QObject(parent) {
	_board = new BoardWidget;
	_history = new HistoryWidget;
	_widget = new QWidget;
	QVBoxLayout *everything = new QVBoxLayout;
	everything->addWidget(_board);
	everything->addWidget(_history);
	_widget->setLayout(everything);
	_history->setGame(this);
	connect(this, &Game::notifyGui, _history, &HistoryWidget::update);
	connect(this, &Game::notifyGui, this, &Game::setupBoard);

	_players[0] = nullptr;
	_players[1] = nullptr;
	_frozen = false;
	_writing = false;

	notifyGui();
}

Game* Game::clone() const {
	Game *other = new Game(parent());
	other->_board->setFlipped(_board->flipped());
	if (enabled()) {
		other->_project[0] = _project[0];
		other->_project[1] = _project[1];
		other->makePlayers();
		other->_actions = _actions;
		other->_states = _states;
		other->_now = _now;
		other->notifyGui();
	}
	return other;
}

// Предусловие: action — один корректный полуход, завершённый командой смены игрока.
void Game::move(QList<Cell> action) {
	if (frozen()) {
		_buffer = action;
		return;
	}
	if (!enabled())
		return;
	BoardState copy = _states.last();
	_players[copy.color()]->deactivate();
	for (Cell cell : action)
		copy.control(cell);
	if (_now == _states.count()-1)
		++ _now;
	_actions.append(action);
	_states.append(copy);
	_players[copy.color()]->activate(copy);
	if (_writing) {
		QFile file(_filename);
		file.open(QIODevice::WriteOnly | QIODevice::Append);
		QDataStream out(&file);
		if (!file.isOpen() || !writeAction(out, action)) {
			_writing = false;
			notifyError(AtStoreError, CouldntWriteError);
		}
	}
	notifyGui();
}

void Game::setFrozen(bool is) {
	_frozen = is;
	if (_frozen)
		_buffer.clear();
	else if (!_buffer.isEmpty())
		move(_buffer);
	notifyGui();
}

void Game::setPlayerType(Role color, PlayerType type) {_project[color] = type;}
void Game::setFilename(QString filename) {_filename = filename;}
void Game::stopWriting() {_writing = false; notifyGui();}
QWidget* Game::gameWidget() const {return _widget;}
bool Game::lost() const {return !enabled() || _states.last().lost();}
bool Game::enabled() const {return !_states.isEmpty();}
bool Game::frozen() const {return _frozen;}
QString Game::filename() const {return _filename;}
bool Game::isWriting() const {return _writing;}

int Game::stoneCount(Role color) const {
	if (!enabled())
		return 0;
	return _states[_now].position().stock(color).size();
}

void Game::postCommand(Command command) {
	try {
		switch(command) {
		case Command::Make: _filename.clear(); doMake(); break;
		case Command::Read: doRead(); break;
		case Command::Write: doWrite(); break;
		case Command::Drop: doDrop(); break;
		case Command::Cut: doCut(); break;
		case Command::Flip: doFlip(); break;
		default: return;
		}
	} catch (Exception& e) {
		QString type;
		if (command == Command::Write || command == Command::Cut)
			type = AtStoreError;
		if (command == Command::Read)
			type = AtLoadError;
		notifyError(type, e.description());
	}
	notifyGui();
}

void Game::doMake() {
	_frozen = false;
	_buffer.clear();
	makePlayers();
	_board->setFlipped(_project[Role::Black] < _project[Role::White]);
	_buffer.clear();
	_actions.clear();
	_states = {initialBoard()};
	if (_filename.isEmpty())   // если игра создаётся не под чтение из файла:
		_players[Role::White]->activate(_states.last());
	_now = 0;
	_writing = false;
}

void Game::doRead() {
	QFile file(_filename);
	file.open(QIODevice::ReadOnly);
	if (!file.isOpen())
		throw Exception(CouldntOpenError);
	QDataStream in(&file);
	if (!checkHeader(in))
		throw Exception(WrongFormatError);
	doMake();
	auto actions = readActions(in);
	BoardState copy = _states.last();
	for (auto action : actions) {
		for (Cell cell : action)
			if (!copy.control(cell)) {
				doDrop();
				throw Exception(WrongFormatError);
			}
		_actions.append(action);
		_states.append(copy);
	}
	if (!_states.last().finished()) {
		doDrop();
		throw Exception(WrongFormatError);
	}
	_players[_states.last().color()]->activate(_states.last());
	_now = _states.count()-1;
	_writing = true;
}

void Game::doWrite() {
	if (!enabled())
		return;
	QFile file(_filename);
	file.open(QIODevice::WriteOnly | QIODevice::Truncate);
	QDataStream out(&file);
	if (!file.isOpen())
		throw Exception(CouldntOpenError);
	if (!writeHeader(out))
		throw Exception(CouldntWriteError);
	for (auto action : _actions)
		if (!writeAction(out, action))
			throw Exception(CouldntWriteError);
	_writing = true;
}

void Game::doDrop() {
	_players[_states.last().color()]->deactivate();
	delete _players[0];
	delete _players[1];
	_players[0] = nullptr;
	_players[1] = nullptr;
	_buffer.clear();
	_states.clear();
	_actions.clear();
	_frozen = false;
	_buffer.clear();
}

void Game::doCut() {
	if (_now == _states.count()-1)
		return;
	_states.erase(_states.begin()+_now+1, _states.end());
	_actions.erase(_actions.begin()+_now, _actions.end());
	_players[_states.last().color()]->activate(_states.last());
	_buffer.clear();
	_frozen = false;
	if (_writing)
		doWrite();
}

void Game::doFlip() {
	_board->setFlipped(!_board->flipped());
}

// Предусловие: игра ведётся.
void Game::travelHistory(Travel travel) {
	switch (travel) {
	case Travel::Back: if (_now > 0) -- _now; break;
	case Travel::Forth: if (_now < _states.count()-1) ++ _now; break;
	case Travel::Start: _now = 0; break;
	case Travel::Finish: _now = _states.count()-1; break;
	default: return;
	}
	notifyGui();
}

// Предусловие: игра ведётся.
bool Game::reportHistory(Travel travel) const {
	switch (travel) {
	case Travel::Back: return _now > 0;
	case Travel::Forth: return _now < _states.count()-1;
	case Travel::Start: return _now > 0;
	case Travel::Finish: return _now < _states.count()-1;
	}
	return false;
}

// Предусловие: игра ведётся.
Role Game::nowColor() const {
	return _states[_now].color();
}

// Предусловие: игра ведётся.
bool Game::nowLost() const {
	return _states[_now].lost();
}

void Game::setupBoard() {
	if (!enabled()) {
		_board->setEnabled(false);
		_board->setPosition(initialPosition());
	}
	else {
		bool active = enabled() && !frozen() && _now == _states.count()-1 && !lost();
		_board->setEnabled(active);
		_board->setPosition(_states[_now].position());
	}
}

bool Game::checkHeader(QDataStream &in) {
	quint8 byte;
	for (int i = 0; i < 4; ++ i) {
		in >> byte;
		if (in.status() != QDataStream::Ok)
			throw Exception("Не получается прочесть файл");
		if (byte != FileHeader[i])
			return false;
	}
	return true;
}

bool Game::writeHeader(QDataStream &out) {
	for (int i = 0; i < 4; ++ i)
		out << FileHeader[i];
	return out.status() == QDataStream::Ok;
}

QList<QList<Cell>> Game::readActions(QDataStream &in) {
	QList<QList<Cell>> actions;
	do {
		QList<Cell> action;
		Cell cell;
		do {
			qint8 byte;
			in >> byte;
			if (in.status() != QDataStream::Ok)
				throw Exception("Не получается прочесть файл");
			cell = Cell::fromIndex(byte);
			action.append(cell);
		} while (cell.valid());
		actions.append(action);
	} while(!in.atEnd());
	return actions;
}

bool Game::writeAction(QDataStream &out, QList<Cell> action) {
	for (Cell cell : action) {
		qint8 byte = cell.index();
		out << byte;
	}
	return out.status() == QDataStream::Ok;
}

BoardState Game::initialBoard() {
	return BoardState(initialPosition(), Role::White);
}

Position Game::initialPosition() {
	Position::Stone cells[32];
	Position::Stone white(Role::White);
	Position::Stone black(Role::Black);
	Position::Stone empty(Role::None);
	int i = 0;
	while (i < 12)
		cells[i++] = white;
	while (i < 20)
		cells[i++] = empty;
	while (i < 32)
		cells[i++] = black;
	return Position(cells);
}

QComboBox* Game::makePlayerList(Role color) {
	if (!color.valid())
		return nullptr;
	QComboBox *combo = new QComboBox;
	combo->addItem("Человек", Game::Human);
	combo->addItem("Машина", Game::Machine);
	combo->setCurrentIndex(color);
	if (color == Role::White)
		combo->setToolTip("Белые");
	if (color == Role::Black)
		combo->setToolTip("Чёрные");
	return combo;
}

void Game::makePlayers() {
	for (int i = 0; i < 2; ++ i) {
		Player *player = nullptr;
		switch (_project[i]) {
		case Human:
			player = new GuiPlayer(this, _board); break;
		case Machine:
			player = new AiPlayer(this); break;
		}
		delete _players[i];
		_players[i] = player;
	}
}
