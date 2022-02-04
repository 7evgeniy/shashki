#include "board_widget.h"
#include "board_controller.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QFont>

BoardWidget::Environment BoardWidget::_env;

BoardWidget::BoardWidget() {
	_flipped = _active = false;
	setFocusPolicy(Qt::ClickFocus);
	setMouseTracking(true);
	setMinimumSize(400, 400);
	setEnabled(false);
	_controller = nullptr;
}

void BoardWidget::setController(BoardController *controller) {
	if (_controller)
		_controller->setControl(nullptr);
	_controller = controller;
	if (_controller)
		_controller->setControl(this);
	setEnabled(_controller);
}

const Position& BoardWidget::position() const {return _position;}
Position& BoardWidget::position() {return _position;}
void BoardWidget::setPosition(const Position &position) {_position = position; update();}
void BoardWidget::setFlipped(bool is) {_flipped = is; update();}
bool BoardWidget::flipped() const {return _flipped;}
void BoardWidget::setActive(bool is) {_active = is; update();}
bool BoardWidget::active() const {return _active;}

void BoardWidget::markBlue(QList<Cell> blue) {_blue = blue; update();}
void BoardWidget::markGreen(QList<Cell> green) {_green = green; update();}
void BoardWidget::markBorder(Cell border) {_border = border; update();}
void BoardWidget::unmark() {_blue.clear(); _green.clear(); _border = Cell(); update();}

QList<Cell> BoardWidget::green() const {return _green;}
QList<Cell> BoardWidget::blue() const {return _blue;}
Cell BoardWidget::border() const {return _border;}

void BoardWidget::keyPressEvent(QKeyEvent *e) {
	if (e->modifiers() != 0)
		e->ignore();
	else if (e->key() == Qt::Key_Escape)
		_controller->reset();
	else
		e->ignore();
}

void BoardWidget::mousePressEvent(QMouseEvent *e) {
	if (e->buttons() == Qt::LeftButton)
		_controller->click(locate(e->pos()));
	else
		_controller->reset();
}

void BoardWidget::mouseMoveEvent(QMouseEvent *e) {
	Cell cell = locate(e->pos());
	if (cell != _hover) {
		_hover = cell;
		_controller->hover(_hover);
	}
}

void BoardWidget::paintEvent(QPaintEvent *) {
	QPainter painter(this);
	painter.setViewport(makeViewport());
	painter.setWindow(-50, -50, 900, 900);
	painter.setFont(_env.LabelFont);
	for (int col = 0; col < 8; ++ col) {
		int file = _flipped ? 7-col : col;
		painter.drawText(col*100, -50, 100, 50, Qt::AlignCenter, QString(1, file+'a'));
		painter.drawText(col*100, 800, 100, 50, Qt::AlignCenter, QString(1, file+'a'));
	}
	for (int row = 0; row < 8; ++ row) {
		int rank = _flipped ? row : 7-row;
		painter.drawText(-50, row*100, 50, 100, Qt::AlignCenter, QString(1, rank+'1'));
		painter.drawText(800, row*100, 50, 100, Qt::AlignCenter, QString(1, rank+'1'));
		for (int col = 0; col < 8; ++ col) {
			int file = _flipped ? 7-col : col;
			Cell cell(file, rank);
			drawSquare(painter, row, col);
			if (_position.color(cell) != Role::None)
				drawStone(painter, QPoint(col*100 + 50, row*100 + 50), cell);
		}
	}
	if (!_active) {
		painter.setBrush(_env.ShadowBrush);
		painter.drawRect(0, 0, 800, 800);
	}
}

void BoardWidget::drawSquare(QPainter &painter, int row, int col) {
    Cell cell = makeCell(col, row);
	painter.save();
	painter.setBrush(((row+col)%2 == 1) ? _env.BlackSquareBrush : _env.WhiteSquareBrush);
	if (cell == _border && cell.valid())
		painter.setPen(QPen(_env.BorderBrush, 5));
	else
		painter.setPen(QPen());
	painter.drawRect(col*100, row*100, 100, 100);

	painter.setPen(QPen());
	if (cell.valid()) {
		if (_blue.contains(cell)) {
			painter.setBrush(_env.BlueBrush);
			painter.drawRect(col*100, row*100, 100, 100);
		}
		else if (_green.contains(cell)) {
			painter.setBrush(_env.GreenBrush);
			painter.drawRect(col*100, row*100, 100, 100);
		}
	}
	painter.restore();
}

void BoardWidget::drawStone(QPainter &painter, QPoint centre, Cell cell) {
	if (!cell.valid())
		return;

	painter.save();
	painter.setPen(Qt::NoPen);
	if (_position.color(cell) == Role::White)
		painter.setBrush(_env.WhiteStoneBrush);
	else
		painter.setBrush(_env.BlackStoneBrush);
	painter.drawEllipse (centre.x()-40, centre.y()-40, 80, 80);

	if (_position.king(cell)) {
		painter.setPen(_env.KingText);
		painter.drawText(centre.x()-30, centre.y()-30, 60, 60, Qt::AlignCenter, "Д");
	}
	painter.restore();
}

QRect BoardWidget::makeViewport() const {
	int side = qMin(width(), height());
	QPoint origin = QPoint(width()/2 - side/2, height()/2 - side/2);
	return QRect(origin, QSize(side, side));
}

Cell BoardWidget::locate(QPoint pos) const {
	QRect viewport = makeViewport();
	pos -= viewport.topLeft();
	pos *= 900;
	pos /= viewport.height();
	pos -= QPoint(50, 50);
	if (pos.x() < 0 || pos.y() < 0)
        return Cell();
    return makeCell(pos.x()/100, pos.y()/100);
}

Cell BoardWidget::makeCell(int col, int row) const {
    int rank = _flipped ? row : 7-row;
    int file = _flipped ? 7-col : col;
    return Cell(file, rank);
}

BoardWidget::Environment::Environment() {
	LabelFont.setPointSize(20);
	BorderBrush = QBrush(QColor(0xFF, 0x00, 0x00, 0xFF));
	WhiteSquareBrush = QBrush(QColor(0xF0, 0xF0, 0xC8, 0xFF));
	BlackSquareBrush = QBrush(QColor(0x00, 0x3C, 0x00, 0xFF));
	GreenBrush = QBrush(QColor(0x80, 0xC0, 0x20, 0x80));
	BlueBrush = QBrush(QColor(0x00, 0x00, 0xFF, 0x80));
	ShadowBrush = QBrush(QColor(0x00, 0x00, 0x00, 0x40));
	KingText = QPen(Qt::black);

	QRadialGradient proto;
	proto.setCoordinateMode(QGradient::ObjectBoundingMode);
	proto.setCenter(0.5, 0.5);
	proto.setFocalPoint(0.5, 0.5);

	QRadialGradient white = proto, black = proto;
	white.setColorAt(0.0, Qt::gray);
	white.setColorAt(1.0, Qt::white);
	black.setColorAt(0.0, QColor(200, 40, 40));
	black.setColorAt(0.8, QColor(240, 60, 60));
	black.setColorAt(1.0, QColor(255, 70, 70));

	WhiteStoneBrush = QBrush(white);
	BlackStoneBrush = QBrush(black);
}
