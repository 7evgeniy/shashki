#include "board_state.h"
#include "segment.h"

bool BoardState::apply(BoardState &board, std::vector<Cell> action) {
	for (Cell cell : action)
		if (!board.control(cell))
			return false;
	return true;
}

BoardState BoardState::initialBoard() {
	Position::Stone cells[32];
	Position::Stone white(Role::White);
	Position::Stone black(Role::Black);
	Position::Stone empty(Role::None);
	int i = 0;
	while (i < 12)
			cells[i++] = white;
	while (i < 20)
			cells[i++] = empty;
	while (i < 32)
			cells[i++] = black;
	return BoardState(Position(cells), Role::White);
}

BoardState::BoardState () : _color(Role::None) {}
BoardState::BoardState (const Position &position, Role start)
	: _position(position), _color(start) {forage();}
const Position& BoardState::position () const {return _position;}
Role BoardState::color () const {return _color;}
Cell BoardState::place () const {return _view.empty() ? _start : _view[_location].cell;}
bool BoardState::capture () const {return !_view.empty() && _view[_location].cell.capture();}
bool BoardState::finished () const {return !_start.valid();}
bool BoardState::quiet () const {return finished() && !_hungry;}

/*
 * Движение шашки разделено на сегменты, когда шашка проходит по одной диагонали.
 * Доска задействует шашку по процедуре startStone(), начинает сегмент по процедуре
 * installDirection(), двигает шашку по сегменту по процедуре advanceLocation(),
 * закрывает последний сегмент полухода по процедуре stopStone().
 */

bool BoardState::control (Cell location) {
	if (!_color.valid())
		return false;
	if (!location.valid())
		return stopStone();
	if (!_start.valid())
		return startStone(location);
	Direction direction = place().connection(location);
	if (direction == Direction::None || direction == _direction.opposite())
		return false;
	if (direction == _direction)
		return advanceLocation();
	else
		return installDirection(direction) ? advanceLocation() : false;
}

bool BoardState::startStone (Cell start) {
	if (imprudentStart(start))     // Захватить шашку, чтобы позже начать ход, создав секцию:
		return false;
	_start = start;
	return true;
}

bool BoardState::installDirection (Direction direction) {
	if (wrongDigression(direction))       // Создать секцию при повороте или при начале хода:
		return false;
	_direction = direction;
	_view = browse(place(), _direction);
	_location = 0;
	return true;
}

bool BoardState::advanceLocation () {    // Совершить проход до очередного поля секции:
	if (crazyAdvance())
		return false;
	Cell from = _view[_location].cell;
	Cell to = _view[++_location].cell;
	if (to.capture()) {
		_position.kill(to);
		return true;
	}
	if (from.capture())
		from = from.neighbour(_direction.opposite());
	_position.move(from, to);
	if (to.promotion(_color))
		_position.promote(to);
	return true;
}

bool BoardState::stopStone () {         // Совершить останов шашки:
	if (untimelyStop())
		return false;
	_position.removeGhosts();
	_view.clear();
	_start = Cell();
	_direction = Direction();
	_color = _color.opposite();
	forage();
	return true;
}

bool BoardState::imprudentStart (Cell location) const {
	if (_start.valid())
		return true;
	if (_position.color(location) != _color)
		return true;
	if (_hungry && !hungry(location))
		return true;
	if (!free(location))
		return true;
	return false;
}

bool BoardState::wrongDigression (Direction direction) const {
	if (!_start.valid())
		return true;
	if (!_direction.valid())
		return _hungry && !_position.captures(place(), direction);
	switch (_direction.derivator(direction)) {
	case Direction::Turn::GoLeft: return !_view[_location].left;
	case Direction::Turn::GoRight: return !_view[_location].right;
	default: return true;
	}
}

bool BoardState::crazyAdvance () const {
	if (_view.empty())
		return true;
	if (_location+1 == static_cast<int>(_view.size()))
		return true;
	return false;
}

bool BoardState::untimelyStop () const {
	if (_view.empty())
		return true;
	if (!_view[_location].stop)
		return true;
	return false;
}

bool BoardState::lost () const {
	if (_start.valid())
		return false;
	std::vector<Cell> stones = _position.stock(_color);
	for (Cell stone : stones)
		if (free(stone))
			return false;
	return true;
}

void BoardState::forage () {
	_hungry = false;
	std::vector<Cell> stones = _position.stock(_color);
	for (Cell stone : stones)
		if (hungry(stone)) {
			_hungry = true;
			return;
		}
}

bool BoardState::free (Cell stone) const {
	const std::vector<Direction>& directions = Direction::enumerate();
	for (Direction direction : directions)
		if (_position.accepts(stone, direction) != Position::Block)
			return true;
	return false;
}

bool BoardState::hungry (Cell stone) const {
	const std::vector<Direction>& directions = Direction::enumerate();
	for (Direction direction : directions)
		if (_position.captures(stone, direction))
			return true;
	return false;
}

/*
 * Процедура, проверяющая действия, возможные во время прохода по тропинке.
 *
 * Процедура сначала собирает все поля, до которых в принципе может добраться
 * шашка, следующая всем правилам, кроме правила, что нельзя избегать взятий.
 * Она при этом устанавливает первое поле, по достижении которого шашка больше
 * не может совершать взятия по сторонам, и первое поле, по достижении которого
 * шашка больше не может не может совершать взятия впереди. В следующем цикле
 * процедура, собрав эти сведения, устанавливает правило неизбежности взятий.
 *
 * Если брать вбок шашка уже не может, хотя могла на предшествующих полях,
 * и брать вперёд тоже уже не может, то процедура должна выбросить все поля,
 * начиная с этого. Если брать вбок или брать вперёд шашка пока может, то
 * процедура должна сбросить возможность остановить ход. Если шашка не сошла
 * пока с места в этом сегменте, то процедура должна сбросить возможность
 * остановить ход для этого сегмента. Пока шашка перескакивает через шашку,
 * она ещё может брать вперёд: на это поле она зайти обязательно может.
 */

std::vector<BoardState::Location> BoardState::browse (Cell start, Direction path) const {
	std::vector<Location> answer;
	Segment segment(_position, start, path);
	Role color = _position.color(start);
	bool king = _position.king(start);

	Cell oblique = start;      // Поле, по достижении которого нельзя брать вбок.
	Cell forward = start;      // Поле, по достижении которого нельзя брать вперёд.
	bool slow = true;       // Шашка может пока совершать тихие ходы.
	bool capture = true;    // Шашка может пока перескакивать через шашки.
	bool escape = false;    // Шашка проходит поля после первого взятия.
	bool extend = false;    // На прошлом поле шашка перескочила шашку.
	for (Cell action : segment) {
		bool left = false, right = false;
		if (escape && !action.capture() && capture) {
			Direction leftwards = path.derive(Direction::Turn::GoLeft);
			Direction rightwards = path.derive(Direction::Turn::GoRight);
			left = _position.captures(action, leftwards, color, king);
			right = _position.captures(action, rightwards, color, king);
		}
		if (action.capture() && !capture)
			break;
		if (!action.capture() && !slow && !extend)
			break;
		answer.push_back(Location(action, left, right));
		if (left || right)
			oblique = action.neighbour(path);
		if (action.capture()) {
			extend = escape = true;
			forward = action.neighbour(path);
		} else
			extend = false;
		if (!king && action == start.neighbour(path)) {
			slow = false;
			if (!action.capture())
				capture = false;
		}
	}

	bool obliqueHungry = true, forwardHungry = true;
	unsigned int i;
	for (i = 0; i < answer.size(); ++ i) {
		if (answer[i].cell == forward)
			forwardHungry = false;
		if (answer[i].cell == oblique) {
			obliqueHungry = false;
			if (!forwardHungry && i != 0)
				break;
		}
		if (i == 0 || forwardHungry || obliqueHungry)
			answer[i].stop = false;
	}
	answer.erase(answer.begin()+i, answer.end());
	return answer;
}
