add_executable(render_engine main.cpp)

target_link_libraries(render_engine PUBLIC logger)
target_link_libraries(render_engine PUBLIC rasterizer)
target_link_libraries(render_engine PUBLIC window)