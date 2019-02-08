#include <QHBoxLayout>
#include "qindigonumber.h"


QIndigoNumber::QIndigoNumber(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent)
    : QWidget(parent), QIndigoItem(p, property, item)
{
    label = new QLabel(m_item->label);
    text = new QLineEdit();
    if (m_property->perm == INDIGO_RO_PERM)
        text->setReadOnly(true);
    update();

    //  Lay the labels out somehow in the widget
    QHBoxLayout* hbox = new QHBoxLayout();
    setLayout(hbox);
    hbox->setAlignment(Qt::AlignLeft);
    hbox->setMargin(0);
    hbox->setSpacing(0);
    hbox->addWidget(label, 35);
    hbox->addWidget(text, 65);
}

QIndigoNumber::~QIndigoNumber() {
    //delete label;
    //delete text;
}

void
QIndigoNumber::update() {
    char buffer[50];
    sprintf(buffer, "%g"/*m_item->number.format*/, m_item->number.value);
    text->setText(buffer);
}
