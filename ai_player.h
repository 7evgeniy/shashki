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
	void activate(const BoardState &board);
	void done();
private:
	typedef std::vector<Cell> Action;
	typedef std::vector<Action> ActionList;
private:
	QFutureWatcher<Action> _watcher;
	QFuture<Action> _future;
private:
	static ActionList explore(const BoardState &board);
	static ActionList explore(const BoardState& board, Action action);
	static Action traverse (BoardState board);
	static double minimax (const BoardState &board, int level, double alpha, double beta);
	static double evaluate (const BoardState &board);
	static double randomise (double value);
	static double random ();
private:
	static const double RandMin;
	static const double RandMax;
	static const double WhiteWin;
	static const double BlackWin;
	static const double MyMove;
	static const int ManPrice;
	static const int KingPrice;
	static const int MaxLevel;
};

#endif
