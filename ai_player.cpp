#include "ai_player.h"
#include "game.h"
#include <QtConcurrent>
#include <random>

AiPlayer::AiPlayer(Game *game) : Player(game) {
	auto finished = &QFutureWatcher<Action>::finished;
	connect(&_watcher, finished, this, &AiPlayer::done);
}

void AiPlayer::activate(const BoardState& board) {
	_future = QtConcurrent::run(traverse, board);
	_watcher.setFuture(_future);
}

void AiPlayer::done() {
	QList<Cell> action;
	for (Cell cell : _future.result())
		action.append(cell);
	_game->move(action);
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

AiPlayer::Action AiPlayer::traverse (BoardState board) {
	std::vector<std::vector<Cell>> actions = explore(board);
	std::vector<Cell> result = actions.front();
	if (actions.empty() || !board.color().valid())
		return {};
	if (actions.size() == 1)
		return result;
	double alpha = BlackOverflow, beta = WhiteOverflow;
	bool isWhite = (board.color() == Role::White);
	for (unsigned int i = 0; i < actions.size(); ++ i) {
		BoardState copy = board;
		BoardState::apply(copy, actions[i]);
		if (!isWhite) {
			double value = white(copy, MaxLevel, alpha, beta);
			if (value < beta) {
				beta = value;
				result = actions[i];
			}
		}
		if (isWhite) {
			double value = black(copy, MaxLevel, alpha, beta);
			if (value > alpha) {
				alpha = value;
				result = actions[i];
			}
		}
	}
	return result;
}

double AiPlayer::white(const BoardState &board, int level, double alpha, double beta) {
	if (level <= 0 && board.quiet())
		return evaluate(board, alpha, beta);
	ActionList every = explore(board);
	if (every.empty())
		return BlackWin;
	double result = BlackOverflow / 2;
	for (auto action : every) {
		BoardState copy = board;
		BoardState::apply(copy, action);
		double value = black(copy, level-1, alpha, beta);
		if (value < BlackOverflow)
			continue;
		if (value > WhiteOverflow)
			return value;
		if (value > alpha)
			result = alpha = value;
	}
	return result;
}

double AiPlayer::black(const BoardState &board, int level, double alpha, double beta) {
	if (level <= 0 && board.quiet())
		return evaluate(board, alpha, beta);
	ActionList every = explore(board);
	if (every.empty())
		return WhiteWin;
	double result = WhiteOverflow * 2;
	for (auto action : every) {
		BoardState copy = board;
		BoardState::apply(copy, action);
		double value = white(copy, level-1, alpha, beta);
		if (value < BlackOverflow)
			return value;
		if (value > WhiteOverflow)
			continue;
		if (value < beta)
			result = beta = value;
	}
	return result;
}

double AiPlayer::evaluate (const BoardState &board, double alpha, double beta) {
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
	double result = static_cast<double>(wc)/static_cast<double>(bc);
	if (result < alpha)
		return BlackOverflow / 2;
	if (result > beta)
		return WhiteOverflow * 2;
	return result;
}

const double AiPlayer::WhiteWin = 50.0;
const double AiPlayer::BlackWin = 1/50.0;
const double AiPlayer::WhiteOverflow = AiPlayer::WhiteWin * 2;
const double AiPlayer::BlackOverflow = AiPlayer::BlackWin / 2;
const int AiPlayer::ManPrice = 1;
const int AiPlayer::KingPrice = 3;
const int AiPlayer::MaxLevel = 7;
