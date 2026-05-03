project(CppSlim-project)
cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------------------------------------------------------------------
# CppSlimFixtures — header-only fixture-authoring library
#   Provides: CppFixtures.h, SlimFixture.h (and their transitive headers)
#   Fixture libraries link against this target only; no protocol code needed.
# ---------------------------------------------------------------------------
add_library(CppSlimFixtures INTERFACE)

target_compile_features(CppSlimFixtures INTERFACE cxx_std_17)

target_include_directories(CppSlimFixtures INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CppSlim
)

# ---------------------------------------------------------------------------
# CppSlimLib — protocol/transport library (no fixture code, no main)
# ---------------------------------------------------------------------------
add_library(CppSlimLib STATIC
    src/SlimUtil.cpp
    src/SlimList.cpp
    src/SlimListSerializer.cpp
    src/SlimListDeserializer.cpp
    src/SymbolTable.cpp
    src/StatementExecutor.cpp
    src/ListExecutor.cpp
    src/SlimConnectionHandler.cpp
    src/Slim.cpp
    src/SocketServer.cpp
    src/TcpComLink.cpp
)

target_link_libraries(CppSlimLib PUBLIC CppSlimFixtures)

if(WIN32)
    target_link_libraries(CppSlimLib PUBLIC ws2_32)
endif()

if(MSVC)
    target_compile_options(CppSlimLib PRIVATE
        /W4
        /D_CRT_SECURE_NO_DEPRECATE
        /D_CRT_NONSTDC_NO_DEPRECATE
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(CppSlimLib PRIVATE -Wall -Wextra)
endif()

# ---------------------------------------------------------------------------
# Fixtures — example fixture library, depends on CppSlimFixtures only
#   Demonstrates that fixture code needs no knowledge of the protocol layer.
# ---------------------------------------------------------------------------
add_library(Fixtures STATIC
    fixtures/Fixtures.cpp
    fixtures/Division.cpp
    fixtures/Count.cpp
    fixtures/EmployeePayRecords.cpp
)

target_link_libraries(Fixtures PUBLIC CppSlimFixtures)

# ---------------------------------------------------------------------------
# CppSlim executable
# ---------------------------------------------------------------------------
add_executable(CppSlim src/main.cpp)

target_link_libraries(CppSlim PRIVATE CppSlimLib Fixtures)

if(MSVC)
    target_compile_options(CppSlim PRIVATE
        /W4
        /D_CRT_SECURE_NO_DEPRECATE
        /D_CRT_NONSTDC_NO_DEPRECATE
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(CppSlim PRIVATE -Wall -Wextra)
endif()

install(TARGETS CppSlim CppSlimLib CppSlimFixtures
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
)

# ---------------------------------------------------------------------------
# Unit tests (gtest, fetched at configure time)
# ---------------------------------------------------------------------------
add_subdirectory(tests)

# ---------------------------------------------------------------------------
# FitNesse jar — download latest release from GitHub
#   Usage: cmake --build build --target fitnesse_jar
# ---------------------------------------------------------------------------
add_custom_target(fitnesse_jar
    COMMAND ${CMAKE_COMMAND}
        -DLIB_DIR=${CMAKE_SOURCE_DIR}/lib
        -P ${CMAKE_SOURCE_DIR}/cmake/FetchFitNesse.cmake
    COMMENT "Downloading latest fitnesse-standalone.jar"
)
