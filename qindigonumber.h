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
    virtual void reset();
    virtual void apply();

signals:

public slots:
    void dirty();

private:
    QLabel* label;
    QLineEdit* text_value;
    QLineEdit* text_target;
    bool m_dirty;
};

#endif // QINDIGONUMBER_H
