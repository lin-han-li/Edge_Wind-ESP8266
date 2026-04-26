# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "D:/Espressif/frameworks/esp-idf-v5.5.4/components/bootloader/subproject")
  file(MAKE_DIRECTORY "D:/Espressif/frameworks/esp-idf-v5.5.4/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "C:/Users/pengjianzhong/Desktop/MY_Project/ESP32/Edge_Wind_System+ESP32+STM32/esp32_spi_coprocessor/build_codex_probe/bootloader"
  "C:/Users/pengjianzhong/Desktop/MY_Project/ESP32/Edge_Wind_System+ESP32+STM32/esp32_spi_coprocessor/build_codex_probe/bootloader-prefix"
  "C:/Users/pengjianzhong/Desktop/MY_Project/ESP32/Edge_Wind_System+ESP32+STM32/esp32_spi_coprocessor/build_codex_probe/bootloader-prefix/tmp"
  "C:/Users/pengjianzhong/Desktop/MY_Project/ESP32/Edge_Wind_System+ESP32+STM32/esp32_spi_coprocessor/build_codex_probe/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/pengjianzhong/Desktop/MY_Project/ESP32/Edge_Wind_System+ESP32+STM32/esp32_spi_coprocessor/build_codex_probe/bootloader-prefix/src"
  "C:/Users/pengjianzhong/Desktop/MY_Project/ESP32/Edge_Wind_System+ESP32+STM32/esp32_spi_coprocessor/build_codex_probe/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/pengjianzhong/Desktop/MY_Project/ESP32/Edge_Wind_System+ESP32+STM32/esp32_spi_coprocessor/build_codex_probe/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/pengjianzhong/Desktop/MY_Project/ESP32/Edge_Wind_System+ESP32+STM32/esp32_spi_coprocessor/build_codex_probe/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
