#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <indigo_bus.h>


class Logger : public QObject {
	Q_OBJECT
public:
	static Logger& instance();

public:
	Logger() {};
	void log(indigo_property* property, const char *message) {
		emit(Logger::instance().log_in_window(property, message));
	}

signals:
	void log_in_window(indigo_property* property, const char *message);
};

inline Logger& Logger::instance() {
	static Logger* me = nullptr;
	if (!me)
		me = new Logger();
	return *me;
}

#endif // LOGGER_H
