// Copyright (c) 2019 Rumen G.Bogdanovski & David Hulse
// All rights reserved.
//
// You can use this software under the terms of 'INDIGO Astronomy
// open-source license' (see LICENSE.md).
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS 'AS IS' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


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
