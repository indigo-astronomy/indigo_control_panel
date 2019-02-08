#ifndef QINDIGOSWITCH_H
#define QINDIGOSWITCH_H

#include <indigo_bus.h>
#include <QCheckBox>


class QIndigoProperty;


class QIndigoItem {
public:
    QIndigoItem(QIndigoProperty* p, indigo_property* property, indigo_item* item) : m_parent(p), m_property(property), m_item(item) {}
    virtual ~QIndigoItem();
    virtual void update() = 0;
    virtual void reset() {}
    virtual void apply() {}

//protected:
    QIndigoProperty* m_parent;
    indigo_property* m_property;
    indigo_item* m_item;
};


class QIndigoSwitch : public QCheckBox, public QIndigoItem
{
    Q_OBJECT
public:
    QIndigoSwitch(QIndigoProperty* p, indigo_property* property, indigo_item* item);
    virtual ~QIndigoSwitch();

    virtual void update();

signals:
    void item_changed();

private slots:
    void switch_clicked(bool checked);
};

#endif // QINDIGOSWITCH_H
