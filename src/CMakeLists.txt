qt_feature_evaluate_features("${CMAKE_CURRENT_SOURCE_DIR}/../configure.cmake")

add_subdirectory(notifications)

if(ANDROID)
    add_subdirectory(android)
endif()