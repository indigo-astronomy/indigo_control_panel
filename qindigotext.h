#ifndef QINDIGOTEXT_H
#define QINDIGOTEXT_H

#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include "qindigoswitch.h"


class QIndigoText : public QWidget, public QIndigoItem
{
    Q_OBJECT
public:
    explicit QIndigoText(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent = nullptr);
    virtual ~QIndigoText();

    virtual void update();

signals:

public slots:

private:
    QLabel* label;
    QLineEdit* text;
};

#endif // QINDIGOTEXT_H
