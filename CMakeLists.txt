cmake_minimum_required(VERSION 3.16)
project(calib_gui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Qt6 ---
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(Qt6 COMPONENTS Charts QUIET)  # Charts opsiyonel (ResultsWindow fallback var)

# --- OpenCV ---
find_package(OpenCV REQUIRED)

# --- Sources ---
add_executable(calib_gui
    src/main.cpp
    src/MainWindow.cpp src/MainWindow.h
    src/SettingsWindow.cpp src/SettingsWindow.h
    src/ResultsWindow.cpp src/ResultsWindow.h
    src/PreviewPane.cpp src/PreviewPane.h
    src/CaptureGallery.cpp src/CaptureGallery.h
    src/CalibrationController.cpp src/CalibrationController.h
    src/Utils.cpp src/Utils.h
    src/DeviceManager.cpp src/DeviceManager.h
    src/PatternSettingsWidget.cpp src/PatternSettingsWidget.h
    src/CornerSettingsWidget.cpp src/CornerSettingsWidget.h
)

# --- Link libraries ---
target_link_libraries(calib_gui
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${OpenCV_LIBS}
)

if (Qt6Charts_FOUND)
    target_link_libraries(calib_gui Qt6::Charts)
    target_compile_definitions(calib_gui PRIVATE HAVE_QTCHARTS=1)
endif()

# --- Varsayılan Release build ---
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
