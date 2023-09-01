cmake_minimum_required(VERSION 3.15)
project(ModernRTAudioProject VERSION 1.0.0)
include(cmake/CXXStandard.cmake)

set(CMAKE_CXX_STANDARD 17)

# Include modules
add_subdirectory(modules/JUCE)
add_subdirectory(RTNeural)
include_directories(
    RTNeural
    dsp
)

# Set RT Settings
include(cmake/ChooseBackend.cmake)

# Set JUCE Settings
juce_add_plugin(ModernRTAudioProject
    PLUGIN_MANUFACTURER_CODE MRTA
    PLUGIN_CODE T3ST
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "ModernRTAudioProject"
    COPY_PLUGIN_AFTER_BUILD TRUE
)
juce_generate_juce_header(ModernRTAudioProject)
juce_add_module(modules/mrta_utils)

# Add sources
target_sources(ModernRTAudioProject PRIVATE
    source/Plugin.cpp
    source/PluginEditor.cpp
    dsp/DelayLine.cpp
    dsp/LFODelayLine.cpp
)

# Add binaries
juce_add_binary_data(BinaryData SOURCES
    weights/neural_net_weights.json
    weights/gru_torch.json
    weights/gru_torch_chowtape.json
)
set_target_properties(BinaryData PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)

# Optionally build ./projects
option(BUILD_PROJECTS "Build projects" OFF)
if(BUILD_PROJECTS)
    message(STATUS "Configuring projects...")
    add_subdirectory(projects)
endif()

# Compile definitions
target_compile_definitions(ModernRTAudioProject
    PUBLIC
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    RTNEURAL_DEFAULT_ALIGNMENT=16
)

# Link libraries
target_link_libraries(ModernRTAudioProject
    PUBLIC
    BinaryData
    juce::juce_audio_utils
    juce::juce_dsp
    RTNeural
    mrta_utils
)
