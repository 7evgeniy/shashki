#ifndef BOARD_WIDGET_H
#define BOARD_WIDGET_H

#include <QWidget>
#include <QList>
#include <QRect>
#include <QPen>
#include "../board/position.h"
#include "../board/cell.h"

class QPainter;
class BoardController;

class BoardWidget : public QWidget {
	Q_OBJECT
public:
	BoardWidget();
	void setController(BoardController *controller);
	const Position& position() const;
	Position& position();
	void setPosition(const Position &position);
	void setFlipped(bool is);
	void markGreen(QList<Cell> green);
	void markBlue(QList<Cell> blue);
	void markBorder(Cell border);
	void unmark();
	bool flipped() const;
	QList<Cell> green() const;
	QList<Cell> blue() const;
	Cell border() const;
protected:
	void keyPressEvent(QKeyEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void paintEvent(QPaintEvent *e);
private:
	QRect makeViewport() const;
	Cell locate(QPoint pos) const;
	void drawSquare(QPainter& painter, int row, int col);
	void drawStone(QPainter& painter, QPoint centre, Cell cell);
    Cell makeCell(int col, int row) const;
private:
	struct Environment {
		QFont LabelFont;
		QBrush BorderBrush;
		QBrush WhiteSquareBrush;
		QBrush BlackSquareBrush;
		QBrush WhiteStoneBrush;
		QBrush BlackStoneBrush;
		QBrush GreenBrush;
		QBrush BlueBrush;
		QBrush ShadowBrush;
		QPen KingText;
		Environment();
	};
	static Environment _env;
private:
	BoardController *_controller;   // если ход делается через виджет, то нужен контроллер
	Position _position;    // позиция, рисуемая на доске
	QList<Cell> _green;    // поля, помечаемые цветом готовности
	QList<Cell> _blue;     // поля, помечаемые цветом обработки
	Cell _border;   // поле, вокруг которого рисуется рамочка
	bool _flipped;  // «изображение доски перевёрнуто»
	Cell _hover;    // уже был отправлен сигнал, что указатель мыши над полем
};

#endif
