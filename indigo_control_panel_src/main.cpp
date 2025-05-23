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
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QSocketNotifier>
#include <signal.h>
#ifdef INDIGO_WINDOWS
#include <QTimer>
#include <windows.h>
#else
#include <QSocketNotifier>
#include <signal.h>
#include <unistd.h>
#endif

#include "browserwindow.h"
#include <conf.h>

conf_t conf;
char config_path[PATH_LEN];

#ifndef INDIGO_WINDOWS
int sigpipe_fd[2];
int sigint_fd[2];

void handle_sigint(int) {
	char a = 1;
	write(sigint_fd[1], &a, sizeof(a));
}

void handle_sigpipe(int) {
	char a = 1;
	write(sigpipe_fd[1], &a, sizeof(a));
}
#else
void write_conf();

BOOL WINAPI console_handler(DWORD signal) {
	if (signal == CTRL_C_EVENT) {
		write_conf();
		indigo_error("Configuration saved. Exiting...");
		QTimer::singleShot(0, qApp, &QCoreApplication::quit);
		return TRUE;
	}
	return FALSE;
}
#endif

void write_conf() {
	char filename[PATH_LEN];
	snprintf(filename, PATH_LEN, "%s/%s", config_path, CONFIG_FILENAME);
	FILE * file= fopen(filename, "wb");
	if (file != nullptr) {
		fwrite(&conf, sizeof(conf), 1, file);
		fclose(file);
	}
}

void read_conf() {
	char filename[PATH_LEN];
	snprintf(filename, PATH_LEN, "%s/%s", config_path, CONFIG_FILENAME);
	FILE * file= fopen(filename, "rb");
	if (file != nullptr) {
		fread(&conf, sizeof(conf), 1, file);
		fclose(file);
	}
}


int main(int argc, char *argv[]) {
	indigo_main_argv = (const char**)argv;
	indigo_main_argc = argc;

	// create config path if it does not exist
	QDir dir("");
	dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
	strncpy(config_path, QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation).toUtf8().constData(), PATH_LEN);

#if defined(INDIGO_WINDOWS)
#define LOG_FILENAME "indigo_control_panel.log"
	char filename[PATH_LEN];
	snprintf(filename, PATH_LEN, "%s/%s", QDir::homePath().toUtf8().constData(), LOG_FILENAME);
	freopen(filename,"w", stderr);
#endif

	memset(&conf,0,sizeof(conf_t));
	conf.blobs_enabled = false;
	conf.auto_connect = true;
	conf.indigo_use_host_suffix = true;
	conf.use_state_icons = false;
	conf.use_system_locale = false;
	conf.indigo_log_level = INDIGO_LOG_INFO;
	conf.preview_stretch_level = STRETCH_NORMAL;
	read_conf();

	if (!conf.use_system_locale) qunsetenv("LC_NUMERIC");

	indigo_set_log_level(conf.indigo_log_level);

	/* This shall be set only before connecting */
	indigo_use_host_suffix = conf.indigo_use_host_suffix;

	for (int i = 1; i < argc; i++) {
		if ((!strcmp(argv[i], "-T") || !strcmp(argv[i], "--master-token")) && i < argc - 1) {
			indigo_set_master_token(indigo_string_to_token(argv[i + 1]));
			i++;
		} else if ((!strcmp(argv[i], "-a") || !strcmp(argv[i], "--acl-file")) && i < argc - 1) {
			indigo_load_device_tokens_from_file(argv[i + 1]);
			i++;
		}
	}

	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication app(argc, argv);

	QFont font("SansSerif", 10, QFont::Medium);
	font.setStyleHint(QFont::SansSerif);
	app.setFont(font);

	QFile f(":qdarkstyle/style.qss");
	f.open(QFile::ReadOnly | QFile::Text);
	QTextStream ts(&f);
	app.setStyleSheet(ts.readAll());
	f.close();

	BrowserWindow browser_window;
	browser_window.show();

#ifndef INDIGO_WINDOWS
	// Handle SIGPIPE on Unix-like systems to prevent application crash
	if (pipe(sigpipe_fd)) {
		indigo_error("Couldn't create pipe for SIGPIPE");
	}
	struct sigaction sa_pipe;
	sa_pipe.sa_handler = handle_sigpipe;
	sigemptyset(&sa_pipe.sa_mask);
	sa_pipe.sa_flags = 0;
	sigaction(SIGPIPE, &sa_pipe, nullptr);

	QSocketNotifier *sn_pipe = new QSocketNotifier(sigpipe_fd[0], QSocketNotifier::Read, &app);
	QObject::connect(sn_pipe, &QSocketNotifier::activated, [&]() {
		sn_pipe->setEnabled(false);
		char tmp;
		read(sigpipe_fd[0], &tmp, sizeof(tmp));
		indigo_debug("SIGPIPE caught");
		sn_pipe->setEnabled(true);
	});

	// Handle SIGINT (Ctrl+C) to save config and exit
	if (pipe(sigint_fd)) {
		indigo_error("Couldn't create pipe for SIGINT");
	}
	struct sigaction sa_int;
	sa_int.sa_handler = handle_sigint;
	sigemptyset(&sa_int.sa_mask);
	sa_int.sa_flags = 0;
	sigaction(SIGINT, &sa_int, nullptr);

	QSocketNotifier *sn_int = new QSocketNotifier(sigint_fd[0], QSocketNotifier::Read, &app);
	QObject::connect(sn_int, &QSocketNotifier::activated, [&]() {
		sn_int->setEnabled(false);
		char tmp;
		read(sigint_fd[0], &tmp, sizeof(tmp));
		write_conf();
		indigo_debug("Configuration saved. Exiting...");
		QCoreApplication::quit();
		sn_int->setEnabled(true);
	});
#else
	if (!SetConsoleCtrlHandler(console_handler, TRUE)) {
		indigo_debug("Could not set control handler");
	}
#endif

	return app.exec();
}
