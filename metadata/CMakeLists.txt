add_contract(metadata metadata ${CMAKE_CURRENT_SOURCE_DIR}/src/metadata.cpp)
target_include_directories(metadata.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include
   ${CMAKE_CURRENT_SOURCE_DIR}/../eosio.system/include)

set_target_properties(metadata.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
