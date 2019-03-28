#ifndef ACTION_BUILDER_H
#define ACTION_BUILDER_H

#include "action_tree.h"
#include "cell.h"
#include <QList>

class ActionBuilder {
public:
	ActionBuilder(const ActionTree *tree);
	void init(ActionTree::Index start);
	void clear();
	bool empty() const;
	ActionTree::Index head() const;
	QList<ActionTree::Index> past() const;
	QList<ActionTree::Index> future() const;
	QList<ActionTree::Index> action() const;
	Cell seed() const;
	bool navigate(Cell cell);
	void append(QList<ActionTree::Index> action);
private:
	QList<ActionTree::Index> select(Cell target);
	QList<ActionTree::Index> prolong(ActionTree::Index start);
private:
	const ActionTree *_tree;
	ActionTree::Index _start;
	QList<ActionTree::Index> _past;
	QList<ActionTree::Index> _future;
};

#endif
