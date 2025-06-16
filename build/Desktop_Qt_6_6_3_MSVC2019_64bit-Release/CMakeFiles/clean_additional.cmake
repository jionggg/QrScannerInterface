# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\ZeissApp1_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ZeissApp1_autogen.dir\\ParseCache.txt"
  "ZeissApp1_autogen"
  )
endif()
