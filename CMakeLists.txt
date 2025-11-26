cmake_minimum_required(VERSION 3.15)
project(HMAC_Service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(cpprestsdk REQUIRED)

add_executable(hmac_service

)

target_link_libraries(hmac_service
        PRIVATE
        cpprestsdk::cpprest
        OpenSSL::SSL
        OpenSSL::Crypto
)

# Optional
# add_executable(rotate_secret
# )

# target_link_libraries(rotate_secret
#         PRIVATE
#         OpenSSL::SSL
#         OpenSSL::Crypto
# )

if(BUILD_TESTS)
# Add tests

endif()