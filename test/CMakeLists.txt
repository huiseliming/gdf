
enable_testing()

add_executable(gdf_test gdf_test.cpp)
target_link_libraries(gdf_test PRIVATE gdf Catch2::Catch2)

add_executable(MouseAndKeyboard MouseAndKeyboard.cpp)
target_link_libraries(MouseAndKeyboard gdf)

add_executable(App App.cpp)
target_link_libraries(App gdf)


add_custom_command(TARGET App POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/asset
        $<TARGET_FILE_DIR:App>/asset
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/fonts
        $<TARGET_FILE_DIR:App>/fonts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:App>/shaders)