set(EMBER_SOURCES
    src/mug.cpp
    src/protocol.cpp
    src/service.cpp
)

set(EMBER_HEADERS
    include/ember/mug.h
    include/ember/types.h
    include/ember/ember.h
    src/service.h
    src/protocol.h
    src/constants.h
)

add_library(ember STATIC ${EMBER_SOURCES} ${EMBER_HEADERS})

target_include_directories(ember
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ember
    PUBLIC
        Qt6::Core
        Qt6::Bluetooth
)

add_library(Ember::ember ALIAS ember)
