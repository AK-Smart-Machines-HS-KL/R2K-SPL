# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\Controller_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Controller_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\SimRobotCore2D_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\SimRobotCore2D_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\SimRobotCore2_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\SimRobotCore2_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\SimRobotEditor_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\SimRobotEditor_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\SimRobot_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\SimRobot_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\bush_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\bush_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\qtpropertybrowser_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\qtpropertybrowser_autogen.dir\\ParseCache.txt"
  "Controller_autogen"
  "SimRobotCore2D_autogen"
  "SimRobotCore2_autogen"
  "SimRobotEditor_autogen"
  "SimRobot_autogen"
  "bush_autogen"
  "qtpropertybrowser_autogen"
  )
endif()
