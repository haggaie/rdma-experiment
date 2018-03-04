cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

# Shared example files
add_library(common STATIC
  common.c
  )

# Link against librdmacm and libibverbs. They must be installed and found
# in the library path. In addition, their header files must also be found
# in the include path.
link_libraries(rdmacm ibverbs common)

# Target to build the client program
add_executable(rdma_client rdma_client.c)

# Target to build the server program
add_executable(rdma_server rdma_server.c)
