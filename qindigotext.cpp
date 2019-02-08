#include <QHBoxLayout>
#include "qindigotext.h"


QIndigoText::QIndigoText(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent)
    : QWidget(parent), QIndigoItem(p, property, item), m_dirty(false)
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

    connect(text, &QLineEdit::textEdited, this, &QIndigoText::dirty);
}

QIndigoText::~QIndigoText() {
    //delete label;
    //delete text;
}

void
QIndigoText::update() {
    //  Apply update from indigo bus only if not being edited
    if (!m_dirty)
        text->setText(m_item->text.value);
}

void
QIndigoText::reset() {
    if (m_dirty) {
        m_dirty = false;
        update();
        text->setStyleSheet("color: #FFFFFF");
        text->clearFocus();
    }
}

void
QIndigoText::apply() {
    if (m_dirty) {
        strncpy(m_item->text.value, text->text().toUtf8().constData(), sizeof(m_item->text.value));
        reset();
    }
}

void
QIndigoText::dirty() {
    //  Set dirty flag
    m_dirty = true;

    //  Colour text red
    text->setStyleSheet("color: #CC0000");
}
