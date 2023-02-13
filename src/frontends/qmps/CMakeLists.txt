add_subdirectory(data)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(umps3
        qmps.qrc
        main.cc
        ../../include/qmps/application.h
        application.cc
        ../../include/qmps/debug_session.h
        debug_session.cc
        ../../include/qmps/monitor_window.h
        ../../include/qmps/monitor_window_priv.h
        monitor_window.cc
        error_hooks.cc
        ../../include/qmps/create_machine_dialog.h
        create_machine_dialog.cc
        ../../include/qmps/stop_mask_view.h
        stop_mask_view.cc
        ../../include/qmps/processor_list_model.h
        processor_list_model.cc
        ../../include/qmps/machine_config_dialog.h
        ../../include/qmps/machine_config_dialog_priv.h
        machine_config_dialog.cc
        ../../include/qmps/machine_config_view.h
        machine_config_view.cc
        ../../include/qmps/trace_browser.h
        ../../include/qmps/trace_browser_priv.h
        trace_browser.cc
        ../../include/qmps/memory_view_delegate.h
        ../../include/qmps/hex_view.h
        ../../include/qmps/hex_view_priv.h
        hex_view.cc
        ../../include/qmps/symbol_table_model.h
        symbol_table_model.cc
        ../../include/qmps/processor_window.h
        processor_window.cc
        ../../include/qmps/code_view.h
        ../../include/qmps/code_view_priv.h
        code_view.cc
        ../../include/qmps/register_set_snapshot.h
        register_set_snapshot.cc
        ../../include/qmps/register_set_widget.h
        register_set_widget.cc
        ../../include/qmps/tlb_model.h
        tlb_model.cc
        ../../include/qmps/terminal_view.h
        terminal_view.cc
        ../../include/qmps/terminal_window.h
        ../../include/qmps/terminal_window_priv.h
        terminal_window.cc
        ../../include/qmps/cpu_status_map.h
        cpu_status_map.cc
        ../../include/qmps/stoppoint_list_model.h
        stoppoint_list_model.cc
        ../../include/qmps/suspect_type_delegate.h
        suspect_type_delegate.cc
        ../../include/qmps/device_tree_view.h
        device_tree_view.cc
        ../../include/qmps/device_tree_model.h
        device_tree_model.cc
        ../../include/qmps/add_breakpoint_dialog.h
        add_breakpoint_dialog.cc
        ../../include/qmps/add_suspect_dialog.h
        add_suspect_dialog.cc
        ../../include/qmps/add_tracepoint_dialog.h
        add_tracepoint_dialog.cc
        ../../include/qmps/address_line_edit.h
        address_line_edit.cc
        ../../include/qmps/mac_id_edit.h
        mac_id_edit.cc
        ../../include/qmps/boolean_item_delegate.h
        boolean_item_delegate.cc
        ../../include/qmps/register_item_delegate.h
        register_item_delegate.cc
        ../../include/qmps/ui_utils.h
        ui_utils.cc
        ../../include/qmps/tree_view.h
        tree_view.cc
        ../../include/qmps/flat_push_button.h
        flat_push_button.cc)

target_include_directories(umps3
        PRIVATE
        ${PROJECT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/frontends
        ${PROJECT_SOURCE_DIR}/src/include)

target_link_libraries(umps3 PRIVATE Qt5::Widgets)

set(QT_DEFINES -DQT_NO_DEBUG -DQT_NO_KEYWORDS -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED)
target_compile_options(umps3 PRIVATE ${QT_DEFINES} ${SIGCPP_CFLAGS})

add_dependencies(umps3 uriscv)

target_compile_definitions(umps3 PRIVATE -DPACKAGE_VERSION="${PROJECT_VERSION}")

target_link_libraries(umps3
        PRIVATE
        uriscv
        Qt5::Widgets
        ${SIGCPP_LIBRARIES}
        ${LIBDL})

install(TARGETS umps3
        RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR})
