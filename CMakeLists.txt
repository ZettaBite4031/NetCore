cmake_minimum_required(VERSION 3.20)
project(netcore LANGUAGES CXX VERSION 0.1.0)

option(NETCORE_WITH_BEAST       "Build Beast transport" ON)
option(NETCORE_WITH_CURL        "Build cURL transport" ON)
option(NETCORE_BUILD_TESTS      "Build unit tests" ON)
option(NETCORE_BUILD_EXAMPLES   "Build examples" ON)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(netcore
    src/beast_http_transport.cpp
    src/beast_ws_transport.cpp
    src/curl_http_transport.cpp
    src/err.cpp
    src/http_client.cpp
    src/log_sinks.cpp
    src/log.cpp
    src/transport_factory.cpp
    src/url.cpp
    src/webhook_client.cpp
    src/ws_session.cpp
)

target_include_directories(netcore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(netcore PRIVATE
    $<$<BOOL:${NETCORE_WITH_HTTPLIB}>:CPPHTTPLIB_OPENSSL_SUPPORT
)

if (NETCORE_WITH_BEAST)
    find_package(Boost REQUIRED)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(netcore PUBLIC OpenSSL::SSL OpenSSL::Crypto)
endif()

if (NETCORE_WITH_CURL)
    find_package(CURL REQUIRED)
    target_link_libraries(netcore PUBLIC CURL::libcurl)
endif()

if (UNIX AND NOT APPLE) 
    target_link_libraries(netcore PUBLIC pthread)
endif()

target_compile_definitions(netcore PUBLIC
    NETCORE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    NETCORE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    NETCORE_VERSION_PATCH=${PROJECT_VERSION_PATCH}    
)

include(GNUInstallDirs)
install(TARGETS netcore
    EXPORT netcoreTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT netcoreTargets
    FILE netcoreTargets.cmake
    NAMESPACE NetCore::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/netcore
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/netcoreConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/netcoreConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/netcore
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/netcoreConfig.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/netcore)

if (NETCORE_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
if (NETCORE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()