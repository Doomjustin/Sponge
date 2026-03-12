configure_file(spg_base_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/spg_base_config.h @ONLY)

file(GLOB_RECURSE SPONGE_BASE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(sponge_base STATIC ${SPONGE_BASE_SOURCES})

add_library(sponge::base ALIAS sponge_base)

target_include_directories(sponge_base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(sponge_base PUBLIC cxx_std_23)

target_link_libraries(sponge_base PUBLIC doctest::doctest Boost::asio)