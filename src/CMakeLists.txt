add_library(NaiveRainbow rainbow.c)
find_package(OpenSSL REQUIRED)
target_include_directories(NaiveRainbow PUBLIC ../include)
target_link_libraries(NaiveRainbow OpenSSL::Crypto)

IF (NOT WIN32)
  target_link_libraries(NaiveRainbow m)
ENDIF()
