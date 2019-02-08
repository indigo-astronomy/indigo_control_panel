#ifndef QINDIGOPROPERTY_H
#define QINDIGOPROPERTY_H

#include <QFrame>
#include <indigo_bus.h>


class QVBoxLayout;
class QIndigoItem;


class QIndigoProperty : public QFrame
{
    Q_OBJECT
public:
    explicit QIndigoProperty(indigo_property* property, QWidget *parent = nullptr);
    virtual ~QIndigoProperty();

    void update_controls();

private:
    void build_property_form(QVBoxLayout* layout);
    void build_text_property_form(QVBoxLayout* layout);
    void build_number_property_form(QVBoxLayout* layout);
    void build_switch_property_form(QVBoxLayout* layout);
    void build_light_property_form(QVBoxLayout* layout);
    void build_buttons(QVBoxLayout* layout);

signals:

public slots:
    void update();
    void property_update(indigo_property* property);
    void set_clicked();
    void reset_clicked();

private:
    indigo_property* m_property;
    QIndigoItem** m_controls;
};

#endif // QINDIGOPROPERTY_H
