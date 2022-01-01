#include <QMainWindow>
#include <QApplication>

int main(int argc, char **argv) {
	QApplication app(argc, argv);
	QMainWindow w;
	w.show();
	return app.exec();
}
