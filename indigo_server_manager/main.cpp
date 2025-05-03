#include "IndigoManagerWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication app(argc, argv);
	app.setApplicationName("INDIGO Server Manager");
	app.setOrganizationName("INDIGO");

	QFont font("SansSerif", 10, QFont::Medium);
	font.setStyleHint(QFont::SansSerif);
	app.setFont(font);

	QFile f(":qdarkstyle/style.qss");
	f.open(QFile::ReadOnly | QFile::Text);
	QTextStream ts(&f);
	app.setStyleSheet(ts.readAll());
	f.close();

	IndigoManagerWindow managerWindow;
	managerWindow.show();

	return app.exec();
}