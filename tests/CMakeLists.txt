cmake_minimum_required(VERSION 3.17)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Prevent gtest from overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(CppSlimTests
    TestSlim.cpp
    SlimUtilTest.cpp
    SymbolTableTest.cpp
    SlimListTest.cpp
    SlimListSerializerTest.cpp
    SlimListDeserializerTest.cpp
    StatementExecutorTest.cpp
    ListExecutorTest.cpp
    SlimConnectionHandlerTest.cpp
)

target_compile_features(CppSlimTests PRIVATE cxx_std_17)

target_include_directories(CppSlimTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/CppSlim
)

target_link_libraries(CppSlimTests PRIVATE
    CppSlimLib
    GTest::gtest
    GTest::gtest_main
)

if(MSVC)
    target_compile_options(CppSlimTests PRIVATE
        /W4
        /D_CRT_SECURE_NO_DEPRECATE
        /D_CRT_NONSTDC_NO_DEPRECATE
    )
endif()

include(GoogleTest)
gtest_discover_tests(CppSlimTests)
