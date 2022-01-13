#ifndef ROOT_H
#define ROOT_H

#include <QList>

template<typename T>
class Root {
public:
	class Iterator {
	public:
		friend bool operator ==(Iterator i1, Iterator i2) {return i1._value == i2._value;}
		friend bool operator !=(Iterator i1, Iterator i2) {return i1._value != i2._value;}
		Iterator() {_value = 0;}
		bool source() const {return _value == 0;}
	private:
		Iterator(int value) {_value = value;}
		int _value;
		friend class Root<T>;
	};
public:
	Root(T data);
	QList<Iterator> heads() const;
	Iterator prev(Iterator at) const;
	void markHead(Iterator at);
	void unmarkHead(Iterator head);
	Iterator appendHead(Iterator at, T data);
	QList<T> makeList(Iterator head) const;
	const T& operator [](Iterator at) const;
private:
	struct Data {
		Data (Iterator p, T d) : parent(p), data(d) {}
		Iterator parent;
		T data;
	};
	QList<Data> _contents;
	QList<Iterator> _heads;
};

#endif
