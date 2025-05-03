#ifndef __INDIGOMANAGERWINDOW_H
#define __INDIGOMANAGERWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>
#include <QStatusBar>
#include <QTimer>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>
#include <QSettings>
#include <QLabel>

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

private:
	void setupUi();
	QStringList buildCommandArguments();
	void processAndDisplayText(const QString &text);
	void appendToLog(const QString &text, bool isError = false);
	QString findServerExecutable();

	void saveConfig();
	void loadConfig();

	QString formatServiceAddress();

	QProcess *indigoServer;
	bool serverRunning;
	bool autoScroll;

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
	QTextEdit *logTextEdit;

	QTemporaryFile *logFile;
	QTextStream logStream;

	QLabel *statusIconLabel;
	QLabel *statusMessageLabel;
};

#endif // __INDIGOMANAGERWINDOW_H