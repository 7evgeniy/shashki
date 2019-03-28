#include "action_builder.h"
#include <QQueue>

struct SelectFunction {
private:
	Cell _target;
	const ActionTree *_tree;
	QList<ActionTree::Index> _now;
	QList<ActionTree::Index> _finally;
	bool _unique;
	int _count;

public:
	SelectFunction(Cell target, ActionTree::Index head, const ActionTree *tree) {
		_target = target;
		_tree = tree;
		_count = 0;
		_now = {head};
	}

public:
	void traverse() {
		traverse(0);
	}

public:
	QList<ActionTree::Index> result() {
		return _unique ? _finally : QList<ActionTree::Index>();
	}

private:
	void traverse(int cnt) {
		if (_tree->cell(_now.last()).capture())
			++ cnt;
		if (_tree->cell(_now.last()) == _target) {
			if (_finally.isEmpty() || _count > cnt) {
				_finally = _now;
				_count = cnt;
				_unique = true;
			}
			else if (_count == cnt)
				_unique = false;
		}
		for (int i = 0; i < ActionTree::MaxBranchFactor; ++ i)
			if (_tree->next(_now.last(), i) != ActionTree::InvalidIndex) {
				_now.append(_tree->next(_now.last(), i));
				traverse(cnt);
			}
		_now.takeLast();
	}
};

ActionBuilder::ActionBuilder(const ActionTree *tree) :
	_tree(tree), _start(ActionTree::InvalidIndex) {}

void ActionBuilder::init(ActionTree::Index start) {
	_start = start;
	_past.clear();
	_future.clear();
}

void ActionBuilder::clear() {
	_past.clear();
	_start = ActionTree::InvalidIndex;
}

bool ActionBuilder::empty() const {
	return _past.isEmpty() && _start == ActionTree::InvalidIndex;
}

ActionTree::Index ActionBuilder::head() const {
	return _past.isEmpty() ? _start : _past.last();
}

QList<ActionTree::Index> ActionBuilder::past() const {
	return _past;
}

QList<ActionTree::Index> ActionBuilder::future() const {
	return _future;
}

QList<ActionTree::Index> ActionBuilder::action() const {
	if (_past.isEmpty())
		return {};
	QList<ActionTree::Index> result({_start});
	result.append(_past);
	return result;
}

Cell ActionBuilder::seed() const {
	return _tree->cell(_start);
}

void ActionBuilder::append(QList<ActionTree::Index> action) {
	_past.append(action);
}

bool ActionBuilder::navigate(Cell target) {
	if (!target.valid()) {
		_future.clear();
		return false;
	}
	_future = select(target);       // добраться до требуемого поля
	if (_future.isEmpty())
		return false;
	_future.takeFirst();            // убрать дублирование исходного поля
	_future.append(prolong(_future.isEmpty() ? _start : _future.last()));
	return true;               // добавить все дальнейшие необходимые движения
}

// выборка начальных фрагментов ходов, проходящих через поле:
// процедура отбирает единственный ход с наименьшим числом взятий (если есть).
QList<ActionTree::Index> ActionBuilder::select(Cell target) {
	SelectFunction function(target, head(), _tree);
	function.traverse();
	return function.result();
}

QList<ActionTree::Index> ActionBuilder::prolong(ActionTree::Index start) {
	QList<ActionTree::Index> result;
	while (_tree->automatic(start)) {
		start = _tree->advance(start);
		result.append(start);
	}
	return result;
}
