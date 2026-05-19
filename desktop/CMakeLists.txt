cmake_minimum_required(VERSION 3.24)

project(CampusBuddyDesktop VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

set(CAMPUS_BUDDY_REQUIRED_QT_VERSION "6.10.3" CACHE STRING "Required Qt 6 version for the desktop technical spike")

find_package(Qt6 ${CAMPUS_BUDDY_REQUIRED_QT_VERSION} REQUIRED COMPONENTS Widgets Test Network)

set(DESKTOP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/SecureTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthTokenStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/LoginWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/LoginWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/RegisterWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/RegisterWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/HomePageWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/HomePageWidget.h
)

qt_add_executable(campus_buddy_desktop ${DESKTOP_SOURCES})

target_link_libraries(campus_buddy_desktop PRIVATE Qt6::Widgets Qt6::Network)

target_include_directories(campus_buddy_desktop PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

enable_testing()

qt_add_executable(api_client_config_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ApiClientConfigTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.h
)

target_link_libraries(api_client_config_test PRIVATE Qt6::Test)

target_include_directories(api_client_config_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_test(NAME api_client_config_test COMMAND api_client_config_test)

qt_add_executable(campus_api_client_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/CampusApiClientTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.h
)

target_link_libraries(campus_api_client_test PRIVATE Qt6::Test Qt6::Network)

target_include_directories(campus_api_client_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(campus_api_client_test PRIVATE
    CAMPUS_BUDDY_DESKTOP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME campus_api_client_test COMMAND campus_api_client_test)

qt_add_executable(auth_token_store_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/AuthTokenStoreTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/SecureTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthTokenStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthTokenStore.h
)

target_link_libraries(auth_token_store_test PRIVATE Qt6::Test)

target_include_directories(auth_token_store_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_test(NAME auth_token_store_test COMMAND auth_token_store_test)
