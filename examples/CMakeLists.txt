add_executable(demo demo.c)
target_link_libraries(demo NaiveRainbow)

add_executable(coverage coverage.c)
target_link_libraries(coverage NaiveRainbow)

add_executable(print_matrix print_matrix.c)
target_link_libraries(print_matrix NaiveRainbow)

add_executable(print_table print_table.c)
target_link_libraries(print_table NaiveRainbow)

add_executable(store_tables store_tables.c)
target_link_libraries(store_tables NaiveRainbow)