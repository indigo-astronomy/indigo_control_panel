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


#ifndef QINDIGOPROPERTY_H
#define QINDIGOPROPERTY_H

#include <QFrame>
#include <QLabel>
#include <indigo_bus.h>


class QVBoxLayout;
class QIndigoItem;


class QIndigoProperty : public QWidget {
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
	void build_blob_property_form(QVBoxLayout* layout);
	void build_buttons(QVBoxLayout* layout);
	void update_property_view();

signals:

public slots:
	void update();
	void property_update(indigo_property* property);
	void set_clicked();
	void reset_clicked();

private:
	indigo_property* m_property;
	QLabel* m_led;
	QIndigoItem** m_controls;
};

#endif // QINDIGOPROPERTY_H
