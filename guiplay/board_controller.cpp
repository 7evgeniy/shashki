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
				std::vector<Cell> action = convertArray(prefix);
				BoardState::apply(_board, action);
				_control->setPosition(_board.position());
				reset();
				emit moved(action);
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
					Cell next = cell.neighbour(d);
					if (copy.control(next)) {
						next.setCapture(copy.capture());
						move->appendHead(head, next);
					}
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

int captureCount(QList<Cell> action, Cell at) {
	int count = 0;
	for (int i = 0; i < action.size(); ++ i) {
		if (action[i] == at)
			break;
		if (action[i].capture())
			++ count;
	}
	return count;
}

class HeadData {
public:
	void appendHead(Root<Cell>::Iterator head, int count) {
		heads.append(head);
		counts.append(count);
	}

	QList<Root<Cell>::Iterator> extractMinCount() {
		int minCount = -1;
		for (int count: counts)
			if (minCount < 0 || minCount > count)
				minCount = count;
		QList<Root<Cell>::Iterator> extraction;
		for (int i = 0; i < heads.size(); ++ i)
			if (counts[i] == minCount)
				extraction.append(heads[i]);
		return extraction;
	}

private:
	QList<Root<Cell>::Iterator> heads;
	QList<int> counts;
};

// Оставить лишь те вершины, в которые можно попасть через поле at.
// Из них взять лишь вершины, где шашка доходит до at за минимальное число взятий.
QList<Root<Cell>::Iterator> BoardController::restrictHeads(Cell at) const {
	HeadData data;
	if (!_move || !at.valid())
		return QList<Root<Cell>::Iterator>();
	for (Root<Cell>::Iterator head: _move->heads()) {
		QList<Cell> action = _move->makeList(head);
		int count = captureCount(action, at);
		if (!action[_count].valid() && action[_count-1] == at)
			data.appendHead(head, count);
		for (int i = _count; i < action.size(); ++ i)
			if (action[i] == at)
				data.appendHead(head, count);
	}
	return data.extractMinCount();
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
