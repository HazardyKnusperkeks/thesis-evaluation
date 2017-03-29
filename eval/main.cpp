#include "hauptfenster.hpp"

#include <QApplication>

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	
	Hauptfenster f;
	f.show();
	
	return app.exec();
}
