#ifndef ACTION_TREE_H
#define ACTION_TREE_H

#include <QList>
#include "board_state.h"

class ActionTree {
public:
	typedef int Index;
public:
	bool init(BoardState board, Cell from);
	void clear();
	bool empty() const;
	int count(Index place) const;
	Index next(Index place, int at) const;
	Cell cell(Index place) const;
public:
	bool automatic(Index place) const;   // «есть возможность автоматически достроить ход»
	Index advance(Index place) const;    // получить движение, автоматически достраивающее ход
public:
	static const Index StartIndex = 1;
	static const Index InvalidIndex = -1;
	static const Index QuitIndex = 0;
	static const int MaxBranchFactor = 4;
private:
	bool validIndex(Index i) const;
	struct Progress {
		Cell cell;
		Index next[MaxBranchFactor];
		Progress(Cell c = Cell()) : cell(c) {
			for (int i = 0; i < MaxBranchFactor; ++ i)
				next[i] = InvalidIndex;
		}
	};
	QList<Progress> _tree;
};

#endif
