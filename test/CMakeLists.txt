
macro(my_add_test test_name)
add_executable(${test_name} ${test_name}.cpp)
target_link_libraries(${test_name} gdf)
endmacro(${test_name})

my_add_test(MouseAndKeyboard)

#add_executable(MouseAndKeyboard MouseAndKeyboard.cpp)
#target_link_libraries(MouseAndKeyboard Renderer)

add_executable(test Main.cpp)
target_link_libraries(test gdf)

