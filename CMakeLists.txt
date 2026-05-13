cmake_minimum_required(VERSION 3.16)

project(RoomAcoustiCPP LANGUAGES CXX)


#
# 3DTI
#

# Generate a list of all source files in the 3dti_AudioToolkit directory and save it in TDTI_SOURCES variable
file(GLOB_RECURSE TDTI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/3dti_AudioToolkit/3dti_Toolkit/BinauralSpatializer/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/3dti_AudioToolkit/3dti_Toolkit/Common/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/3dti_AudioToolkit/3dti_Toolkit/HAHLSimulation/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/3dti_AudioToolkit/3dti_Toolkit/ISM/*.cpp"
)

# Add the 3DTI library based on the source files defined in TDTI_SOURCES variable
add_library(3DTI STATIC ${TDTI_SOURCES})

# Include directories for 3DTI library to allow other targets to find the necessary header files
target_include_directories(3DTI
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/3dti_AudioToolkit"
        "${CMAKE_CURRENT_SOURCE_DIR}/3dti_AudioToolkit/3dti_Toolkit"
        "${CMAKE_CURRENT_SOURCE_DIR}/3dti_AudioToolkit/3dti_ResourceManager"
        "${CMAKE_CURRENT_SOURCE_DIR}/3dti_AudioToolkit/3dti_ResourceManager/third_party_libraries/boost_circular_buffer"
        "${CMAKE_CURRENT_SOURCE_DIR}/3dti_AudioToolkit/3dti_ResourceManager/third_party_libraries/cereal/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/3dti_AudioToolkit/3dti_ResourceManager/third_party_libraries/eigen"
)

target_compile_features(3DTI PUBLIC cxx_std_20)

#
# RoomAcoustiCPP
#

# Generate a list of all source files in the RoomAcoustiCpp directory and save it in RAC_SOURCES variable
file(GLOB_RECURSE RAC_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/RoomAcoustiCpp/source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RoomAcoustiCpp/codegen/dll/myNN/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/RoomAcoustiCpp/codegen/lib/myNN/source/*.cpp"
)

# Add the RoomAcoustiCPP library based on the source files defined in RAC_SOURCES variable
add_library(RoomAcoustiCPP STATIC ${RAC_SOURCES})

# Include Matlab libraries and include directories for RoomAcoustiCPP target
find_package(Matlab REQUIRED COMPONENTS MX_LIBRARY ENG_LIBRARY)

target_link_libraries(RoomAcoustiCPP PRIVATE
    Matlab::mx
    Matlab::eng
)

target_include_directories(RoomAcoustiCPP PRIVATE
    ${Matlab_INCLUDE_DIRS}
)

target_link_libraries(RoomAcoustiCPP
    PUBLIC
        3DTI
)

target_include_directories(RoomAcoustiCPP
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/RoomAcoustiCpp/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/RoomAcoustiCpp/include/Spatialiser"
        "${CMAKE_CURRENT_SOURCE_DIR}/RoomAcoustiCpp/codegen/dll/myNN"
        "${CMAKE_CURRENT_SOURCE_DIR}/RoomAcoustiCpp/codegen/lib/myNN/include"
)

target_compile_features(RoomAcoustiCPP PUBLIC cxx_std_20)

if(WIN32)
    target_compile_definitions(RoomAcoustiCPP
        PRIVATE
            RPC_NO_WINDOWS_H
            WIN32_LEAN_AND_MEAN
    )
endif()

