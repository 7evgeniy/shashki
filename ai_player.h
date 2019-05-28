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
	double _ability;     // Вероятность выбрать ход, если он хороший; из интервала (0; 1)
private:
	struct PlayInfo {
		ActionList actions;
		std::vector<double> quality;
		bool isWhite;
		double ability;
	};
	static ActionList explore(const BoardState &board);
	static ActionList explore(const BoardState& board, Action action);
	static Action traverse (BoardState board);
	static double minimax (const BoardState &board, int level, double alpha, double beta);
	static double evaluate (const BoardState &board);
	static Action randomisePlay(PlayInfo play);
private:
	static const double WhiteWin;
	static const double BlackWin;
	static const int ManPrice;
	static const int KingPrice;
	static const int MaxLevel;
};

#endif
