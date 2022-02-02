#include "game.h"

Game::Game() : _game(BoardState::initialBoard()) {}

bool Game::empty() const {
	for (Root<BoardState>::Iterator it: _game.heads())
		if (_game.makeList(it).size() > 1)
			return false;
	return true;
}

int Game::length(int head) const {
	return _game.makeList(_game.heads()[head]).size();
}

BoardState Game::at(int head, int depth) const {
	auto list = _game.makeList(_game.heads()[head]);
	return list[list.size()-depth-1];
}

void Game::cut(int head, int depth) {
	Root<BoardState>::Iterator it = _game.heads()[head];
	for (int c = 0; c < depth; ++ c)
		it = _game.prev(it);
	_game.markHead(it);
}

BoardState Game::evolve(int head, std::vector<Cell> action) {
	BoardState board = _game[_game.heads()[head]];
	BoardState::apply(board, action);
	board = _game[_game.appendHead(_game.heads()[head], board)];
	_game.unmarkHead(_game.heads()[head]);
	return board;
}

#include "root.tpp"
