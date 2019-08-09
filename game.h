#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QString>
#include <QList>
#include "board_state.h"

class QWidget;
class QComboBox;
class BoardWidget;
class HistoryWidget;
class Player;
class QDataStream;

class Game : public QObject {
	Q_OBJECT
public:
	typedef int PlayerType;
	static const PlayerType None = -1, Human = 0, Machine = 1;
	enum class Command {Make, Read, Write, Drop, Cut, Flip};
	enum class Travel {Back, Forth, Start, Finish};
	static QComboBox* makePlayerList(Role color);
public:
	Game(QObject *parent = nullptr);
	Game* clone() const;
	QWidget* gameWidget() const;
	void move(QList<Cell> action);
public:
	void postCommand(Command command);
	void setPlayerType(Role color, PlayerType type);
	void setFilename(QString filename);
	void setFrozen(bool is);
	void stopWriting();
	bool lost() const;
	bool frozen() const;    // вызов разрешён, только если enabled() == true
	bool enabled() const;
	QString filename() const;
	bool isWriting() const;
	int stoneCount(Role color) const;
	void setupBoard();
public:
	void travelHistory(Travel travel);
	bool reportHistory(Travel travel) const;
	Role nowColor() const;
	bool nowLost() const;
signals:
	void notifyGui();
	void notifyError(QString type, QString description);
private:
	void doMake();
	void doRead();
	void doWrite();
	void doDrop();
	void doCut();
	void doFlip();
private:
	void makePlayers();
private:
	static const uint8_t FileHeader[4];
	static BoardState initialBoard();
	static Position initialPosition();
	static bool checkHeader(QDataStream &in);
	static QList<QList<Cell>> readActions(QDataStream &in);
	static bool writeHeader(QDataStream &out);
	static bool writeAction(QDataStream &out, QList<Cell> action);
private:
	static const QString AtLoadError;
	static const QString AtStoreError;
	static const QString CouldntOpenError;
	static const QString WrongFormatError;
	static const QString CouldntReadError;
	static const QString CouldntWriteError;
	class Exception {
	public:
		Exception(QString d) : _description(d) {}
		QString description() const {return _description;}
	private:
		QString _description;
	};
private:
	QWidget *_widget;
	BoardWidget *_board;
	HistoryWidget *_history;
private:
	PlayerType _project[2];
	QString _filename;
	bool _writing;              // «сделанные полуходы немедля записываются в файл _filename»
private:
	Player *_players[2];
	QList<BoardState> _states;    // положение на доске, начиная с исходной позиции.
	QList<QList<Cell>> _actions;  // полуход, сделанный из этого положения.
	int _now;
private:
	QList<Cell> _buffer;
	bool _frozen;
};

#endif
