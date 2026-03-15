file(GLOB_RECURSE SPONGE_CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(sponge_core STATIC ${SPONGE_CORE_SOURCES})

add_library(sponge::core ALIAS sponge_core)

target_compile_features(sponge_core PUBLIC cxx_std_23)

target_compile_definitions(sponge_core PUBLIC BOOST_ASIO_HAS_IO_URING)

target_include_directories(sponge_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(sponge_core 
    PUBLIC 
        Boost::asio 
        Boost::beast
        sponge::base
        uring
)