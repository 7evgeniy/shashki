#include "ai_player.h"
#include "game.h"
#include <QtConcurrent>
#include <random>

AiPlayer::AiPlayer(Game *game) : Player(game), _ability(0.9) {
	auto finished = &QFutureWatcher<Action>::finished;
	connect(&_watcher, finished, this, &AiPlayer::done);
}

void AiPlayer::setAbility(double ability) {
	if (ability > 0.0 && ability < 1.0)
		_ability = ability;
}

void AiPlayer::activate(const BoardState& board) {
	_future = QtConcurrent::run(traverse, board);
	_watcher.setFuture(_future);
}

void AiPlayer::done() {
	Action result = select(_future.result());
	QList<Cell> action;
	for (Cell cell : result)
		action.append(cell);
	_game->move(action);
}

AiPlayer::Action AiPlayer::select(ActionList actions) {
	if (actions.size() == 1)
		return actions.back();
	static std::random_device RD;
	static std::default_random_engine RE(RD());
	static std::uniform_real_distribution<double> UD(0.0, 1.0);
	double choice = UD(RE);
	if (choice <= _ability)
		return actions.back();
	choice = (choice-_ability)/(1.0-_ability);
	int index = choice * (actions.size()-1);
	return actions[index];
}

AiPlayer::ActionList AiPlayer::explore(const BoardState& board) {
	ActionList result;
	std::vector<Cell> stock = board.position().stock(board.color());
	for (Cell cell : stock) {
		BoardState copy = board;
		if (copy.control(cell)) {
			auto append = explore(copy, {{cell}});
			result.insert(result.end(), append.begin(), append.end());
		}
	}
	return result;
}

AiPlayer::ActionList AiPlayer::explore(const BoardState& board, AiPlayer::Action action) {
	if (!action.back().valid())
		return {action};
	ActionList result;
	for (Cell cell : Cell::neighbours(action.back())) {
		BoardState copy = board;
		if (copy.control(cell)) {
			Action completion = action;
			completion.insert(completion.end(), cell);
			auto append = explore(copy, completion);
			result.insert(result.end(), append.begin(), append.end());
		}
	}
	return result;
}

// Процедура возвращает список всех возможных ходов,
// но ставит ход с наилучшей гарантией в конец списка.
AiPlayer::ActionList AiPlayer::traverse (BoardState board) {
	ActionList actions = explore(board);
	int index = 0;
	if (actions.empty() || !board.color().valid())
		return {};
	if (actions.size() == 1)
		return actions;
	double alpha = BlackOverflow, beta = WhiteOverflow;
	for (unsigned int i = 0; i < actions.size(); ++ i) {
		BoardState copy = board;
		BoardState::apply(copy, actions[i]);
		if (board.color() == Role::White) {
			double value = black(copy, MaxLevel, alpha, beta);
			if (value != 0.0 && value > alpha) {
				alpha = value;
				index = i;
			}
		}
		if (board.color() == Role::Black) {
			double value = white(copy, MaxLevel, alpha, beta);
			if (value != 0.0 && value < beta) {
				beta = value;
				index = i;
			}
		}
	}
	std::swap(actions[index], actions.back());
	return actions;
}

// найти, какую гарантию позиция даёт белым, если ход принадлежит белым.
double AiPlayer::white(const BoardState &board, int level, double alpha, double beta) {
	if (level <= 0 && board.quiet())
		return evaluate(board);
	ActionList every = explore(board);
	if (every.empty())
		return BlackWin;
	double result = 0.0;
	for (auto action : every) {
		BoardState copy = board;
		BoardState::apply(copy, action);
		double value = black(copy, level-1, alpha, beta);
		if (value == 0.0)    // если гарантия не вычислена, игнорировать
			continue;
		if (value > beta)    // если гарантия заведомо лучше уже полученной, не вычислять
			return 0.0;
		if (value > alpha)
			alpha = value;     // граница на наилучшую гарантию для чёрных
		if (result == 0.0 || value > result)
			result = value;
	}
	return result;
}

double AiPlayer::black(const BoardState &board, int level, double alpha, double beta) {
	if (level <= 0 && board.quiet())
		return evaluate(board);
	ActionList every = explore(board);
	if (every.empty())
		return WhiteWin;
	double result = 0.0;
	for (auto action : every) {
		BoardState copy = board;
		BoardState::apply(copy, action);
		double value = white(copy, level-1, alpha, beta);
		if (value == 0.0)
			continue;
		if (value < alpha)
			return 0.0;
		if (value < beta)
			beta = value;
		if (result == 0.0 || value < result)
			result = value;
	}
	return result;
}

double AiPlayer::evaluate (const BoardState &board) {
	bool isWhite = board.color() == Role::White;
	if (board.lost())
		return isWhite ? BlackWin : WhiteWin;
	std::vector<Cell> white = board.position().stock(Role::White);
	std::vector<Cell> black = board.position().stock(Role::Black);
	int wc = 0, bc = 0;
	for (Cell cell : white)
		wc += board.position().king(cell) ? KingPrice : ManPrice;
	for (Cell cell : black)
		bc += board.position().king(cell) ? KingPrice : ManPrice;
	return static_cast<double>(wc)/static_cast<double>(bc);
}

const double AiPlayer::WhiteWin = 50.0;
const double AiPlayer::BlackWin = 1/50.0;
const double AiPlayer::WhiteOverflow = AiPlayer::WhiteWin * 2.0;
const double AiPlayer::BlackOverflow = AiPlayer::BlackWin / 2.0;
const int AiPlayer::ManPrice = 1;
const int AiPlayer::KingPrice = 3;
const int AiPlayer::MaxLevel = 7;
