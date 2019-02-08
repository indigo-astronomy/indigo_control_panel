#include <QHBoxLayout>
#include "qindigotext.h"


QIndigoText::QIndigoText(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent)
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

QIndigoText::~QIndigoText() {
    //delete label;
    //delete text;
}

void
QIndigoText::update() {
    //fprintf(stderr, "UPDATING TO [%s]\n", m_item->text.value);
    text->setText(m_item->text.value);
}
