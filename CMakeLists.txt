# mklink /D "C:\Program Files\Common Files\VST3\generativedelay20-debug.vst3" "C:\Users\savas\Documents\JUCE Projects\generativedelay20\cmake_build\generativedelay20_artefacts\Debug\VST3\generativedelay20.vst3"
# command: C:\Program Files (x86)\Image-Line\FL Studio 12\FL64.exe
# 
# 
cmake_minimum_required(VERSION 3.15)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for macOS")
endif()

project(generativedelay20 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# Build configuration options
option(DEMO_BUILD "Build demo version" OFF)

# JUCE path
if(APPLE)
    set(JUCE_PATH "/Users/admin/Documents/repos/JUCE")
elseif(NOT APPLE)
    set(JUCE_PATH "C:/Program Files/JUCE")
endif()

# Include JUCE
add_subdirectory(${JUCE_PATH} JUCE)

# Conditional formats
if(APPLE)
    set(PLUGIN_FORMATS AU VST3)
elseif(NOT APPLE)
    set(PLUGIN_FORMATS VST3)
endif()

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
    add_compile_options(
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:Release>:/MT>
    )
endif()

# Product name
set(PLUGIN_PRODUCT_NAME "generativedelay20")

# Define user library path for plugin installation (macOS-specific)
if(APPLE)
    set(USER_LIBRARY_DIR "$ENV{HOME}/Library/Audio/Plug-Ins")
endif()

# Plugin setup arguments (common)
set(PLUGIN_ARGS
    PLUGIN_MANUFACTURER_CODE "Manu"
    PLUGIN_CODE "CtnE"
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "${PLUGIN_PRODUCT_NAME}"
    COMPANY_NAME "54v450l4r"
    BUNDLE_ID "com.54v450l4r.generativedelay20"
    PLUGIN_IS_A_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    MIDI_INPUT_NAME "MIDI Input"
    MIDI_OUTPUT_NAME "MIDI Output"
)

# Conditional plugin setup arguments
if(APPLE)
    list(APPEND PLUGIN_ARGS
        COPY_PLUGIN_AFTER_BUILD TRUE
        AU_COPY_DIR "${USER_LIBRARY_DIR}/Components"
        VST3_COPY_DIR "${USER_LIBRARY_DIR}/VST3"
        VST3_COPY_DIR_DISABLE_AUTO_COPY TRUE
    )
else()
    list(APPEND PLUGIN_ARGS
        COPY_PLUGIN_AFTER_BUILD FALSE
    )
endif()

# Plugin setup
juce_add_plugin(generativedelay20 ${PLUGIN_ARGS})

target_compile_definitions(generativedelay20
    PRIVATE
        JUCE_DISABLE_VST2_BUILDING=1
        JUCE_PLUGINHOST_VST3=1
        JUCE_PLUGINHOST_VST=0
        $<$<BOOL:${DEMO_BUILD}>:DEMO_BUILD>
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
)

target_include_directories(generativedelay20 PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/adamski/audio_fft
#     ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/adamski/audio_fft/AudioFFT
#     ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/adamski/pitch_detector
#     ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/adamski/pitch_detector/source
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/signalsmith/stretch
    ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/
#     ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/adamski
	${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/dywapitchtrack/
	${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/dywapitchtrack/src
)

# if(APPLE)
#     target_include_directories(generativedelay20 PRIVATE
#         /Users/admin/Documents/repos/audio_fft
#         /Users/admin/Documents/repos/audio_fft/AudioFFT
#         /Users/admin/Documents/repos/pitch_detector
#         /Users/admin/Documents/repos/pitch_detector/source
#         /Users/admin/Documents/repos
#         /Users/admin/Documents/repos/stretch
#     )
# elseif(NOT APPLE)
#     target_include_directories(generativedelay20 PRIVATE
#         C:/repos/adamski/audio_fft
#         C:/repos/adamski/audio_fft/AudioFFT
#         C:/repos/adamski/pitch_detector
#         C:/repos/adamski/pitch_detector/source
#         C:/repos/adamski
#         C:/repos/stretch
#     )
# endif()


# Source files


target_sources(generativedelay20 PRIVATE
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
#     Dependencies/adamski/audio_fft/audio_fft.cpp
#     Dependencies/adamski/audio_fft/audio_fft.mm
#     Dependencies/adamski/pitch_detector/pitch_detector.cpp
#     Dependencies/adamski/pitch_detector/pitch_detector.mm
	Dependencies/dywapitchtrack/src/dywapitchtrack.c
)


# Binary data
juce_add_binary_data(BinaryResources SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Graphics/B612Mono-Regular.ttf
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Graphics/cap-mel-texture.png
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Graphics/gen-mel-texture.png
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Graphics/overlap-mel-texture.png
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Graphics/preset-menu.png
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Graphics/preset-menu-hover.png
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Graphics/ui-body.png
)




# # include onnxruntime static
# if(APPLE)
#     set(ONNXRUNTIME_DIR "/Users/admin/Documents/repos/onnxruntime-static")
# elseif(NOT APPLE)
#     set(ONNXRUNTIME_DIR "C:/repos/onnxruntime-static-debug")
# #     set(ONNXRUNTIME_DIR "C:/repos/onnxruntime-static")
# endif()


# target_include_directories(generativedelay20 PRIVATE ${ONNXRUNTIME_DIR}/include)

# if(APPLE)
#     target_link_libraries(generativedelay20 PRIVATE "${ONNXRUNTIME_DIR}/lib/libonnxruntime.a")
# elseif(NOT APPLE)
#     target_link_libraries(generativedelay20 PRIVATE "${ONNXRUNTIME_DIR}/lib/onnxruntime.lib")
# endif()

# Link everything
target_link_libraries(generativedelay20 PRIVATE
    BinaryResources
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_osc
)

juce_generate_juce_header(generativedelay20)

