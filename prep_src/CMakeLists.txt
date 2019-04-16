cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)


# Link against librdmacm and libibverbs. They must be installed and found
# in the library path. In addition, their header files must also be found
# in the include path.
link_libraries(rdmacm ibverbs)

# Target to build the client program
add_executable(exp_client rdma_client.c)

# Target to build the server program
add_executable(exp_server rdma_server.c)
