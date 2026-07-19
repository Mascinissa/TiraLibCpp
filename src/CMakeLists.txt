set(INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${TIRAMISU_INSTALL}/include/
)

option(USE_SQLITE "Use SQLITE" OFF)

if(USE_SQLITE)
    MESSAGE(STATUS "USE_SQLITE")
    add_definitions(-DUSE_SQLITE)
endif()

set(HEADER_FILES
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/utils.h
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/actions.h
)

if(USE_SQLITE)
    list(APPEND HEADER_FILES ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/dbhelpers.h)
endif()

set(SOURCES utils.cc actions.cc)

if(USE_SQLITE)
    list(APPEND SOURCES dbhelpers.cc)
endif()

add_library(TiraLibCPP SHARED ${SOURCES})

target_include_directories(TiraLibCPP PUBLIC ${INCLUDES})
target_link_directories(TiraLibCPP PUBLIC ${TIRAMISU_INSTALL}/lib)

set(LIBRARIES tiramisu tiramisu_auto_scheduler Halide isl)

if(USE_SQLITE)
    list(APPEND LIBRARIES sqlite3)
endif()

target_link_libraries(TiraLibCPP ${LIBRARIES})
set_target_properties(TiraLibCPP PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")
install(TARGETS TiraLibCPP LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include/${PROJECT_NAME})
