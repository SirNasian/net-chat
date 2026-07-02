cmake_minimum_required(VERSION 4.3.3)
project(cmake-chat)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(vendor/enet)

add_executable(net-chat-client)
target_sources(net-chat-client PRIVATE src/client.c src/chat_interface.c src/network.c)
target_include_directories(net-chat-client PRIVATE vendor/enet/include)
target_link_libraries(net-chat-client PRIVATE enet ncurses)

add_executable(net-chat-server)
target_sources(net-chat-server PRIVATE src/server.c src/network.c)
target_include_directories(net-chat-server PRIVATE vendor/enet/include)
target_link_libraries(net-chat-server PRIVATE enet)
