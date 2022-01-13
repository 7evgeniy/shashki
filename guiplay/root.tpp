#include "root.h"

template<typename T> Root<T>::Root(T data) {
	_contents.append(Data(-1, data));
	_heads.append(0);
}

template<typename T> QList<typename Root<T>::Iterator> Root<T>::heads() const {
	return _heads;
}

template<typename T> typename Root<T>::Iterator Root<T>::prev(Iterator at) const {
	return _contents[at._value].parent;
}

template<typename T> void Root<T>::markHead(Iterator at) {
	if (!_heads.contains(at))
		_heads.append(at);
}

template<typename T> void Root<T>::unmarkHead(Iterator head) {
	_heads.removeOne(head);
}

template<typename T> typename Root<T>::Iterator Root<T>::appendHead(Iterator at, T data) {
	_heads.append(_contents.size());
	_contents.append(Data(at, data));
	return _heads.last();
}

template<typename T> const T& Root<T>::operator [](Iterator at) const {
	return _contents[at._value].data;
}

template<typename T> QList<T> Root<T>::makeList(Iterator head) const {
   QList<T> list;
   typename Root<T>::Iterator it = head;
   while (true) {
	   list.prepend((*this)[it]);
	   if (it.source())
		   return list;
	   it = prev(it);
   }
}
