set(PROJECTS_OUT_DIR ${PROJECT_BINARY_DIR}/projects_out)
file(MAKE_DIRECTORY ${PROJECTS_OUT_DIR})

function(create_example project_name)
    message(STATUS "RTNeural -- Configuring project: ${project_name}")
    add_executable(${project_name} ${project_name}.cpp)
    target_include_directories(${project_name} PUBLIC ${CMAKE_SOURCE_DIR})
    target_link_libraries(${project_name} PUBLIC RTNeural)
    target_compile_features(${project_name} PRIVATE cxx_std_17)

    add_custom_command(TARGET ${project_name}
                       POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E echo "copying $<TARGET_FILE:${project_name}> to ${PROJECTS_OUT_DIR}/${project_name}"
                       COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${project_name}> ${PROJECTS_OUT_DIR}/${project_name})
endfunction()

add_subdirectory(hello_rtneural)
add_subdirectory(torch_gru)
add_subdirectory(torch_gru_chowtape)
