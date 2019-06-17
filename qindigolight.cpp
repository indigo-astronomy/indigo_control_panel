#include <QHBoxLayout>
#include "qindigolight.h"


QIndigoLight::QIndigoLight(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent)
    : QWidget(parent), QIndigoItem(p, property, item)
{
    led = new QLabel();
    label = new QLabel(m_item->label);
    update();

    //  Lay the labels out somehow in the widget
    QHBoxLayout* hbox = new QHBoxLayout();
    setLayout(hbox);
    hbox->setAlignment(Qt::AlignLeft);
    hbox->setMargin(0);
    hbox->setSpacing(0);
    hbox->addWidget(led);
    hbox->addWidget(label);
    hbox->addStretch();
}

QIndigoLight::~QIndigoLight() {
    //delete led;
    //delete label;
}

void
QIndigoLight::update() {
    switch (m_item->light.value) {
    case INDIGO_IDLE_STATE:
        led->setPixmap(QPixmap(":resource/led-grey.png"));
        break;
    case INDIGO_BUSY_STATE:
        led->setPixmap(QPixmap(":resource/led-orange.png"));
        break;
    case INDIGO_ALERT_STATE:
        led->setPixmap(QPixmap(":resource/led-red.png"));
        break;
    case INDIGO_OK_STATE:
        led->setPixmap(QPixmap(":resource/led-green.png"));
        break;
    }
}
