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

#ifndef __INDIGOMANAGERWINDOW_H
#define __INDIGOMANAGERWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QToolButton>
#include <QSpinBox>
#include <QGroupBox>
#include <QStatusBar>
#include <QTimer>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>
#include <QSettings>
#include <QLabel>
#include <QMap>
#include <QCloseEvent>

class IndigoManagerWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit IndigoManagerWindow(QWidget *parent = nullptr);
	~IndigoManagerWindow();

protected:
	void closeEvent(QCloseEvent *event) override;

private slots:
	void startStopServer();
	void saveLog();
	void handleProcessOutput();
	void handleProcessError();
	void handleProcessStateChanged(QProcess::ProcessState newState);
	void showServerHelp();
	void populateDriversMenu();
	void resetToDefaults();
	void showAboutDialog();

private:
	void setupUi();
	void initializeServerAndDrivers();
	QStringList buildCommandArguments();
	void processAndDisplayText(const QString &text);
	void appendToLog(const QString &text, bool isError = false);
	void updateControlsState();

	void saveConfig();
	void loadConfig();

	QString formatServiceAddress();
	bool isOptionInAdditionalParams(const QString &option);

	QProcess *indigoServer;
	bool serverRunning;
	bool autoScroll;

	QString serverExecutablePath;
	QString installationPrefix;
	QStringList driverFilePaths;
	QMap<QString, QPair<QString, QString>> driverDefinitions;
	bool serverAndDriversInitialized;

	QSpinBox *portSpinBox;
	QLineEdit *bonjourNameEdit;
	QCheckBox *disableBonjourCheck;
	QCheckBox *disableBlobBufferingCheck;
	QCheckBox *enableBlobCompressionCheck;
	QComboBox *verbosityComboBox;
	QLineEdit *additionalParamsEdit;

	QPushButton *startStopButton;
	QPushButton *saveLogButton;
	QPushButton *helpButton;
	QPushButton *resetButton;
	QToolButton *driversMenuButton;
	QTextEdit *logTextEdit;

	QTemporaryFile *logFile;
	QTextStream logStream;

	QLabel *statusIconLabel;
	QLabel *statusMessageLabel;
	QPushButton *logoButton;
};

#endif // __INDIGOMANAGERWINDOW_H