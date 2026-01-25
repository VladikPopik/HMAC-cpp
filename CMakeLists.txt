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

add_executable(rotate_secret
        src/rotate_secret/main.cpp
        src/rotate_secret/rotate.cpp
)

target_link_libraries(rotate_secret
        PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        nlohmann_json::nlohmann_json
)

if(BUILD_TESTS)
# Add tests

endif()