cmake_minimum_required(VERSION 3.5.0)
project(vntc)

set(CMAKE_CXX_STANDARD 17)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${NATIVERENDER_ROOT_PATH}/include)

if(NOT DEFINED OHOS_ARCH OR OHOS_ARCH STREQUAL "")
  set(OHOS_ARCH arm64-v8a)
endif()

add_library(vnt_ffi SHARED IMPORTED)
set_target_properties(vnt_ffi PROPERTIES
  IMPORTED_LOCATION ${NATIVERENDER_ROOT_PATH}/../../../libs/${OHOS_ARCH}/libvnt_ffi.so
  IMPORTED_NO_SONAME TRUE)

add_library(vnt_napi SHARED napi_init.cpp)
target_link_libraries(vnt_napi PUBLIC libace_napi.z.so libhilog_ndk.z.so vnt_ffi)
