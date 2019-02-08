#ifndef QINDIGOLIGHT_H
#define QINDIGOLIGHT_H

#include <QLabel>
#include <QWidget>
#include "qindigoswitch.h"


class QIndigoLight : public QWidget, public QIndigoItem
{
    Q_OBJECT
public:
    explicit QIndigoLight(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent = nullptr);
    virtual ~QIndigoLight();

    virtual void update();

signals:

public slots:

private:
    QLabel* led;
    QLabel* label;
};

#endif // QINDIGOLIGHT_H
