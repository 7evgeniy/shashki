#include "game.h"

Game::Game() : _game(BoardState::initialBoard()) {}

int Game::length(int head) const {
	return _game.makeList(_game.heads()[head]).size();
}

BoardState Game::at(int head, int depth) const {
	return *(_game.makeList(_game.heads()[head]).rbegin()+depth);
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
	return _game[_game.appendHead(_game.heads()[head], board)];
}

#include "root.tpp"
