if(AU_USE_VENDORED_GTEST)
    include(FetchContent)
    FetchContent_Declare(
       googletest
       GIT_REPOSITORY https://github.com/google/googletest.git
       GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850 # Release 1.12.1
       FIND_PACKAGE_ARGS
          1.12.1
          NAMES GTest
    )

    # https://google.github.io/googletest/quickstart-cmake.html
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)
else()
    find_package(GTest REQUIRED)
endif()

include(GoogleTest)

file(GLOB TESTS CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

foreach(test_file ${TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_executable(${test_name} ${test_file})
    target_link_libraries(${test_name} PRIVATE Au::au GTest::gmock_main)
    gtest_discover_tests(${test_name})
endforeach()
