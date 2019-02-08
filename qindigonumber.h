#ifndef QINDIGONUMBER_H
#define QINDIGONUMBER_H

#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include "qindigoswitch.h"


class QIndigoProperty;


class QIndigoNumber : public QWidget, public QIndigoItem
{
    Q_OBJECT
public:
    explicit QIndigoNumber(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent = nullptr);
    virtual ~QIndigoNumber();

    virtual void update();

signals:

public slots:

private:
    QLabel* label;
    QLineEdit* text;
};

#endif // QINDIGONUMBER_H
