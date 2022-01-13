#include "board_controller.h"

std::vector<Cell> convertArray(QList<Cell> in) {
	std::vector<Cell> out;
	out.reserve(in.size());
	for (Cell cell: in)
		out.push_back(cell);
	return out;
}

BoardController::BoardController(QObject *parent)
	: QObject(parent), _control(nullptr), _move(nullptr),
	  _count(0), _board(BoardState::initialBoard()) {}

void BoardController::setControl(BoardWidget *control) {
	_control = control;
	if (_control)
		_control->setPosition(_board.position());
}

void BoardController::setBoard(BoardState board) {
	_board = board;
	if (_control)
		_control->setPosition(_board.position());
}

void BoardController::click(Cell at) {
	if (!at.valid()) {
		reset();
		return;
	}
	if (!_count) {
		_move = makeRoot(at);
		if (_move) {
			++ _count;
			_heads = _move->heads();
			_control->markBorder(at);
		}
	}
	else {
		_heads = restrictHeads(at);
		if (!_heads.isEmpty()) {
			QList<Cell> prefix = commonPrefix(_heads, at);
			if (prefix.last().valid()) {
				_count = prefix.size();
				_control->markGreen(prefix);
				_control->markBlue(QList<Cell>());
			}
			else {
				BoardState::apply(_board, convertArray(prefix));
				_control->setPosition(_board.position());
				reset();
			}
		}
		else
			reset();
	}
}

void BoardController::hover(Cell at) const {
	QList<Cell> prefix = commonPrefix(restrictHeads(at), at);
	if (prefix.isEmpty()) {
		_control->markBlue(QList<Cell>());
		return;
	}
	int greenCount = _control->green().size();
	if (!prefix.last().valid()) {
		prefix.removeLast();
		if (prefix.size() == greenCount)
			-- greenCount;
	}
	prefix.erase(prefix.begin(), prefix.begin()+greenCount);
	_control->markBlue(prefix);
}

void BoardController::reset() {
	_control->unmark();
	_move = nullptr;
	_count = 0;
	_heads.clear();
}

// Составить дерево ходов; источник — начальный ход; вершины — завершения хода.
std::shared_ptr<Root<Cell>> BoardController::makeRoot(Cell at) const {
	BoardState test = _board;
	if (!test.control(at))
		return nullptr;
	std::shared_ptr<Root<Cell>> move(new Root<Cell>(at));
	bool empty;
	do {
		empty = true;
		for (Root<Cell>::Iterator head: move->heads()) {
			BoardState board = _board;
			BoardState::apply(board, convertArray(move->makeList(head)));
			Cell cell = (*move)[head];
			if (cell.valid()) {
				for (Direction d: Direction::enumerate()) {
					BoardState copy = board;
					if (copy.control(cell.neighbour(d)))
						move->appendHead(head, cell.neighbour(d));
				}
				if (board.control(Cell()))
					move->appendHead(head, Cell());
				move->unmarkHead(head);
				empty = false;
				break;
			}
		}
	} while (!empty);
	if (move->heads().empty())
		return nullptr;
	return move;
}

// Оставить лишь те вершины, в которые можно попасть через поле at.
QList<Root<Cell>::Iterator> BoardController::restrictHeads(Cell at) const {
	QList<Root<Cell>::Iterator> result;
	if (!_move || !at.valid())
		return result;
	for (Root<Cell>::Iterator head: _move->heads()) {
		QList<Cell> move = _move->makeList(head);
		if (!move[_count].valid() && move[_count-1] == at)
			result.append(head);
		for (int i = _count; i < move.size(); ++ i)
			if (move[i] == at)
				result.append(head);
	}
	return result;
}

// Общий префикс для всех вершин; добавить завершение хода, если уместно.
QList<Cell> BoardController::commonPrefix(QList<Root<Cell>::Iterator> heads, Cell at) const {
	QList<Cell> prefix;
	for (Root<Cell>::Iterator head: heads) {
		QList<Cell> action = _move->makeList(head);
		if (prefix.isEmpty())
			prefix = action;
		else {
			int i = 0;
			while (i < prefix.size() && i < action.size() && prefix[i] == action[i])
				++ i;
			bool stop = false;
			if (prefix[i-1] == at) {
				if (i < prefix.size() && !prefix[i].valid())
					stop = true;
				if (i < action.size() && !action[i].valid())
					stop = true;
			}
			prefix.erase(prefix.begin()+i, prefix.end());
			if (stop) {
				prefix.append(Cell());
				break;
			}
		}
	}
	return prefix;
}

#include "root.tpp"
