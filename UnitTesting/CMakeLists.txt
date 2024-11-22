# CMakeList.txt : CMake project for UnitTesting, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("UnitTesting")

# TODO: Add tests and install targets if needed.

# Enable testing
enable_testing()

# Option 1: Use FetchContent to download Catch2 automatically
Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

# Path to RAC project source code
set(RAC_PROJECT_INCLUDE_DIR "C:/Documents/GitHub/jmannall/RoomAcoustiCpp/RoomAcoustiCpp/include")
set(RAC_PROJECT_SOURCE_DIR "C:/Documents/GitHub/jmannall/RoomAcoustiCpp/RoomAcoustiCpp/source")

include_directories(${CMAKE_SOURCE_DIR}/external/catch2)
#include_directories(${RAC_PROJECT_INCLUDE_DIR})

include_directories("C:/Documents/GitHub/jmannall/RoomAcoustiCpp/3dti_AudioToolkit/3dti_Toolkit")
include_directories("C:/Documents/GitHub/jmannall/RoomAcoustiCpp/RoomAcoustiCpp/codegen/lib/myNN/include")
include_directories("C:/Documents/GitHub/jmannall/RoomAcoustiCpp/3dti_AudioToolkit/3dti_ResourceManager/third_party_libraries/boost_circular_buffer")
include_directories("C:/Documents/GitHub/jmannall/RoomAcoustiCpp/3dti_AudioToolkit/3dti_ResourceManager")
include_directories("C:/Documents/GitHub/jmannall/RoomAcoustiCpp/3dti_AudioToolkit/3dti_ResourceManager/third_party_libraries/cereal/include")

file(GLOB_RECURSE RAC_PROJECT_SOURCE_FILES
    ${RAC_PROJECT_SOURCE_DIR}/*.cpp
)

# Source files
#set(SRC_FILES
#    ${RAC_PROJECT_SOURCE_DIR}/DSP/GraphicEQ.cpp
#)

# Test files
set(TEST_FILES
    Test_main.cpp
    GraphicEQTests.cpp
)

# Add executable for tests
#add_executable(UnitTesting ${RAC_PROJECT_SOURCE_FILES} ${TEST_FILES})
add_executable(UnitTesting ${TEST_FILES})

# Link Catch2 with your test executable
target_link_libraries(UnitTesting PRIVATE Catch2::Catch2WithMain)
# target_link_libraries(UnitTesting Catch2::Catch2)

add_library(RoomAcoustiCpp STATIC ${RAC_PROJECT_SOURCE_FILES})
target_include_directories(RoomAcoustiCpp PUBLIC ${RAC_PROJECT_INCLUDE_DIR})
target_compile_features(RoomAcoustiCpp PUBLIC cxx_std_17)
target_link_libraries(UnitTesting PRIVATE RoomAcoustiCpp)

# Enable C++17 (required by Catch2 v3)
target_compile_features(UnitTesting PRIVATE cxx_std_17)

# Register the tests with CTest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(UnitTesting)