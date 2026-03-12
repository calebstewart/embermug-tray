qt_add_resources(APP_RESOURCES resources/resources.qrc)

add_executable(embermug-tray
    src/main.cpp
    src/devicemonitor.cpp
    src/connectionmanager.cpp
    src/traycontroller.cpp
    ${APP_RESOURCES}
)

target_link_libraries(embermug-tray
    PRIVATE
        Ember::ember
        Qt6::Widgets
        Qt6::Svg
)

install(TARGETS embermug-tray RUNTIME DESTINATION bin)
