#include <QFile>
#include <QTextStream>
#include <unistd.h>
#include "browserwindow.h"
#include <conf.h>

conf_t conf;

void write_conf() {
	char filename[PATH_MAX];
	snprintf(filename, PATH_MAX, "%s/%s", getenv("HOME"), ".indigo_panel.config");
	FILE * file= fopen(filename, "wb");
	if (file != NULL) {
		fwrite(&conf, sizeof(conf), 1, file);
		fclose(file);
	}
}

void read_conf() {
	char filename[PATH_MAX];
	snprintf(filename, PATH_MAX, "%s/%s", getenv("HOME"), ".indigo_panel.config");
	FILE * file= fopen(filename, "rb");
	if (file != NULL) {
		fread(&conf, sizeof(conf), 1, file);
		fclose(file);
	}
}

int main(int argc, char *argv[]) {
	/* This is important if coma is used for numeric it breaks everything */
	unsetenv("LC_NUMERIC");
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	memset(&conf,0,sizeof(conf_t));
	conf.blobs_enabled = true;
	conf.auto_connect = true;
	conf.indigo_use_host_suffix = true;
	conf.indigo_log_level = 1; //Not used yet
	read_conf();

	QApplication app(argc, argv);

	QFile f(":qdarkstyle/style.qss");
	f.open(QFile::ReadOnly | QFile::Text);
	QTextStream ts(&f);
	app.setStyleSheet(ts.readAll());

	BrowserWindow browser_window;
	browser_window.show();

	return app.exec();
}
