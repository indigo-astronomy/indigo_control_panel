#!/bin/bash
make clean
rm indigo_control_panel_src/Makefile.indigo_control_panel
rm indigo_server_manager_src/Makefile.indigo_server_manager
rm Makefile
qmake
