#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <unistd.h>
#include "browserwindow.h"

int
main(int argc, char *argv[])
{
	/* This is important if coma is used for numeric it breaks everything */
	unsetenv("LC_NUMERIC");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    QFile f(":qdarkstyle/style.qss");
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);
    app.setStyleSheet(ts.readAll());

    BrowserWindow browser_window;
    browser_window.show();

    return app.exec();
}
