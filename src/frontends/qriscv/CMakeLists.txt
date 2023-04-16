add_subdirectory(data)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(uriscv
        qriscv.qrc
        main.cc
        ../../include/qriscv/application.h
        application.cc
        ../../include/qriscv/debug_session.h
        debug_session.cc
        ../../include/qriscv/monitor_window.h
        ../../include/qriscv/monitor_window_priv.h
        monitor_window.cc
        error_hooks.cc
        ../../include/qriscv/create_machine_dialog.h
        create_machine_dialog.cc
        ../../include/qriscv/stop_mask_view.h
        stop_mask_view.cc
        ../../include/qriscv/processor_list_model.h
        processor_list_model.cc
        ../../include/qriscv/machine_config_dialog.h
        ../../include/qriscv/machine_config_dialog_priv.h
        machine_config_dialog.cc
        ../../include/qriscv/machine_config_view.h
        machine_config_view.cc
        ../../include/qriscv/trace_browser.h
        ../../include/qriscv/trace_browser_priv.h
        trace_browser.cc
        ../../include/qriscv/memory_view_delegate.h
        ../../include/qriscv/hex_view.h
        ../../include/qriscv/hex_view_priv.h
        hex_view.cc
        ../../include/qriscv/symbol_table_model.h
        symbol_table_model.cc
        ../../include/qriscv/processor_window.h
        processor_window.cc
        ../../include/qriscv/code_view.h
        ../../include/qriscv/code_view_priv.h
        code_view.cc
        ../../include/qriscv/register_set_snapshot.h
        register_set_snapshot.cc
        ../../include/qriscv/register_set_widget.h
        register_set_widget.cc
        ../../include/qriscv/tlb_model.h
        tlb_model.cc
        ../../include/qriscv/terminal_view.h
        terminal_view.cc
        ../../include/qriscv/terminal_window.h
        ../../include/qriscv/terminal_window_priv.h
        terminal_window.cc
        ../../include/qriscv/cpu_status_map.h
        cpu_status_map.cc
        ../../include/qriscv/stoppoint_list_model.h
        stoppoint_list_model.cc
        ../../include/qriscv/suspect_type_delegate.h
        suspect_type_delegate.cc
        ../../include/qriscv/device_tree_view.h
        device_tree_view.cc
        ../../include/qriscv/device_tree_model.h
        device_tree_model.cc
        ../../include/qriscv/add_breakpoint_dialog.h
        add_breakpoint_dialog.cc
        ../../include/qriscv/add_suspect_dialog.h
        add_suspect_dialog.cc
        ../../include/qriscv/add_tracepoint_dialog.h
        add_tracepoint_dialog.cc
        ../../include/qriscv/address_line_edit.h
        address_line_edit.cc
        ../../include/qriscv/mac_id_edit.h
        mac_id_edit.cc
        ../../include/qriscv/boolean_item_delegate.h
        boolean_item_delegate.cc
        ../../include/qriscv/register_item_delegate.h
        register_item_delegate.cc
        ../../include/qriscv/ui_utils.h
        ui_utils.cc
        ../../include/qriscv/tree_view.h
        tree_view.cc
        ../../include/qriscv/flat_push_button.h
        flat_push_button.cc)

target_include_directories(uriscv
        PRIVATE
        ${PROJECT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/frontends
        ${PROJECT_SOURCE_DIR}/src/include)

target_link_libraries(uriscv PRIVATE Qt6::Widgets)

set(QT_DEFINES -DQT_NO_DEBUG -DQT_NO_KEYWORDS -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED)
target_compile_options(uriscv PRIVATE ${QT_DEFINES} ${SIGCPP_CFLAGS})

add_dependencies(uriscv uriscv-lib)

target_compile_definitions(uriscv PRIVATE -DPACKAGE_VERSION="${PROJECT_VERSION}")

target_link_libraries(uriscv
        PRIVATE
        uriscv-lib
        Qt6::Widgets
        ${SIGCPP_LIBRARIES}
        ${LIBDL})

install(TARGETS uriscv
        RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR})
