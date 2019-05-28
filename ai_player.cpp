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
	PlayInfo play;
	play.actions = explore(board);
	if (play.actions.empty() || !board.color().valid())
		return {};
	if (play.actions.size() == 1)
		return play.actions.front();
	double alpha = BlackWin, beta = WhiteWin;
	play.isWhite = (board.color() == Role::White);
	for (unsigned int i = 0; i < play.actions.size(); ++ i) {
		BoardState copy = board;
		BoardState::apply(copy, play.actions[i]);
		double value = minimax(copy, MaxLevel, alpha, beta);
		play.quality.push_back(value);
		if (play.isWhite && value > alpha)
			alpha = value;
		if (!play.isWhite && value < beta)
			beta = value;
	}
	return randomisePlay(play);
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
	return static_cast<double>(wc)/static_cast<double>(bc);
}

// 1. Отсортировать ходы по возрастанию качества;
// 2. Наиболее вероятный ход — в конце массива для белых, в начале массива для чёрных.
AiPlayer::Action AiPlayer::randomisePlay(PlayInfo play) {
	for (unsigned int i = 0; i < play.quality.size(); ++i) {
		unsigned int min = i+1;
		for (unsigned int j = i+2; j < play.quality.size(); ++ j)
			if (play.quality[min] > play.quality[j])
				min = j;
		if (min < play.quality.size() && play.quality[i] > play.quality[min]) {
				std::swap(play.quality[i], play.quality[min]);
				std::swap(play.actions[i], play.actions[min]);
		}
	}
	static std::random_device RD;
	static std::default_random_engine RE(RD());
	static std::geometric_distribution<unsigned int> UD(play.ability);
	unsigned int i = UD(RE);
	if (i >= play.actions.size())
		i = 0;
	if (play.isWhite)
		i = play.actions.size()-i-1;
	return play.actions[i];
}

const double AiPlayer::WhiteWin = 50.0;
const double AiPlayer::BlackWin = 1/50.0;
const int AiPlayer::ManPrice = 1;
const int AiPlayer::KingPrice = 3;
const int AiPlayer::MaxLevel = 7;
