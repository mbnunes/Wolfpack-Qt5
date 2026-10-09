cmake_minimum_required(VERSION 3.16)
project(wolfpack VERSION 13.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/release")

# Find Dependencies
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Sql Xml)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Embed)
find_package(Boost REQUIRED COMPONENTS python314)
find_package(ZLIB REQUIRED)

# Common sources
set(WOLFPACK_SOURCES
    src/accounts.cpp
    src/action.cpp
    src/basechar.cpp
    src/basedef.cpp
    src/basics.cpp
    src/combat.cpp
    src/commands.cpp
    src/content.cpp
    src/serverconfig.cpp
    src/console.cpp
    src/contextmenu.cpp
    src/coord.cpp
    src/corpse.cpp
    src/customtags.cpp
    src/dbdriver.cpp
    src/definable.cpp
    src/definitions.cpp
    src/dragdrop.cpp
    src/exportdefinitions.cpp
    src/guilds.cpp
    src/gumps.cpp
    src/items.cpp
    src/log.cpp
    src/mapobjects.cpp
    src/multi.cpp
    src/npc.cpp
    src/optionparser.cpp
    src/party.cpp
    src/pathfinding.cpp
    src/persistentbroker.cpp
    src/persistentobject.cpp
    src/player.cpp
    src/preferences.cpp
    src/profile.cpp
    src/pythonscript.cpp
    src/quests.cpp
    src/scriptmanager.cpp
    src/server.cpp
    src/skills.cpp
    src/spawnregions.cpp
    src/speech.cpp
    src/targetrequests.cpp
    src/territories.cpp
    src/timing.cpp
    src/timers.cpp
    src/trade.cpp
    src/uobject.cpp
    src/walking.cpp
    src/world.cpp
    src/twofish/twofish2.cpp
)

# AI sources
list(APPEND WOLFPACK_SOURCES
    src/ai/ai.cpp
    src/ai/ai_animals.cpp
    src/ai/ai_humans.cpp
    src/ai/ai_monsters.cpp
    src/ai/ai_mage.cpp
    src/ai/ai_bladespirit.cpp
    src/ai/ai_commoner.cpp
    src/ai/ai_energyvortex.cpp
)

# Muls sources
list(APPEND WOLFPACK_SOURCES
    src/muls/maps.cpp
    src/muls/multiscache.cpp
    src/muls/tilecache.cpp
)

# Network sources
list(APPEND WOLFPACK_SOURCES
    src/network/clientversion.cpp
    src/network/encryption.cpp
    src/network/network.cpp
    src/network/uopacket.cpp
    src/network/uorxpackets.cpp
    src/network/uosocket.cpp
    src/network/uotxpackets.cpp
)

# Python sources
list(APPEND WOLFPACK_SOURCES
    src/python/char.cpp
    src/python/converters.cpp
    src/python/engine.cpp
    src/python/global.cpp
    src/python/item.cpp
    src/python/pyaccount.cpp
    src/python/pyaction.cpp
    src/python/pyai.cpp
    src/python/pycoord.cpp
    src/python/pypacket.cpp
    src/python/pyprofiler.cpp
    src/python/pyregion.cpp
    src/python/pyspawnregion.cpp
    src/python/pytooltip.cpp
    src/python/region.cpp
    src/python/socket.cpp
    src/python/target.cpp
    src/python/worlditerator.cpp
)

# GUI sources & resources
set(WOLFPACK_GUI_SOURCES
    src/gui/wpmain.cpp
    src/gui/mainwindow.cpp
    src/gui/lineedithistory.cpp
    src/gui/profilersessionmodel.cpp
    src/gui/profilerwindow.cpp
    src/gui/pythoninteractivewindow.cpp
    src/gui/trayicon/trayicon.cpp
    src/gui/mainwindow.ui
    src/gui/profilerwindow.ui
    src/gui/gui.qrc
)

if(UNIX)
    list(APPEND WOLFPACK_SOURCES
        src/unix/config_unix.cpp
    )
elseif(WIN32)
    list(APPEND WOLFPACK_SOURCES
        src/win/config_win.cpp
        build/res.rc
    )
endif()

add_executable(wolfpack ${WOLFPACK_SOURCES} ${WOLFPACK_GUI_SOURCES})

target_include_directories(wolfpack PRIVATE
    src
    src/network
    src/python
    src/ai
    src/muls
    src/gui
    src/twofish
    ${Python3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

target_compile_definitions(wolfpack PRIVATE
    BOOST_ALL_NO_LIB
)

target_link_libraries(wolfpack PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Xml
    Python3::Python
    Boost::python314
    ZLIB::ZLIB
)

if(UNIX)
    add_executable(wolfpack-console ${WOLFPACK_SOURCES} src/unix/console_unix.cpp)

    target_include_directories(wolfpack-console PRIVATE
        src
        src/network
        src/python
        src/ai
        src/muls
        src/twofish
        ${Python3_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
    )

    target_compile_definitions(wolfpack-console PRIVATE
        BOOST_ALL_NO_LIB
    )

    target_link_libraries(wolfpack-console PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Sql
        Qt6::Xml
        Python3::Python
        Boost::python314
        ZLIB::ZLIB
    )
endif()

if(WIN32)
    target_link_libraries(wolfpack PRIVATE ws2_32)
endif()
