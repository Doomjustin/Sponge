file(GLOB_RECURSE SPONGE_REDIS_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(sponge_redis STATIC ${SPONGE_REDIS_SOURCES})

add_library(sponge::redis ALIAS sponge_redis)

target_compile_features(sponge_redis PUBLIC cxx_std_23)

target_include_directories(sponge_redis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(sponge_redis 
    PUBLIC 
        Boost::asio 
        Boost::beast       
        sponge::base 
)