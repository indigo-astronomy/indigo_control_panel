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
struct PropertyNode;
struct TreeNode;


class BrowserWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit BrowserWindow(QWidget *parent = nullptr);

signals:
	void enable_blobs(bool on);

public slots:
	void on_selection_changed(const QItemSelection &selected, const QItemSelection &deselected);
	void on_property_log(indigo_property* property, const char *message);
	void on_blobs_changed(bool status);
	void on_bonjour_changed(bool status);
	void on_property_changed();
	void on_exit_act();
	void on_about_act();

private:
	QPlainTextEdit* mLog;
	QTreeView* mProperties;
	QScrollArea* mScrollArea;
	QWidget* form_panel;
	QVBoxLayout* form_layout;

	ServiceModel* mServiceModel;
	PropertyModel* mPropertyModel;
	PropertyNode* current_node;
	void clear_window();
};

#endif // BROWSERWINDOW_H
