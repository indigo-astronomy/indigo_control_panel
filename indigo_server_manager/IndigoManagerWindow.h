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

class IndigoManagerWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit IndigoManagerWindow(QWidget *parent = nullptr);
	~IndigoManagerWindow();

private slots:
	void startStopServer();
	void saveLog();
	void handleProcessOutput();
	void handleProcessError();
	void handleProcessStateChanged(QProcess::ProcessState newState);
	void showServerHelp();
	void populateDriversMenu();

private:
	void setupUi();
	void initializeServerAndDrivers();
	QStringList buildCommandArguments();
	void processAndDisplayText(const QString &text);
	void appendToLog(const QString &text, bool isError = false);
	QPair<QString, QString> findServerExecutable();
	QStringList findDriverFiles();
	QMap<QString, QPair<QString, QString>> parseDriverFiles(const QStringList &files);

	void saveConfig();
	void loadConfig();

	QString formatServiceAddress();

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
	QToolButton *driversMenuButton;
	QTextEdit *logTextEdit;

	QTemporaryFile *logFile;
	QTextStream logStream;

	QLabel *statusIconLabel;
	QLabel *statusMessageLabel;
};

#endif // __INDIGOMANAGERWINDOW_H