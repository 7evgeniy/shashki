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

bool Game::inner(int head, int depth) const {
	if (depth != 0)
		return true;
	auto at = find(head, depth);
	for(auto it : _game.heads())
		if (at != it && doesReach(at, it))
			return true;
	return false;
}

BoardState Game::at(int head, int depth) const {
	return _game[find(head, depth)];
}

void Game::fork(int head, int depth) {
	_game.markHead(find(head, depth));
}

BoardState Game::cut(int head, int depth) {
	auto heads = _game.heads();
	auto at = find(head, depth);
	for(auto it : heads)
		if (doesReach(at, it))
			_game.unmarkHead(it);
	_game.markHead(at);
	return _game[at];
}

BoardState Game::evolve(int head, std::vector<Cell> action) {
	BoardState board = _game[_game.heads()[head]];
	BoardState::apply(board, action);
	board = _game[_game.appendHead(_game.heads()[head], board)];
	_game.unmarkHead(_game.heads()[head]);
	return board;
}

Root<BoardState>::Iterator Game::find(int head, int depth) const {
	Root<BoardState>::Iterator it = _game.heads()[head];
	for (int c = 0; c < depth; ++ c)
		it = _game.prev(it);
	return it;
}

bool Game::doesReach(Root<BoardState>::Iterator from, Root<BoardState>::Iterator to) const {
	while (!to.source()) {
		if (to == from)
			return true;
		to = _game.prev(to);
	}
	return false;
}

#include "root.tpp"
