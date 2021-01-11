#ifndef AI_PLAYER_H
#define AI_PLAYER_H

#include "player.h"
#include "board_state.h"
#include <vector>
#include <QFutureWatcher>

class AiPlayer : public Player {
	Q_OBJECT
public:
	AiPlayer(Game *game);
	void setAbility(double ability);
	void activate(const BoardState &board);
	void done();
private:
	typedef std::vector<Cell> Action;
	typedef std::vector<Action> ActionList;
private:
	QFutureWatcher<Action> _watcher;
	QFuture<Action> _future;
	double _ability;        // склонность игрока правильно оценивать позиции.
private:
	static ActionList explore(const BoardState &board);
	static ActionList explore(const BoardState& board, Action action);
	static Action traverse(BoardState board);
	static double white(const BoardState &board, int level, double alpha, double beta);
	static double black(const BoardState &board, int level, double alpha, double beta);
	static double evaluate(const BoardState &board);
private:
	static const double WhiteWin;
	static const double BlackWin;
	static const int ManPrice;
	static const int KingPrice;
	static const int MaxLevel;
};

#endif
