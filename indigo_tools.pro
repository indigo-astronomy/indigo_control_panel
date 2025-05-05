TEMPLATE = subdirs

# Always build the control panel
SUBDIRS += indigo_control_panel

# Only build server manager on Linux
unix:!macx {
    SUBDIRS += indigo_server_manager
}

# Define the project files for each subdir
indigo_control_panel.file = indigo_control_panel.pro
indigo_server_manager.file = indigo_server_manager/indigo_server_manager.pro