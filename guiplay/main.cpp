#include <QApplication>
#include "main_window.h"
#include "board_controller.h"

int main(int argc, char **argv) {
	QApplication app(argc, argv);
	MainWindow w;
	w.show();
	return app.exec();
}
