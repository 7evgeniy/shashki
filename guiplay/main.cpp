#include <QApplication>
#include "board_widget.h"
#include "board_controller.h"

int main(int argc, char **argv) {
	QApplication app(argc, argv);
	BoardWidget w;
	BoardController c;
	w.setController(&c);
	w.show();
	return app.exec();
}
