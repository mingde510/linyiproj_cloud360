cmake_minimum_required(VERSION 3.10)
project(lidar_bg_subtract_qt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(PCL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

# ---- Unitree SDK 路径（按实际修改）---
include_directories(
    ${PCL_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ../../include
    ../person_tr
)

set(UNITREE_SDK_LIB_DIR
    ../../lib/x86_64
)

find_library(UNITREE_SDK_LIB
    NAMES unilidar_sdk2
    PATHS ${UNITREE_SDK_LIB_DIR}
    NO_DEFAULT_PATH
)


set(TENSORRT_ROOT "/usr/lib/x86_64-linux-gnu" CACHE PATH "TensorRT root")
set(CUDA_ROOT "/usr/local/cuda" CACHE PATH "CUDA root")

include_directories(
    ${CUDA_ROOT}/include
    /usr/include/x86_64-linux-gnu
)

link_directories(
    ${PCL_LIBRARY_DIRS}
    ${TENSORRT_ROOT}
    ${CUDA_ROOT}/lib64
    ../lib/x86_64
)

find_package(VTK REQUIRED COMPONENTS
  vtkCommonCore
  vtkRenderingCore
  vtkRenderingOpenGL2
  vtkGUISupportQt
)

include(${VTK_USE_FILE})

add_definitions(${PCL_DEFINITIONS})

set(SOURCES
    main.cpp
    core/LidarProcessor.cpp
    core/SessionManager.cpp
    core/BackgroundModel.cpp
    core/UnitreePclBridge.cpp
    core/RoiAnalyzer.cpp
    core/VisualizerHelper.cpp
    ui/MainWindow.cpp
    ../person_tr/PersonDetectorApp.cpp
    ../person_tr/RtspCapture.cpp
    ../person_tr/RoiManager.cpp
    ../person_tr/TrtYoloDetector.cpp
    ../person_tr/Visualizer.cpp
)

set(HEADERS
    core/LidarProcessor.h
    core/SessionManager.h
    core/BackgroundModel.h
    core/UnitreePclBridge.h
    core/UnitreePointType.h
    core/RoiAnalyzer.h
    core/VisualizerHelper.h
    ui/MainWindow.h
    ../person_tr/PersonDetectorApp.h
    ../person_tr/RtspCapture.hpp
    ../person_tr/RoiManager.hpp
    ../person_tr/TrtYoloDetector.hpp
    ../person_tr/Visualizer.hpp
)

set(UIS
    ui/MainWindow.ui
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UIS}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE PCL_NO_PRECOMPILE)

target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    ${PCL_LIBRARIES}
    ${UNITREE_SDK_LIB}          # 按实际库名（可能不同）
    ${OpenCV_LIBS}
    ${VTK_LIBRARIES}
    nvinfer
    nvinfer_plugin
    nvonnxparser
    cudart
    Threads::Threads
)
