cmake_minimum_required(VERSION 3.5)
project(g29_can)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

add_executable(g29_can src/g29cansend.cpp)
ament_target_dependencies(g29_can rclcpp std_msgs sensor_msgs can_msgs)

install(TARGETS
  g29_can
  DESTINATION lib/${PROJECT_NAME})

ament_package()
