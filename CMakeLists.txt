cmake_minimum_required(VERSION 3.10)
project(lvgl)
set(CMAKE_C_COMPILER /home/wang/tina-linux/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-musl/toolchain/bin/arm-openwrt-linux-gcc)#C99 # lvgl officially support C99 and above
set(CMAKE_CXX_COMPILER /home/wang/tina-linux/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-musl/toolchain/bin/arm-openwrt-linux-g++)#C17
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
message(STATUS "LV_CONF_BUILD_DISABLE_EXAMPLES ${CMAKE_C_COMPILER}")
option(LV_USE_DRAW_SDL "Use SDL draw unit" OFF)
option(LV_USE_LIBPNG "Use libpng to decode PNG" OFF)
option(LV_USE_LIBJPEG_TURBO "Use libjpeg turbo to decode JPEG" OFF)
option(LV_USE_FFMPEG "Use libffmpeg to display video using lv_ffmpeg" OFF)
option(LV_USE_FREETYPE "Use freetype lib" ON)

option(LV_CONF_BUILD_DISABLE_EXAMPLES "LV_CONF_BUILD_DISABLE_EXAMPLES" OFF)
option(LV_CONF_BUILD_DISABLE_DEMOS "LV_CONF_BUILD_DISABLE_DEMOS" OFF)

#set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#find_package(SDL2 REQUIRED SDL2)

add_compile_options(-g3)
add_compile_definitions(__USE_GNU)
#add_compile_definitions($<$<BOOL:${LV_USE_DRAW_SDL}>:LV_USE_DRAW_SDL=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBPNG}>:LV_USE_LIBPNG=1>)
add_compile_definitions($<$<BOOL:${LV_USE_LIBJPEG_TURBO}>:LV_USE_LIBJPEG_TURBO=1>)
add_compile_definitions($<$<BOOL:${LV_USE_FFMPEG}>:LV_USE_FFMPEG=1>)
include_directories(/home/wang/tina-linux/out/t113-mq_r/compile_dir/target/freetype-2.9.1/include)
add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR} ${SDL2_INCLUDE_DIRS} inc
/home/wang/tina-linux/app/paho.mqtt.c/src
/home/wang/tina-linux/out/t113-mq_r/compile_dir/target/curl-7.54.1/ipkg-install/usr/include
)


# /usr/include/freetype2
# set(CMAKE_CXX_FLAGS -lfreetype)
add_executable(main main.c mouse_cursor_icon.c 
./src/net.c 
./src/gui.c 
./src/cJSON.c 
./src/mqtt.c 
./src/stock.c 
./src/gui_dev.c 
./src/gui_predict.c 
./src/gui_main.c 
./src/gui_stock.c 
./src/linux_stat.c 
./src/weather.c 
./src/gui_weather.c 
./src/gui_linux.c
./src/gui_weather_station.c
./src/weather_station.c
./src/thread_base.c 
)

target_compile_definitions(main PRIVATE LV_CONF_INCLUDE_SIMPLE)
#target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${SDL2_LIBRARIES} m pthread)
target_link_libraries(main lvgl  lvgl::thorvg m pthread)
add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/main DEPENDS main)

# 使用X11
#find_package(X11 REQUIRED)
#target_include_directories(lvgl PUBLIC ${X11_INCLUDE_DIR})
#target_link_libraries(main ${X11_LIBRARIES})


if(LV_USE_LIBPNG)
    find_package(PNG REQUIRED)
    target_include_directories(lvgl PUBLIC ${PNG_INCLUDE_DIR})
    target_link_libraries(main ${PNG_LIBRARY})
endif(LV_USE_LIBPNG)

#if(LV_USE_FREETYPE)
if (OFF)
    find_package(Freetype REQUIRED)
    target_link_libraries(main ${FREETYPE_LIBRARIES})
    target_include_directories(lvgl PUBLIC ${FREETYPE_INCLUDE_DIRS})
endif(LV_USE_FREETYPE)

#find_package(eclipse-paho-mqtt-c REQUIRED)
#target_link_libraries(main eclipse-paho-mqtt-c::paho-mqtt3c
target_link_libraries(main /home/wang/tina-linux/app/paho.mqtt.c/build/src/libpaho-mqtt3c.so.1.3.13 
/home/wang/tina-linux/out/t113-mq_r/compile_dir/target/zlib-1.2.8/libz.so.1.2.8 
/home/wang/tina-linux/out/t113-mq_r/compile_dir/target/bzip2-1.0.6/libbz2.so.1.0.6
/home/wang/tina-linux/out/t113-mq_r/compile_dir/target/curl-7.54.1/ipkg-install/usr/lib/libcurl.so.4.4.0
/home/wang/tina-linux/out/t113-mq_r/compile_dir/target/nghttp2-1.24.0/ipkg-install/usr/lib/libnghttp2.so.14.13.3
/home/wang/tina-linux/out/t113-mq_r/compile_dir/target/freetype-2.9.1/ipkg-install/usr/lib/libfreetype.so)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(lvgl PRIVATE
    src PRIVATE
        -pedantic-errors
        -Wall
        -Wclobbered
        -Wdeprecated
        -Wdouble-promotion
        -Wempty-body
        -Wextra
        -Wformat-security
        -Wmaybe-uninitialized
        # -Wmissing-prototypes
        -Wpointer-arith
        -Wmultichar
        -Wno-pedantic # ignored for now, we convert functions to pointers for propertis table.
        -Wreturn-type
        -Wshadow
        -Wshift-negative-value
        -Wsizeof-pointer-memaccess
        -Wtype-limits
        -Wundef
        -Wuninitialized
        -Wunreachable-code
        -Wfloat-conversion
        -Wstrict-aliasing
    )

    target_compile_options(main PRIVATE -fsanitize=address,leak,undefined)
    target_link_options(main PRIVATE -fsanitize=address,leak,undefined)
endif()
message(STATUS "MAKE_CXX_FLAGS -lfreetype ${CMAKE_C_COMPILER}")