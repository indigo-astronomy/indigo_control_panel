#include <QObject>
#include "qindigoproperty.h"
#include "qindigoswitch.h"

QIndigoItem::~QIndigoItem(){

}


QIndigoSwitch::QIndigoSwitch(QIndigoProperty* p, indigo_property* property, indigo_item* item)
	: QCheckBox(), QIndigoItem(p, property, item) {

	this->setObjectName("INDIGO_property");
	setText(m_item->label);
	if (m_property->perm == INDIGO_RO_PERM) setEnabled(false);
	update();

	connect(this, &QCheckBox::clicked, this, &QIndigoSwitch::switch_clicked);
}

QIndigoSwitch::~QIndigoSwitch(){
}

void QIndigoSwitch::update() {
	setChecked(m_item->sw.value);
}

void QIndigoSwitch::switch_clicked(bool checked) {
	//  Update the switch item
	indigo_set_switch(m_property, m_item, checked);

	//  Make the GUI controls consistent with the switches
	emit(item_changed());
}
