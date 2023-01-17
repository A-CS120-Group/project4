cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 17)

set(NODE1 "Node1")
set(NODE2 "Node2")
project(${NODE1} VERSION 0.0.1)
project(${NODE2} VERSION 0.0.1)
add_subdirectory(JUCE)
file(GLOB INCLUDE_FILE include/*.h include/*.cpp)

macro(setup name)
    juce_add_gui_app(${name} PRODUCT_NAME ${name})
    juce_generate_juce_header(${name})
    target_link_libraries(${name}
            PRIVATE
            juce::juce_analytics
            juce::juce_audio_basics
            juce::juce_audio_devices
            juce::juce_audio_formats
            juce::juce_audio_plugin_client
            juce::juce_audio_processors
            juce::juce_audio_utils
            juce::juce_box2d
            juce::juce_core
            juce::juce_cryptography
            juce::juce_data_structures
            juce::juce_dsp
            juce::juce_events
            juce::juce_graphics
            juce::juce_gui_basics
            juce::juce_gui_extra
            juce::juce_opengl
            juce::juce_osc
            juce::juce_product_unlocking
            juce::juce_video
            PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)
    target_sources(${name}
            PRIVATE
            ${INCLUDE_FILE}
            )
endmacro(setup)

setup(${NODE1})
setup(${NODE2})
target_include_directories(${NODE1} PRIVATE Node1)
target_include_directories(${NODE2} PRIVATE Node2)

find_package(Boost COMPONENTS filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
