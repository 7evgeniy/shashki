#include "minimax.h"

const double WhiteWin = 50.0;
const double BlackWin = 1.0 / 50.0;
const int KingPrice = 3;
const int ManPrice = 1;
const int MaxLevel = 7;

std::vector<std::vector<Cell>> explore(const BoardState& board, std::vector<Cell> action) {
	if (!action.back().valid())
		return {action};
	std::vector<std::vector<Cell>> result;
	for (Cell cell : Cell::neighbours(action.back())) {
		BoardState copy = board;
		if (copy.control(cell)) {
			std::vector<Cell> completion = action;
			completion.insert(completion.end(), cell);
			auto append = explore(copy, completion);
			result.insert(result.end(), append.begin(), append.end());
		}
	}
	return result;
}

std::vector<std::vector<Cell>> explore(const BoardState &board) {
	std::vector<std::vector<Cell>> result;
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

double evaluate (const BoardState &board) {
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

double white(const BoardState& board, int level, double alpha, double beta);
double black(const BoardState& board, int level, double alpha, double beta);

double white(const BoardState& board, int level, double alpha, double beta) {
	if (level <= 0 && board.quiet())
		return evaluate(board);
	std::vector<std::vector<Cell>> every = explore(board);
	if (every.empty())
		return BlackWin;
	double result = BlackWin / 2.0;
	for (auto action : every) {
		BoardState copy = board;
		BoardState::apply(copy, action);
		double value = black(copy, level-1, alpha, beta);
		if (value > beta)
			return value;
		if (value > alpha)
			alpha = value;
		if (value > result)
			result = value;
	}
	return result;
}

double black(const BoardState& board, int level, double alpha, double beta) {
	if (level <= 0 && board.quiet())
		return evaluate(board);
	std::vector<std::vector<Cell>> every = explore(board);
	if (every.empty())
		return WhiteWin;
	double result = WhiteWin * 2.0;
	for (auto action : every) {
		BoardState copy = board;
		BoardState::apply(copy, action);
		double value = white(copy, level-1, alpha, beta);
		if (value < alpha)
			return value;
		if (value < beta)
			beta = value;
		if (value < result)
			result = value;
	}
	return result;
}

std::vector<Cell> minimax(BoardState board) {
	std::vector<std::vector<Cell>> actions = explore(board);
	if (actions.empty() || !board.color().valid())
		return {};
	if (actions.size() == 1)
		return actions[0];
	int index = 0;
	double alpha = BlackWin, beta = WhiteWin;
	if (board.color() == Role::White) {
		for (unsigned int i = 0; i < actions.size(); ++ i) {
			BoardState copy = board;
			BoardState::apply(copy, actions[i]);
			double value = black(copy, MaxLevel, alpha, beta);
			if (value > alpha) {
				alpha = value;
				index = i;
			}
		}
	}
	else {
		for (unsigned int i = 0; i < actions.size(); ++ i) {
			BoardState copy = board;
			BoardState::apply(copy, actions[i]);
			double value = white(copy, MaxLevel, alpha, beta);
			if (value < beta) {
				beta = value;
				index = i;
			}
		}
	}
	return actions[index];
}
