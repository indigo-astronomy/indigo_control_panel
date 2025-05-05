#include "IndigoManagerWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QHostInfo>
#include <QScrollBar>
#include <QDir>
#include <QTextStream>
#include <QTemporaryFile>
#include <QSettings>
#include <QMenu>

// Constants for configuration paths
const QString CONFIG_DIR = QDir::homePath() + "/.config";
const QString CONFIG_FILE = CONFIG_DIR + "/indigo_server_manager.conf";

IndigoManagerWindow::IndigoManagerWindow(QWidget *parent) : QMainWindow(parent)
	, indigoServer(new QProcess(this))
	, serverRunning(false)
	, autoScroll(true)
	, logFile(new QTemporaryFile(this))
	, serverAndDriversInitialized(false) {

	logFile->open();
	logStream.setDevice(logFile);

	setupUi();
	loadConfig();

	initializeServerAndDrivers();

	// Configure process to prevent blocking
	indigoServer->setProcessChannelMode(QProcess::SeparateChannels);
	indigoServer->setReadChannel(QProcess::StandardOutput);

	connect(startStopButton, &QPushButton::clicked, this, &IndigoManagerWindow::startStopServer);
	connect(saveLogButton, &QPushButton::clicked, this, &IndigoManagerWindow::saveLog);
	connect(helpButton, &QPushButton::clicked, this, &IndigoManagerWindow::showServerHelp);  // Connect help button

	connect(indigoServer, &QProcess::readyReadStandardOutput, this, &IndigoManagerWindow::handleProcessOutput);
	connect(indigoServer, &QProcess::readyReadStandardError, this, &IndigoManagerWindow::handleProcessError);
	connect(indigoServer, &QProcess::stateChanged, this, &IndigoManagerWindow::handleProcessStateChanged);

	// Track whether we should auto-scroll
	connect(logTextEdit->verticalScrollBar(), &QScrollBar::valueChanged, [this](int value) {
		QScrollBar *sb = logTextEdit->verticalScrollBar();
		autoScroll = (value == sb->maximum());
	});

	// Configure log text edit for better performance
	logTextEdit->document()->setMaximumBlockCount(1000);

	helpButton->setEnabled(!serverRunning);
}

IndigoManagerWindow::~IndigoManagerWindow() {
	saveConfig();

	logStream.flush();
	logFile->close();
	logFile->setAutoRemove(true);

	// Process termination
	if (indigoServer->state() != QProcess::NotRunning) {
		indigoServer->terminate();
		indigoServer->waitForFinished(3000);
		if (indigoServer->state() != QProcess::NotRunning) {
			indigoServer->kill();
		}
	}
}

void IndigoManagerWindow::setupUi() {
	setWindowTitle("INDIGO Server Controller");
	resize(800, 600);

	QWidget *centralWidget = new QWidget(this);
	setCentralWidget(centralWidget);

	QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

	// Configuration group
	QGroupBox *configGroup = new QGroupBox("Server Configuration", centralWidget);
	QVBoxLayout *configMainLayout = new QVBoxLayout(configGroup);

	QHBoxLayout *namePortLayout = new QHBoxLayout();

	QLabel *serviceNameLabel = new QLabel("Service Name:", configGroup);
	bonjourNameEdit = new QLineEdit(configGroup);
	bonjourNameEdit->setText(QHostInfo::localHostName());
	namePortLayout->addWidget(serviceNameLabel);
	namePortLayout->addWidget(bonjourNameEdit, 3);

	QLabel *portLabel = new QLabel("Port:", configGroup);
	portSpinBox = new QSpinBox(configGroup);
	portSpinBox->setRange(1024, 65535);
	portSpinBox->setValue(7624);
	namePortLayout->addWidget(portLabel);
	namePortLayout->addWidget(portSpinBox, 1);

	QLabel *verbosityLabel = new QLabel("Log Level:", configGroup);
	verbosityComboBox = new QComboBox(configGroup);
	verbosityComboBox->addItem("Error", "");
	verbosityComboBox->addItem("Info", "-v");
	verbosityComboBox->addItem("Debug", "-vv");
	verbosityComboBox->addItem("Trace Bus", "-vvb");
	verbosityComboBox->addItem("Trace", "-vvv");
	namePortLayout->addWidget(verbosityLabel);
	namePortLayout->addWidget(verbosityComboBox, 2);

	configMainLayout->addLayout(namePortLayout);

	QHBoxLayout *driversLayout = new QHBoxLayout();
	QLabel *driversLabel = new QLabel("Drivers:", configGroup);

	driversLabel->setFixedWidth(serviceNameLabel->sizeHint().width());
	additionalParamsEdit = new QLineEdit(configGroup);
	additionalParamsEdit->setPlaceholderText("indigo_ccd_simulator indigo_mount_simulator ...");
	additionalParamsEdit->setToolTip("Enter driver names or other parameters to pass to indigo_server");
	driversLayout->addWidget(driversLabel);
	driversLayout->addWidget(additionalParamsEdit);

	driversMenuButton = new QToolButton(configGroup);
	driversMenuButton->setText("…");  // Unicode ellipsis (U+2026)
	driversMenuButton->setToolTip("Click to select drivers from the installed INDIGO drivers");
	connect(driversMenuButton, &QToolButton::clicked, this, &IndigoManagerWindow::populateDriversMenu);
	driversLayout->addWidget(driversMenuButton);

	configMainLayout->addLayout(driversLayout);

	QHBoxLayout *checkboxesAndLogoLayout = new QHBoxLayout();

	QVBoxLayout *checkboxesLayout = new QVBoxLayout();

	QHBoxLayout *bonjourLayout = new QHBoxLayout();
	QLabel *bonjourSpacerLabel = new QLabel("", configGroup);
	bonjourSpacerLabel->setFixedWidth(serviceNameLabel->sizeHint().width());
	bonjourLayout->addWidget(bonjourSpacerLabel);
	disableBonjourCheck = new QCheckBox("Disable Bonjour", configGroup);
	bonjourLayout->addWidget(disableBonjourCheck);
	bonjourLayout->addStretch(1);
	checkboxesLayout->addLayout(bonjourLayout);

	QHBoxLayout *blobLayout = new QHBoxLayout();
	QLabel *blobSpacerLabel = new QLabel("", configGroup);
	blobSpacerLabel->setFixedWidth(serviceNameLabel->sizeHint().width());
	blobLayout->addWidget(blobSpacerLabel);
	disableBlobBufferingCheck = new QCheckBox("Disable BLOB Buffering", configGroup);
	blobLayout->addWidget(disableBlobBufferingCheck);
	blobLayout->addStretch(1);
	checkboxesLayout->addLayout(blobLayout);

	QHBoxLayout *compressionLayout = new QHBoxLayout();
	QLabel *compressionSpacerLabel = new QLabel("", configGroup);
	compressionSpacerLabel->setFixedWidth(serviceNameLabel->sizeHint().width());
	compressionLayout->addWidget(compressionSpacerLabel);
	enableBlobCompressionCheck = new QCheckBox("Enable BLOB Compression", configGroup);
	compressionLayout->addWidget(enableBlobCompressionCheck);
	compressionLayout->addStretch(1);
	checkboxesLayout->addLayout(compressionLayout);

	checkboxesAndLogoLayout->addLayout(checkboxesLayout);

	QLabel *logoLabel = new QLabel(configGroup);
	QPixmap logoPixmap(":/resource/indigo_logo.png");
	logoPixmap = logoPixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	logoLabel->setPixmap(logoPixmap);
	logoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	checkboxesAndLogoLayout->addWidget(logoLabel);

	configMainLayout->addLayout(checkboxesAndLogoLayout);

	mainLayout->addWidget(configGroup);

	// Buttons
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	startStopButton = new QPushButton("Start Server", centralWidget);
	saveLogButton = new QPushButton("Save Log", centralWidget);
	helpButton = new QPushButton("Server Help", centralWidget);
	buttonLayout->addWidget(startStopButton);
	buttonLayout->addWidget(saveLogButton);
	buttonLayout->addWidget(helpButton);
	mainLayout->addLayout(buttonLayout);

	// Log area
	QGroupBox *logGroup = new QGroupBox("Server Output", centralWidget);
	QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
	logTextEdit = new QTextEdit(logGroup);
	logTextEdit->setReadOnly(true);
	logTextEdit->setLineWrapMode(QTextEdit::NoWrap);
	logTextEdit->document()->setMaximumBlockCount(1000);
	QFont monoFont("Monospace");
	monoFont.setStyleHint(QFont::TypeWriter);
	logTextEdit->setFont(monoFont);
	logTextEdit->setAcceptRichText(false);
	logLayout->addWidget(logTextEdit);
	mainLayout->addWidget(logGroup);

	// Set stretch factors to make log area taller
	mainLayout->setStretchFactor(configGroup, 1);
	mainLayout->setStretchFactor(logGroup, 3);

	// Status bar with icon
	statusIconLabel = new QLabel(this);
	statusIconLabel->setPixmap(QPixmap(":resource/led-grey.png"));
	statusIconLabel->setMargin(3);
	statusIconLabel->setIndent(2);

	// Create a label for the status message with less padding
	statusMessageLabel = new QLabel("Ready", this);
	statusMessageLabel->setMargin(1);
	statusMessageLabel->setIndent(0);
	statusMessageLabel->setTextFormat(Qt::RichText);

	// Create a spacer widget to add some space at the beginning of status bar
	QWidget* leftSpacer = new QWidget(this);
	leftSpacer->setFixedWidth(2);

	// Add permanent widgets to status bar
	statusBar()->addPermanentWidget(leftSpacer);
	statusBar()->addPermanentWidget(statusIconLabel);
	statusBar()->addPermanentWidget(statusMessageLabel, 1);
}

void IndigoManagerWindow::startStopServer() {
	if (!serverRunning) {
		logTextEdit->clear();
		logFile->resize(0);

		QString program = serverExecutablePath;
		if (program.isEmpty()) {
			initializeServerAndDrivers();
			program = serverExecutablePath;
		}

		if (program.isEmpty()) {
			statusIconLabel->setPixmap(QPixmap(":resource/led-red.png"));
			appendToLog("Cannot start server: indigo_server executable not found!", true);
			statusMessageLabel->setText("Error: Server executable not found");
			return;
		}

		QStringList arguments = buildCommandArguments();

		appendToLog("> " + program + " " + arguments.join(" "), false);

		// Set process environment to prevent output blocking
		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("PYTHONUNBUFFERED", "1");
		env.insert("UNBUFFERED", "1");
		indigoServer->setProcessEnvironment(env);

		indigoServer->setProgram(program);
		indigoServer->setArguments(arguments);
		indigoServer->start(QIODevice::ReadOnly | QIODevice::Unbuffered);

		if (!indigoServer->waitForStarted(1000)) {
			statusIconLabel->setPixmap(QPixmap(":resource/led-red.png"));
			appendToLog("Failed to start indigo_server process!", true);
			statusMessageLabel->setText("Error: Failed to start server");
			return;
		}

		statusIconLabel->setPixmap(QPixmap(":resource/led-green.png"));
		startStopButton->setText("Stop Server");
		serverRunning = true;
		helpButton->setEnabled(false);
		statusMessageLabel->setText("Server started on " + formatServiceAddress());
	} else {
		// Stop the server
		appendToLog("> Stopping indigo_server...", false);
		statusIconLabel->setPixmap(QPixmap(":resource/led-orange.png"));
		statusMessageLabel->setText("Stopping server...");

		indigoServer->terminate();
		if (!indigoServer->waitForFinished(3000)) {
			appendToLog("Server not responding, forcing termination!", true);
			indigoServer->kill();
		}

		statusIconLabel->setPixmap(QPixmap(":resource/led-grey.png"));
		startStopButton->setText("Start Server");
		serverRunning = false;
		helpButton->setEnabled(true);
		statusMessageLabel->setText("Server stopped");
	}
}

QStringList IndigoManagerWindow::buildCommandArguments() {
	QStringList args;

	// Port
	args << "-p" << QString::number(portSpinBox->value());

	// Bonjour
	if (disableBonjourCheck->isChecked()) {
		args << "-b-";
	} else {
		// Use hostname as default if service name is empty
		QString serviceName = bonjourNameEdit->text().trimmed();
		if (serviceName.isEmpty()) {
			serviceName = QHostInfo::localHostName();
			bonjourNameEdit->setText(serviceName);
		}
		args << "-b" << serviceName;
	}

	// BLOB Buffering
	if (disableBlobBufferingCheck->isChecked()) {
		args << "-d-";
	}

	// BLOB Compression
	if (enableBlobCompressionCheck->isChecked()) {
		args << "-C";
	}

	// Verbosity
	QString verbosityFlag = verbosityComboBox->currentData().toString();
	if (!verbosityFlag.isEmpty()) {
		args << verbosityFlag;
	}

	// Additional parameters (typically drivers to load)
	QString additionalParams = additionalParamsEdit->text().trimmed();
	if (!additionalParams.isEmpty()) {
		foreach (const QString &param, additionalParams.split(QRegExp("\\s+"), QString::SkipEmptyParts)) {
			args << param;
		}
	}

	return args;
}

void IndigoManagerWindow::saveLog() {
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Save Log File"), QString(), tr("Log Files (*.log);;Text Files (*.txt);;All Files (*)"));

	if (fileName.isEmpty()) {
		return;
	}

	logStream.flush();

	if (!QFile::copy(logFile->fileName(), fileName)) {
		QMessageBox::warning(this, tr("Save Log"),
							tr("Cannot write to file %1.")
							.arg(QDir::toNativeSeparators(fileName)));
		return;
	}

	statusMessageLabel->setText(tr("Log saved to %1").arg(fileName));
}

void IndigoManagerWindow::handleProcessOutput() {
	QByteArray data = indigoServer->readAllStandardOutput();
	if (data.isEmpty()) return;

	QString text = QString::fromLocal8Bit(data);

	logStream << text;
	logStream.flush();

	processAndDisplayText(text);
}

void IndigoManagerWindow::handleProcessError() {
	QByteArray data = indigoServer->readAllStandardError();
	if (data.isEmpty()) return;

	QString text = QString::fromLocal8Bit(data);

	logStream << text;
	logStream.flush();

	processAndDisplayText(text);
}

void IndigoManagerWindow::processAndDisplayText(const QString &text) {
	static QRegExp newlineRegex("[\r\n]");

	QStringList lines = text.split(newlineRegex, QString::SkipEmptyParts);

	if (lines.isEmpty()) return;

	const int maxLinesToProcess = 50;
	int linesToProcess = qMin(lines.size(), maxLinesToProcess);

	if (linesToProcess > 10) {
		QString batchText;
		for (int i = 0; i < linesToProcess; i++) {
			batchText += lines[i] + "\n";
		}

		if (!batchText.isEmpty()) {
			batchText.chop(1);
		}

		logTextEdit->append(batchText);

		// If there are more lines, log a message and skip them to prevent UI freezing
		if (lines.size() > maxLinesToProcess) {
			logTextEdit->append(
				"NOTICE: Output truncated, " +
				QString::number(lines.size() - maxLinesToProcess) +
				" lines skipped to maintain performance"
			);
		}
	} else {
		// Process each line individually for small batches
		for (int i = 0; i < linesToProcess; i++) {
			logTextEdit->append(lines[i]);
		}
	}

	if (autoScroll) {
		QScrollBar *sb = logTextEdit->verticalScrollBar();
		sb->setValue(sb->maximum());
	}
}

void IndigoManagerWindow::appendToLog(const QString &text, bool isError) {
	if (text.isEmpty()) return;

	logStream << text << "\n";
	logStream.flush();

	logTextEdit->append(text);

	if (autoScroll) {
		QScrollBar *sb = logTextEdit->verticalScrollBar();
		sb->setValue(sb->maximum());
	}
}

void IndigoManagerWindow::handleProcessStateChanged(QProcess::ProcessState newState) {
	switch (newState) {
	case QProcess::NotRunning:
		if (serverRunning) {
			int exitCode = indigoServer->exitCode();
			appendToLog(QString("Server process exited with code %1").arg(exitCode), false);
			startStopButton->setText("Start Server");
			serverRunning = false;
			helpButton->setEnabled(true);

			if (exitCode == 0) {
				statusIconLabel->setPixmap(QPixmap(":resource/led-grey.png"));
				statusMessageLabel->setText("Server stopped normally");
			} else {
				statusIconLabel->setPixmap(QPixmap(":resource/led-red.png"));
				statusMessageLabel->setText("Server stopped with error code " + QString::number(exitCode));
			}
		}
		break;
	case QProcess::Starting:
		appendToLog("Server process starting...", false);
		statusIconLabel->setPixmap(QPixmap(":resource/led-orange.png"));
		statusMessageLabel->setText("Starting server...");
		helpButton->setEnabled(false);
		break;
	case QProcess::Running:
		appendToLog("Server process running...", false);
		statusIconLabel->setPixmap(QPixmap(":resource/led-green.png"));
		statusMessageLabel->setText("Server running on " + formatServiceAddress());
		helpButton->setEnabled(false);
		break;
	}
}

QString IndigoManagerWindow::formatServiceAddress() {
	QString hostname;

	if (!disableBonjourCheck->isChecked() && !bonjourNameEdit->text().isEmpty()) {
		hostname = bonjourNameEdit->text();
	} else {
		hostname = QHostInfo::localHostName();
	}

	return QString("<b>%1.local:%2</b>").arg(hostname).arg(portSpinBox->value());
}

void IndigoManagerWindow::initializeServerAndDrivers() {
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

	if (!env.value("INDIGO_INSTALL_PREFIX").isEmpty()) {
		installationPrefix = env.value("INDIGO_INSTALL_PREFIX");
		serverExecutablePath = installationPrefix + "/bin/indigo_server";
		if (QFile::exists(serverExecutablePath)) {
			appendToLog("Using indigo_server from environment variable: " + serverExecutablePath, false);
		} else {
			serverExecutablePath = "";
		}
	}

	if (serverExecutablePath.isEmpty()) {
		QStringList searchPaths = {
			QDir::homePath() + "/work/indigo.git/build/bin/indigo_server",
			"/usr/bin/indigo_server",
			"/bin/indigo_server",
			"/usr/local/bin/indigo_server"
		};

		for (const QString &path : searchPaths) {
			if (QFile::exists(path)) {
				serverExecutablePath = path;
				int binIndex = path.lastIndexOf("/bin/indigo_server");
				if (binIndex > 0) {
					installationPrefix = path.left(binIndex);
					appendToLog("Using indigo_server at: " + path, false);
					break;
				}
			}
		}
	}

	if (!installationPrefix.isEmpty()) {
		QString driverPath = installationPrefix + "/share/indigo/";

		if (QFile::exists(driverPath + "indigo_drivers")) {
			driverFilePaths << driverPath + "indigo_drivers";
		}
		if (QFile::exists(driverPath + "indigo_linux_drivers")) {
			driverFilePaths << driverPath + "indigo_linux_drivers";
		}

		if (!driverFilePaths.isEmpty()) {
			for (const QString &file : driverFilePaths) {
				QFile driverFile(file);
				if (driverFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QTextStream in(&driverFile);
					while (!in.atEnd()) {
						QString line = in.readLine().trimmed();
						if (line.isEmpty() || line.startsWith("#")) {
							continue;
						}

						QRegExp rx("\"([^\"]+)\",\\s*\"([^\"]+)\",\\s*(\\w+)");
						if (rx.indexIn(line) != -1) {
							QString name = rx.cap(1);
							QString description = rx.cap(2);
							QString version = rx.cap(3);
							driverDefinitions[name] = qMakePair(description, version);
						}
					}
					driverFile.close();
				}
			}
		} else {
			appendToLog("No driver definition files found under prefix: " + installationPrefix, true);
		}
	}

	if (!serverExecutablePath.isEmpty() && !driverDefinitions.isEmpty()) {
		serverAndDriversInitialized = true;
	} else {
		appendToLog("Warning: INDIGO server or driver definitions could not be fully initialized", true);
	}
}

void IndigoManagerWindow::saveConfig() {
	QDir configDir(CONFIG_DIR);
	if (!configDir.exists()) {
		configDir.mkpath(".");
	}

	QSettings settings(CONFIG_FILE, QSettings::IniFormat);

	settings.beginGroup("ServerConfig");
	settings.setValue("Port", portSpinBox->value());
	settings.setValue("BonjourName", bonjourNameEdit->text());
	settings.setValue("DisableBonjour", disableBonjourCheck->isChecked());
	settings.setValue("DisableBlobBuffering", disableBlobBufferingCheck->isChecked());
	settings.setValue("EnableBlobCompression", enableBlobCompressionCheck->isChecked());
	settings.setValue("VerbosityIndex", verbosityComboBox->currentIndex());
	settings.setValue("AdditionalParameters", additionalParamsEdit->text());
	settings.endGroup();

	settings.beginGroup("WindowGeometry");
	settings.setValue("Size", size());
	settings.setValue("Position", pos());
	settings.endGroup();

	settings.sync();
}

void IndigoManagerWindow::loadConfig() {
	QSettings settings(CONFIG_FILE, QSettings::IniFormat);

	settings.beginGroup("ServerConfig");
	portSpinBox->setValue(settings.value("Port", 7624).toInt());
	bonjourNameEdit->setText(settings.value("BonjourName", QHostInfo::localHostName()).toString());
	disableBonjourCheck->setChecked(settings.value("DisableBonjour", false).toBool());
	disableBlobBufferingCheck->setChecked(settings.value("DisableBlobBuffering", false).toBool());
	enableBlobCompressionCheck->setChecked(settings.value("EnableBlobCompression", false).toBool());
	verbosityComboBox->setCurrentIndex(settings.value("VerbosityIndex", 0).toInt());
	additionalParamsEdit->setText(settings.value("AdditionalParameters", "").toString());
	settings.endGroup();

	settings.beginGroup("WindowGeometry");
	QSize savedSize = settings.value("Size", QSize(800, 600)).toSize();
	QPoint savedPos = settings.value("Position", QPoint(100, 100)).toPoint();
	resize(savedSize);
	move(savedPos);
	settings.endGroup();
}

void IndigoManagerWindow::showServerHelp() {
	if (serverRunning) {
		return;
	}

	logTextEdit->clear();

	if (serverExecutablePath.isEmpty()) {
		statusIconLabel->setPixmap(QPixmap(":resource/led-red.png"));
		appendToLog("Cannot show help: indigo_server executable not found!", true);
		statusMessageLabel->setText("Error: Server executable not found");
		return;
	}

	QProcess helpProcess;
	appendToLog("> " + serverExecutablePath + " -h", false);
	helpProcess.setProcessChannelMode(QProcess::MergedChannels);
	helpProcess.start(serverExecutablePath, QStringList() << "-h");

	helpProcess.waitForFinished(5000);

	QByteArray output = helpProcess.readAll();
	QString helpText = QString::fromLocal8Bit(output);

	logStream << helpText;
	logStream.flush();

	appendToLog("--- INDIGO Server Help ---", false);
	processAndDisplayText(helpText);
	appendToLog("--- End of Help ---", false);

	statusMessageLabel->setText("Server help displayed");
}

void IndigoManagerWindow::populateDriversMenu() {
	if (driverDefinitions.isEmpty()) {
		QMessageBox::information(this, tr("Driver Files"),
						tr("No driver definitions available.\n"
							"Please make sure INDIGO is properly installed."));
		return;
	}

	QMenu driversMenu;

	QMap<QString, QMap<QString, QPair<QString, QString>>> driversByType;
	QStringList otherDrivers;

	QMapIterator<QString, QPair<QString, QString>> i(driverDefinitions);
	while (i.hasNext()) {
		i.next();
		QString driverName = i.key();
		QString displayText = i.value().first;

		QRegExp namePattern("indigo_([^_]+)_(.+)");
		if (namePattern.indexIn(driverName) != -1) {
			QString deviceType = namePattern.cap(1);

			QString deviceTypeTitle = deviceType;
			if (!deviceTypeTitle.isEmpty()) {
				if (deviceTypeTitle.toLower() == "ccd" ||
					deviceTypeTitle.toLower() == "ao" ||
					deviceTypeTitle.toLower() == "aux" ||
					deviceTypeTitle.toLower() == "gps") {
					deviceTypeTitle = deviceTypeTitle.toUpper();
				} else {
					deviceTypeTitle[0] = deviceTypeTitle[0].toUpper();
				}
			}

			driversByType[deviceTypeTitle][driverName] = i.value();
		} else {
			otherDrivers << driverName;
		}
	}

	QMapIterator<QString, QMap<QString, QPair<QString, QString>>> typeIter(driversByType);
	while (typeIter.hasNext()) {
		typeIter.next();
		QString deviceType = typeIter.key();

		QString menuTitle;
		if (deviceType.toLower() == "agent") {
			menuTitle = "Agents";  // For agents, just use "Agents" without "Drivers"
		} else {
			menuTitle = deviceType + " Drivers";  // For others, append "Drivers"
		}

		QMenu *subMenu = driversMenu.addMenu(menuTitle);


		QList<QPair<QString, QString>> sortedDrivers;
		QMapIterator<QString, QPair<QString, QString>> driverIter(typeIter.value());
		
		while (driverIter.hasNext()) {
			driverIter.next();
			QString driverName = driverIter.key();
			QString displayText = driverIter.value().first;
			sortedDrivers.append(qMakePair(displayText, driverName));
		}

		std::sort(sortedDrivers.begin(), sortedDrivers.end(), 
			[](const QPair<QString, QString> &a, const QPair<QString, QString> &b) {
				return a.first.toLower() < b.first.toLower();
			});

		for (const auto &driver : sortedDrivers) {
			QAction *action = subMenu->addAction(driver.first);
			action->setData(driver.second);
		}
	}

	if (!otherDrivers.isEmpty()) {
		QMenu *otherMenu = driversMenu.addMenu("Other Drivers");
		
		QList<QPair<QString, QString>> sortedOtherDrivers;
		
		for (const QString &driverName : otherDrivers) {
			QString displayText = driverDefinitions[driverName].first;
			sortedOtherDrivers.append(qMakePair(displayText, driverName));
		}
		
		std::sort(sortedOtherDrivers.begin(), sortedOtherDrivers.end(),
			[](const QPair<QString, QString> &a, const QPair<QString, QString> &b) {
				return a.first.toLower() < b.first.toLower();
			});
			
		for (const auto &driver : sortedOtherDrivers) {
			QAction *action = otherMenu->addAction(driver.first);
			action->setData(driver.second);
		}
	}

	QPoint pos = driversMenuButton->mapToGlobal(QPoint(0, driversMenuButton->height()));
	QAction *selectedAction = driversMenu.exec(pos);

	if (selectedAction) {
		QString driverName = selectedAction->data().toString();
		QString currentText = additionalParamsEdit->text().trimmed();
		if (!currentText.isEmpty() && !currentText.endsWith(" ")) {
			currentText += " ";
		}
		additionalParamsEdit->setText(currentText + driverName);
	}
}