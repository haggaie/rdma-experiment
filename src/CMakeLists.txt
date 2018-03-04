cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

link_libraries(rdmacm ibverbs)
add_executable(rdma_client rdma_client.c)
add_executable(rdma_server rdma_server.c)
