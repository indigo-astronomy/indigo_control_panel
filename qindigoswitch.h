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


class QIndigoSwitch : public QCheckBox, public QIndigoItem {
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
