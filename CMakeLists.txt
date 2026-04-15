cmake_minimum_required (VERSION 3.2)

project (NetworkToolsLib)

option(WITH_CURL_UTILITIES "Enable libcurl utilities" OFF)

set(SOURCES
   src/AbstractServer.cpp include/AbstractServer.h
   src/AbstractClient.cpp include/AbstractClient.h
   src/PosixTcpServer.cpp include/PosixTcpServer.h
   src/PosixTcpClient.cpp include/PosixTcpClient.h
   src/emailaccountdata.cpp include/emailaccountdata.h
   src/emaildata.cpp include/emaildata.h
   src/mimetools.cpp include/mimetools.h
   src/networkwaker.cpp include/networkwaker.h
)

if(WITH_CURL_UTILITIES)
   list(APPEND SOURCES
      src/curllibsender.cpp
      src/curllibsender.h
   )
endif()

add_library(NetworkToolsLib ${SOURCES})
target_include_directories(NetworkToolsLib PUBLIC "include")
target_link_libraries (NetworkToolsLib BasicToolsLib)

