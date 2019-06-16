#ifndef QINDIGOBLOB_H
#define QINDIGOBLOB_H

#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include "qindigoswitch.h"
#include "logger.h"


class QIndigoBLOB : public QWidget, public QIndigoItem {
	Q_OBJECT
public:
	explicit QIndigoBLOB(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent = nullptr);
	virtual ~QIndigoBLOB();

	virtual void update();
	virtual void reset();
	virtual void apply();

signals:

public slots:
	void dirty();
	void save_blob_item();

private:
	Logger* m_logger;
	QLabel* label;
	QLineEdit* text;
	bool m_dirty;
};

#endif // QINDIGOBLOB_H
