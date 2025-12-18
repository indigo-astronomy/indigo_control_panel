// Copyright (c) 2025 Rumen G.Bogdanovski
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

#include "IndigoManagerWindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QFont>

int main(int argc, char *argv[]) {
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication app(argc, argv);
	app.setApplicationName("INDIGO Server Manager");
	app.setOrganizationName("INDIGO");

	app.setWindowIcon(QIcon(":/resource/server_manager.png"));

	int id = QFontDatabase::addApplicationFont(":/fonts/Hack-Regular.ttf");
	QFontDatabase::addApplicationFont(":/fonts/Hack-Bold.ttf");
	QFontDatabase::addApplicationFont(":/fonts/Hack-Italic.ttf");
	QFontDatabase::addApplicationFont(":/fonts/Hack-BoldItalic.ttf");
	if (id != -1) {
		QStringList families = QFontDatabase::applicationFontFamilies(id);
		if (!families.isEmpty()) {
			QString monoFamily = families.at(0);
			QFont::insertSubstitution("monospace", monoFamily);
		}
	} else {
		qWarning("Failed to load embedded Hack Mono font, using system default.");
	}

	id = QFontDatabase::addApplicationFont(":/fonts/DejaVuSans.ttf");
	QFontDatabase::addApplicationFont(":/fonts/DejaVuSans-Bold.ttf");
	QFontDatabase::addApplicationFont(":/fonts/DejaVuSans-Oblique.ttf");
	QFontDatabase::addApplicationFont(":/fonts/DejaVuSans-BoldOblique.ttf");
	QFontDatabase::addApplicationFont(":/fonts/DejaVuSans-ExtraLight.ttf");
	if (id != -1) {
		QString family = QFontDatabase::applicationFontFamilies(id).at(0);
		app.setFont(QFont(family, 10, QFont::Medium));
	} else {
		qWarning("Failed to load embedded DejaVu Sans font, using system default.");
		QFont font("SansSerif", 10, QFont::Medium);
		font.setStyleHint(QFont::SansSerif);
		app.setFont(font);
	}

	QFile f(":qdarkstyle/style.qss");
	f.open(QFile::ReadOnly | QFile::Text);
	QTextStream ts(&f);
	app.setStyleSheet(ts.readAll());
	f.close();

	IndigoManagerWindow managerWindow;
	managerWindow.show();

	return app.exec();
}
