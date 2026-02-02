cmake_minimum_required(VERSION 3.15)
project(HMAC_Service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64")

find_package(OpenSSL REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(nlohmann_json 3.6.1 REQUIRED)

include_directories(src)
include_directories(include)

add_executable(hmac_service
        src/hmac_service/main.cpp
        src/hmac_service/config.cpp
        src/hmac_service/http.cpp
        src/hmac_service/logging.cpp
)

target_link_libraries(hmac_service
        PRIVATE
        cpprestsdk::cpprest
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
)

add_executable(rotate-secret
        src/rotate_secret/main.cpp
        src/rotate_secret/rotate.cpp
)

target_link_libraries(rotate-secret
        PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
)

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
    
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0 
    )
    
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(googletest)
    
    add_executable(tests
        tests/test_hmac.cpp
        src/hmac_service/main.cpp
        src/hmac_service/config.cpp
        src/hmac_service/http.cpp
        src/hmac_service/logging.cpp
    )
    
    target_link_libraries(tests
        PRIVATE
        gtest_main
        gmock
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
        cpprestsdk::cpprest
    )
    
    target_include_directories(tests PRIVATE
        src/hmac_service/
        include/
    )
    
    add_test(NAME UnitTests COMMAND tests)
    
    set_target_properties(tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
endif()