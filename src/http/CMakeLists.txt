file(GLOB_RECURSE SPONGE_HTTP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(sponge_http STATIC ${SPONGE_HTTP_SOURCES})

add_library(sponge::http ALIAS sponge_http)

target_compile_features(sponge_http PUBLIC cxx_std_23)

target_include_directories(sponge_http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(sponge_http 
    PUBLIC 
        Boost::asio 
        Boost::beast   
        sponge::base     
)