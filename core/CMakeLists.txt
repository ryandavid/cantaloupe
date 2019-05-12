pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

include_directories(
    include
    ${SPDLOG_INCLUDE_DIRS}
)

include_directories(SYSTEM ${LIBUSB_INCLUDE_DIRS})

link_directories(${LIBUSB_LIBRARY_DIRS})

add_executable(cantaloupe
    src/cantaloupe.cpp
    src/log.cpp
    src/usb_wrapper.cpp
)

target_link_libraries(cantaloupe
    ${LIBUSB_LIBRARIES}
)
