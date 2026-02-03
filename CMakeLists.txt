cmake_minimum_required(VERSION 3.14)
project(hmac_service VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest)
find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(hmac_service_lib STATIC
    src/hmac_service/config.cpp
    src/hmac_service/logging.cpp
    src/hmac_service/http.cpp
)

target_include_directories(hmac_service_lib
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(hmac_service_lib
    PUBLIC
    cpprestsdk::cpprest
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    PRIVATE
    OpenSSL::SSL
    pthread
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/hmac_service/main.cpp)
    add_executable(hmac-service
        src/hmac_service/main.cpp
    )
    
    target_link_libraries(hmac-service
        hmac_service_lib
    )
endif()

add_executable(rotate-secret
    src/rotate_secret/main.cpp
    src/rotate_secret/rotate.cpp
)

target_include_directories(rotate-secret
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rotate-secret
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)

option(BUILD_TESTS "Build tests" ON)

# Tests
if(BUILD_TESTS)
    enable_testing()
    
    # Test 1: HMAC unit tests
    add_executable(test_hmac tests/test_hmac.cpp)
    target_link_libraries(test_hmac
        hmac_service_lib
        GTest::gtest_main
    )
    
    # Test 2: Service tests  
    add_executable(test_service tests/test_service.cpp)
    target_link_libraries(test_service
        hmac_service_lib
        GTest::gtest_main
    )
    
    add_test(NAME test_hmac COMMAND test_hmac)
    add_test(NAME test_service COMMAND test_service)
endif()