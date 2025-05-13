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

int main(int argc, char *argv[]) {
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication app(argc, argv);
	app.setApplicationName("INDIGO Server Manager");
	app.setOrganizationName("INDIGO");

	app.setWindowIcon(QIcon(":/resource/server_manager.png"));

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