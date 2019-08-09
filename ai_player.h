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
public:
	static double DefaultAbility;    // сила игры, присваиваемая игроку в конструкторе
private:
	typedef std::vector<Cell> Action;
	typedef std::vector<Action> ActionList;
private:
	QFutureWatcher<ActionList> _watcher;
	QFuture<ActionList> _future;
	double _ability;
private:
	static ActionList explore(const BoardState &board);
	static ActionList explore(const BoardState& board, Action action);
	static ActionList traverse(BoardState board);
	static double white(const BoardState &board, int level, double alpha, double beta);
	static double black(const BoardState &board, int level, double alpha, double beta);
	static double evaluate(const BoardState &board, double alpha, double beta);
	Action select(ActionList actions);
private:
	static const double WhiteWin;
	static const double BlackWin;
	static const double WhiteOverflow;
	static const double BlackOverflow;
	static const int ManPrice;
	static const int KingPrice;
	static const int MaxLevel;
};

#endif
