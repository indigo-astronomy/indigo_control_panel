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


#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <indigo_bus.h>

class QPlainTextEdit;
class QTreeView;
class ServiceModel;
class PropertyModel;
class QItemSelection;
class QVBoxLayout;
class QScrollArea;
class QIndigoServers;
struct PropertyNode;
struct TreeNode;


class BrowserWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit BrowserWindow(QWidget *parent = nullptr);
	virtual ~BrowserWindow();
	void property_define_delete(indigo_property* property, const char *message, bool action_deleted);
	void repaint_property_window(TreeNode* node);

signals:
	void enable_blobs(bool on);

public slots:
	void on_selection_changed(const QItemSelection &selected, const QItemSelection &deselected);
	void on_property_log(indigo_property* property, const char *message);
	void on_property_define(indigo_property* property, const char *message);
	void on_property_delete(indigo_property* property, const char *message);
	void on_blobs_changed(bool status);
	void on_bonjour_changed(bool status);
	void on_use_suffix_changed(bool status);
	void on_log_error();
	void on_log_info();
	void on_log_debug();
	void on_log_trace();
	void on_servers_act();
	void on_exit_act();
	void on_about_act();

private:
	QPlainTextEdit* mLog;
	QTreeView* mProperties;
	QScrollArea* mScrollArea;
	QWidget* form_panel;
	QVBoxLayout* form_layout;

	QIndigoServers *mIndigoServers;
	ServiceModel* mServiceModel;
	PropertyModel* mPropertyModel;
	TreeNode* current_node;

	void clear_window();
};

#endif // BROWSERWINDOW_H
