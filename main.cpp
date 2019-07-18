// Copyright (c) 2019 Rumen G.Bogdanovski & David Hulse
// All rights reserved.
//
// You can use this software under the terms of 'INDIGO Astronomy
// open-source license' (see LICENSE.md).
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS 'AS IS' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <QFile>
#include <QTextStream>
//#include <unistd.h>
#include "browserwindow.h"
#include <conf.h>

conf_t conf;

void write_conf() {
    char filename[PATH_LEN];
    snprintf(filename, PATH_LEN, "%s/%s", getenv("HOME"), CONFIG_FILENAME);
	FILE * file= fopen(filename, "wb");
    if (file != nullptr) {
		fwrite(&conf, sizeof(conf), 1, file);
		fclose(file);
	}
}

void read_conf() {
    char filename[PATH_LEN];
    snprintf(filename, PATH_LEN, "%s/%s", getenv("HOME"), CONFIG_FILENAME);
	FILE * file= fopen(filename, "rb");
    if (file != nullptr) {
		fread(&conf, sizeof(conf), 1, file);
		fclose(file);
	}
}

int main(int argc, char *argv[]) {
	indigo_main_argv = (const char**)argv;
	indigo_main_argc = argc;

	/* This is important if coma is used for numeric it breaks everything */
	qunsetenv("LC_NUMERIC");
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	memset(&conf,0,sizeof(conf_t));
	conf.blobs_enabled = true;
	conf.auto_connect = true;
	conf.indigo_use_host_suffix = true;
	conf.indigo_log_level = INDIGO_LOG_INFO;
	read_conf();

	indigo_set_log_level(conf.indigo_log_level);

	/* This shall be set only before connecting */
	indigo_use_host_suffix = conf.indigo_use_host_suffix;

	QApplication app(argc, argv);

	QFile f(":qdarkstyle/style.qss");
	f.open(QFile::ReadOnly | QFile::Text);
	QTextStream ts(&f);
	app.setStyleSheet(ts.readAll());

	BrowserWindow browser_window;
	browser_window.show();

	return app.exec();
}
