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
	ActionList every = explore(board);
	if (every.empty() || !board.color().valid())
		return {};
	if (every.size() == 1)
		return every.front();
	unsigned int choice = 0;
	double alpha = BlackWin, beta = WhiteWin;
	bool isWhite = (board.color() == Role::White);
	for (unsigned int i = 0; i < every.size(); ++ i) {
		BoardState copy = board;
		BoardState::apply(copy, every[i]);
		double value = minimax(copy, MaxLevel, alpha, beta);
		if (isWhite && value > alpha) {
			alpha = value;
			choice = i;
		}
		if (!isWhite && value < beta) {
			beta = value;
			choice = i;
		}
	}
	return every[choice];
}

double AiPlayer::minimax (const BoardState &board, int level, double alpha, double beta) {
	if (level <= 0 && board.quiet())
		return evaluate(board);
	ActionList every = explore(board);
	bool isWhite = (board.color() == Role::White);
	if (every.empty())
		return isWhite ? BlackWin : WhiteWin;
	for (auto action : every) {
		BoardState copy = board;
		BoardState::apply(copy, action);
		if (isWhite)
			alpha = std::max(alpha, minimax(copy, level-1, alpha, beta));
		if (!isWhite)
			beta = std::min(beta, minimax(copy, level-1, alpha, beta));
		if (alpha >= beta)
			break;
	}
	return isWhite ? alpha : beta;
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
	double value = randomise(static_cast<double>(wc)/static_cast<double>(bc));
	return isWhite ? value*MyMove : value/MyMove;
}

double AiPlayer::randomise (double value) {
	return value * (1 + random());
}

double AiPlayer::random() {
	static std::random_device RD;
	static std::default_random_engine RE(RD());
	static std::uniform_real_distribution<double> UD(RandMin, RandMax);
	return UD(RE);
}

const double AiPlayer::RandMin = -0.02;
const double AiPlayer::RandMax = 0.02;
const double AiPlayer::WhiteWin = 50.0;
const double AiPlayer::BlackWin = 1/50.0;
const double AiPlayer::MyMove = 1.05;
const int AiPlayer::ManPrice = 1;
const int AiPlayer::KingPrice = 3;
const int AiPlayer::MaxLevel = 7;
