cmake_minimum_required(VERSION 3.20)
project(AirQualityProject VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Charts
)

# nlohmann/json (header-only, z external/)
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/nlohmann
)

set(SOURCES
    src/main.cpp

    # Core models
    src/core/Station.cpp
    src/core/Sensor.cpp
    src/core/Measurement.cpp
    src/core/AirQualityIndex.cpp

    # Storage
    src/storage/DatabaseManager.cpp

    # API
    src/api/GiosClient.cpp
    src/api/AsyncFetcher.cpp

    # Analysis
    src/analysis/AirAnalyzer.cpp
    src/analysis/LinearTrendStrategy.cpp
    src/analysis/MovingAverageTrendStrategy.cpp
    src/analysis/GeoLocator.cpp

    # GUI
    src/gui/MainWindow.cpp
    src/gui/ChartWidget.cpp
    src/gui/StationListWidget.cpp
    src/gui/DateRangeSelector.cpp
    src/gui/RadiusSearchDialog.cpp
    src/gui/SensorSelectorWidget.cpp
    src/gui/BatchDownloader.cpp

    # Utils
    src/utils/Logger.cpp
    src/utils/AppExceptions.cpp
)

set(HEADERS
    include/core/Station.hpp
    include/core/Sensor.hpp
    include/core/Measurement.hpp
    include/core/AirQualityIndex.hpp

    include/storage/DatabaseManager.hpp

    include/api/GiosClient.hpp
    include/api/AsyncFetcher.hpp

    include/analysis/AirAnalyzer.hpp
    include/analysis/ITrendStrategy.hpp
    include/analysis/LinearTrendStrategy.hpp
    include/analysis/MovingAverageTrendStrategy.hpp
    include/analysis/GeoLocator.hpp

    include/gui/MainWindow.hpp
    include/gui/ChartWidget.hpp
    include/gui/StationListWidget.hpp
    include/gui/DateRangeSelector.hpp
    include/gui/RadiusSearchDialog.hpp
    include/gui/SensorSelectorWidget.hpp
    include/gui/BatchDownloader.hpp

    include/utils/Logger.hpp
    include/utils/AppExceptions.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
)

# Google Test - pobierany automatycznie przez FetchContent
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(AirQualityTests
    tests/test_AirAnalyzer.cpp
    tests/test_DatabaseManager.cpp
    tests/test_GeoLocator.cpp
    tests/test_AirQualityIndex.cpp
    tests/test_Sensor.cpp
    src/core/Station.cpp
    src/core/Sensor.cpp
    src/core/Measurement.cpp
    src/core/AirQualityIndex.cpp
    src/storage/DatabaseManager.cpp
    src/analysis/AirAnalyzer.cpp
    src/analysis/LinearTrendStrategy.cpp
    src/analysis/MovingAverageTrendStrategy.cpp
    src/analysis/GeoLocator.cpp
    src/utils/Logger.cpp
    src/utils/AppExceptions.cpp
)
target_link_libraries(AirQualityTests gtest gtest_main Qt6::Core Qt6::Network)
add_test(NAME AirQualityTests COMMAND AirQualityTests)