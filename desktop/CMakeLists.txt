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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AppStyles.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AppStyles.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/UiHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/UiHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/RegisterWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/RegisterWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/HomePageWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/HomePageWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/IdentityVerificationWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/IdentityVerificationWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/PartnerPostModels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ContactConversationModels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/PartnerPostApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/PartnerPostApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ContactConversationApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ContactConversationApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/MyPartnerPostModels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/MyPartnerPostApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/MyPartnerPostApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ReviewCreditModels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ReviewCreditApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ReviewCreditApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PostEditorWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PostEditorWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MyPostsWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/MyPostsWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PlazaWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/PlazaWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ConversationsWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ConversationsWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ReviewCreditWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/ReviewCreditWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/AdminReviewModels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/AdminReviewApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/AdminReviewApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AdminReviewWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/AdminReviewWidget.h
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

qt_add_executable(qt_server_integration_smoke
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/QtServerIntegrationSmoke.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/PartnerPostApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/PartnerPostApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ContactConversationApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ContactConversationApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/SecureTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthTokenStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/PartnerPostModels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ContactConversationModels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/MyPartnerPostModels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/MyPartnerPostApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/MyPartnerPostApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ReviewCreditModels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ReviewCreditApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ReviewCreditApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/AdminReviewModels.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/AdminReviewApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/AdminReviewApiService.h
)

target_link_libraries(qt_server_integration_smoke PRIVATE Qt6::Core Qt6::Network)

target_include_directories(qt_server_integration_smoke PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

qt_add_executable(server_smoke_security_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ServerSmokeSecurityTest.cpp
)

target_link_libraries(server_smoke_security_test PRIVATE Qt6::Test)

target_include_directories(server_smoke_security_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(server_smoke_security_test PRIVATE
    CAMPUS_BUDDY_DESKTOP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME server_smoke_security_test COMMAND server_smoke_security_test)

qt_add_executable(partner_post_api_service_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/PartnerPostApiServiceTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/PartnerPostApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/PartnerPostApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/SecureTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/PartnerPostModels.h
)

target_link_libraries(partner_post_api_service_test PRIVATE Qt6::Test Qt6::Network)

target_include_directories(partner_post_api_service_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_test(NAME partner_post_api_service_test COMMAND partner_post_api_service_test)

qt_add_executable(contact_conversation_api_service_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ContactConversationApiServiceTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ContactConversationApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ContactConversationApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/SecureTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ContactConversationModels.h
)

target_link_libraries(contact_conversation_api_service_test PRIVATE Qt6::Test Qt6::Network)

target_include_directories(contact_conversation_api_service_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_test(NAME contact_conversation_api_service_test COMMAND contact_conversation_api_service_test)

qt_add_executable(my_partner_post_api_service_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/MyPartnerPostApiServiceTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/MyPartnerPostApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/MyPartnerPostApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/SecureTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/MyPartnerPostModels.h
)

target_link_libraries(my_partner_post_api_service_test PRIVATE Qt6::Test Qt6::Network)

target_include_directories(my_partner_post_api_service_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_test(NAME my_partner_post_api_service_test COMMAND my_partner_post_api_service_test)

qt_add_executable(student_post_plaza_widget_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/StudentPostPlazaWidgetTest.cpp
)

target_link_libraries(student_post_plaza_widget_test PRIVATE Qt6::Test)

target_include_directories(student_post_plaza_widget_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(student_post_plaza_widget_test PRIVATE
    CAMPUS_BUDDY_DESKTOP_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME student_post_plaza_widget_test COMMAND student_post_plaza_widget_test)

qt_add_executable(review_credit_api_service_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ReviewCreditApiServiceTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ReviewCreditApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/ReviewCreditApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/SecureTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ReviewCreditModels.h
)

target_link_libraries(review_credit_api_service_test PRIVATE Qt6::Test Qt6::Network)

target_include_directories(review_credit_api_service_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_test(NAME review_credit_api_service_test COMMAND review_credit_api_service_test)

qt_add_executable(admin_review_api_service_test
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/AdminReviewApiServiceTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/CampusApiClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/AdminReviewApiService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/api/AdminReviewApiService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/SecureTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/InMemorySessionTokenStore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/ApiClientConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/domain/AdminReviewModels.h
)

target_link_libraries(admin_review_api_service_test PRIVATE Qt6::Test Qt6::Network)

target_include_directories(admin_review_api_service_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_test(NAME admin_review_api_service_test COMMAND admin_review_api_service_test)
