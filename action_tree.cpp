#include "action_tree.h"
#include "board_state.h"
#include <QQueue>

bool ActionTree::init(BoardState board, Cell from) {

	struct Data {
		BoardState board;
		Cell from;
		int current;
		Data(const BoardState& b, Cell f, int c) : board(b), from(f), current(c) {}
		Data(BoardState&& b, Cell f, int c) : board(b), from(f), current(c) {}
	};

	if (!board.finished() || !board.control(from))
		return false;

	_tree.append(Progress());
	_tree.append(Progress(from));
	QQueue<Data> input;
	input.enqueue(Data(board, from, StartIndex));
	while (!input.isEmpty()) {
		Data d = input.dequeue();
		int i = 0;
		for (Cell cell : Cell::neighbours(d.from)) {
			BoardState copy = d.board;
			if (copy.control(cell)) {
				if (cell.valid()) {
					int daughter = _tree.size();
					_tree[d.current].next[i++] = daughter;
					_tree.append(Progress(cell));
					input.enqueue(Data(std::move(copy), cell, daughter));
				}
				else
					_tree[d.current].next[i++] = QuitIndex;
			}
		}
	}
	return true;
}

void ActionTree::clear() {
	_tree.clear();
}

int ActionTree::count(Index place) const {
	if (!validIndex(place))
		return 0;
	Progress p = _tree[place];
	int c = 0;
	for (int i = 0; i < MaxBranchFactor; ++ i)
		if (p.next[i] != InvalidIndex)
			c ++;
	return c;
}

ActionTree::Index ActionTree::next(Index place, int at) const {
	if (!validIndex(place))
		return InvalidIndex;
	if (at < 0 || at >= MaxBranchFactor)
		return InvalidIndex;
	return _tree[place].next[at];
}

Cell ActionTree::cell(Index place) const {
	if (!validIndex(place))
		return Cell();
	return _tree[place].cell;
}

bool ActionTree::automatic(Index place) const {
	if (!validIndex(place) || place == QuitIndex)
		return false;
	Progress p = _tree[place];
	int c = 0;
	for (int i = 0; i < MaxBranchFactor; ++ i)
		if (p.next[i] != InvalidIndex && cell(p.next[i]).valid())
			c ++;
	return c <= 1;
}

ActionTree::Index ActionTree::advance(Index place) const {
	if (!validIndex(place))     // передача хода сопернику — обладает приоритетом
		return InvalidIndex;
	Progress p = _tree[place];
	for (int i = 0; i < MaxBranchFactor; ++ i)
		if (p.next[i] == QuitIndex)
			return QuitIndex;
	for (int i = 0; i < MaxBranchFactor; ++ i)
		if (p.next[i] != InvalidIndex)
			return p.next[i];
	return QuitIndex;
}

bool ActionTree::validIndex(Index i) const {
	return i >= 0 && i < _tree.length();
}
