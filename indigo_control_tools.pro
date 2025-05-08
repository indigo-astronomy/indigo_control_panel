TEMPLATE = subdirs

# Always build the control panel
SUBDIRS += indigo_control_panel_src

# Build server manager on Linux only
unix:!macx {
	SUBDIRS += indigo_server_manager_src
}

indigo_control_panel_src.file = indigo_control_panel_src/indigo_control_panel.pro
indigo_server_manager_src.file = indigo_server_manager_src/indigo_server_manager.pro

DISTFILES += \
	README.md \
	LICENCE.md
