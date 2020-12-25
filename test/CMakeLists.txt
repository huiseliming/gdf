
enable_testing()

add_executable(gdf_test gdf_test.cpp)
target_link_libraries(gdf_test PRIVATE gdf Catch2::Catch2)

add_executable(MouseAndKeyboard MouseAndKeyboard.cpp)
target_link_libraries(MouseAndKeyboard gdf)

add_executable(Main Main.cpp)
target_link_libraries(Main gdf)

 