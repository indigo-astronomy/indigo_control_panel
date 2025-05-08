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


#ifndef QINDIGOTEXT_H
#define QINDIGOTEXT_H

#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QWidget>
#include "qindigoswitch.h"


class QIndigoText : public QWidget, public QIndigoItem {
	Q_OBJECT

public:
	explicit QIndigoText(QIndigoProperty* p, indigo_property* property, indigo_item* item, QWidget *parent = nullptr);
	virtual ~QIndigoText();

	virtual void update();
	virtual void reset();
	virtual void apply();

signals:

public slots:
	void dirty();

private:
	QLabel* label;
	QLineEdit* text;
	QPlainTextEdit* text_edit;
	bool m_dirty;
};

#endif // QINDIGOTEXT_H
