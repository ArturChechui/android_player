cmake_minimum_required(VERSION 3.22.1)

project("player")

add_library(
        player
        SHARED
        JniBridge.cpp
        AudioEngine.cpp
        HttpDataSource.cpp
        AudioDecoder.cpp
)

target_link_libraries(
        player
        android
        log
        aaudio
        mediandk
)
