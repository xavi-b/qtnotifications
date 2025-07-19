# Build C++ examples
add_subdirectory(cpp)

# Build Qt Widgets examples if Qt::Widgets is available
if(TARGET Qt6::Widgets)
    add_subdirectory(widgets)
endif()

# Build QML examples if Qt::Qml is available
if(TARGET Qt6::Qml)
    add_subdirectory(qml)
endif()