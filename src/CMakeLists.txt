add_subdirectory(base)
add_subdirectory(core)
# add_subdirectory(http)
add_subdirectory(redis)


if(${PROJECT_NAME}_BUILD_TESTS)
    add_executable(sponge.test unit_test.cpp)

    target_link_libraries(sponge.test
        PRIVATE
            $<LINK_LIBRARY:WHOLE_ARCHIVE,sponge::base>
            $<LINK_LIBRARY:WHOLE_ARCHIVE,sponge::core>
            # $<LINK_LIBRARY:WHOLE_ARCHIVE,sponge::http>
            $<LINK_LIBRARY:WHOLE_ARCHIVE,sponge::redis>
            doctest::doctest
    )

    target_compile_features(sponge.test PRIVATE cxx_std_23)
    add_test(NAME sponge.test COMMAND sponge.test)
endif()


add_executable(sponge-redis-server redis_server.cpp)
target_link_libraries(sponge-redis-server PUBLIC sponge::redis)

add_executable(sponge-server sponge_server.cpp)
target_link_libraries(sponge-server 
    PUBLIC 
        sponge::base 
        sponge::core
        sponge::redis 
        Boost::asio
)