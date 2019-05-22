pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

# Explicitly set LibUSB's include directory as a system header so clang doesn't bitch about stuff we can't fix.
include_directories(SYSTEM ${LIBUSB_INCLUDE_DIRS})
link_directories(${LIBUSB_LIBRARY_DIRS})

include_directories(
    include
    ${SPDLOG_INCLUDE_DIRS}
)

# Core canataloupe lib.
add_library(cantaloupe SHARED
    src/gs_usb_wrapper.cpp
    src/log.cpp
)

# Provide the LibUSB version into the lib as a string.  This is to sidestep us from having to assemble it at runtime
# ourselves.
target_compile_definitions(cantaloupe PRIVATE LIBUSB_VERSION_STRING="${LIBUSB_VERSION}")

target_link_libraries(cantaloupe
    ${LIBUSB_LIBRARIES}
)

# Create a test program.
add_executable(test_cantaloupe
    src/test_cantaloupe.cpp
)

target_link_libraries(test_cantaloupe
    cantaloupe
)


