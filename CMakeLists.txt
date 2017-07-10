cmake_minimum_required(VERSION 3.7)
project(MediaServer)

include_directories(include)
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)
set(CMAKE_CXX_STANDARD 14)

FIND_PACKAGE(FRNETLIB)
INCLUDE_DIRECTORIES(${FRNETLIB_INCLUDE_DIRS})

FIND_PACKAGE(FFMPEG)
INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIRS})

FIND_PACKAGE(AO)
INCLUDE_DIRECTORIES(${AO_INCLUDE_DIRS})

FIND_PACKAGE(ALSA)
INCLUDE_DIRECTORIES(${ALSA_INCLUDE_DIRS})


set(SOURCE_FILES main.cpp src/MusicPlayer.cpp include/MusicPlayer.h src/APIServer.cpp include/APIServer.h include/Log.h src/Filesystem.cpp include/Filesystem.h src/MusicStorage.cpp include/MusicStorage.h src/ALSAController.cpp include/ALSAController.h src/Playlist.cpp include/Playlist.h src/InternetRadio.cpp include/InternetRadio.h src/StreamDecoderBase.cpp include/StreamDecoderBase.h src/M3UStreamDecoder.cpp include/M3UStreamDecoder.h src/DirectStreamDecoder.cpp include/DirectStreamDecoder.h)
add_executable(MediaServer ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(MediaServer ${FRNETLIB_LIBRARIES} -pthread -static-libstdc++ -lasound)
TARGET_LINK_LIBRARIES(MediaServer ${FFMPEG_LIBRARIES})
TARGET_LINK_LIBRARIES(MediaServer ${AO_LIBRARIES})
TARGET_LINK_LIBRARIES(MediaServer ${ALSA_LIBRARIES})
