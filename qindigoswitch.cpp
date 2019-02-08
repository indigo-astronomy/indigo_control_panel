#include <QObject>
#include "qindigoproperty.h"
#include "qindigoswitch.h"

QIndigoItem::~QIndigoItem()
{}


QIndigoSwitch::QIndigoSwitch(QIndigoProperty* p, indigo_property* property, indigo_item* item)
    : QCheckBox(), QIndigoItem(p, property, item)
{
    setText(m_item->label);
    if (m_property->perm == INDIGO_RO_PERM)
        setEnabled(false);
    update();

    //ItemNode* item = reinterpret_cast<ItemNode*>(p->children[row]);
    //item->input_control = data;

    connect(this, &QCheckBox::clicked, this, &QIndigoSwitch::switch_clicked);
}

QIndigoSwitch::~QIndigoSwitch()
{}

void
QIndigoSwitch::update() {
    setChecked(m_item->sw.value);
}

void
QIndigoSwitch::switch_clicked(bool checked)
{
    //fprintf(stderr, "CHECK BOX TOGGLE  %d [%s]\n", checked, m_item->label);

    //  Update the switch item
    indigo_set_switch(m_property, m_item, checked);

    //  Make the GUI controls consistent with the switches
    emit(item_changed());

//    p->each_child([](ItemNode* x){
//        //ItemNode* x = reinterpret_cast<ItemNode*>(*i);
//        if (x->input_control) {
//            QCheckBox* cb = reinterpret_cast<QCheckBox*>(x->input_control);
//            cb->setCheckState(x->item->sw.value ? Qt::Checked : Qt::Unchecked);
//        }
//    });

//    for (TreeIterator i = p->children.begin(); i != p->children.end(); i++) {
//        ItemNode* x = reinterpret_cast<ItemNode*>(*i);
//        if (x->input_control) {
//            QCheckBox* cb = reinterpret_cast<QCheckBox*>(x->input_control);
//            cb->setCheckState(x->item->sw.value ? Qt::Checked : Qt::Unchecked);
//        }
//    }
}
