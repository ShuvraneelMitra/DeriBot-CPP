cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)
project(DeriBot VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(DeriBotConfig.h.in DeriBotConfig.h)

include(FetchContent)
FetchContent_Declare(
    websocketpp
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
    GIT_TAG master
)
FetchContent_MakeAvailable(websocketpp)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281)
FetchContent_MakeAvailable(fmt)

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

add_executable(DeriBot src/password.cpp
                       src/deribit_api.cpp
                       src/utils.cpp
                       deribot.cpp)

target_include_directories(DeriBot PRIVATE ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${websocketpp_SOURCE_DIR} ${fmt_SOURCE_DIR})
target_include_directories(DeriBot PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
target_include_directories(DeriBot PUBLIC src)
target_link_libraries(DeriBot PRIVATE Boost::system Boost::thread OpenSSL::SSL OpenSSL::Crypto fmt::fmt)
