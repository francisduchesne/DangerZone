cmake_minimum_required(VERSION 3.22)

# Danger Zone — Arcade-branded LinnDrum-style instrument (VST3 + Standalone).
# Not an FVS_Host module. FVS names in the mixer are stub insert slots only.

project(DangerZone VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(cmake/FetchJUCE.cmake)

juce_add_plugin(DangerZone
    COMPANY_NAME "Arcades"
    COMPANY_WEBSITE "https://github.com/francisduchesne/DangerZone"
    BUNDLE_ID com.arcades.dangerzone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Arcd
    PLUGIN_CODE Dzln
    FORMATS VST3 Standalone
    PRODUCT_NAME "Danger Zone"
    PLUGIN_NAME "Danger Zone"
    DESCRIPTION "Arcade-branded LinnDrum-style drum instrument"
    VST3_CATEGORIES Instrument Drum
)

juce_generate_juce_header(DangerZone)

target_sources(DangerZone
    PRIVATE
        Source/Parameters.cpp
        Source/Sequencer.cpp
        Source/VoiceEngine.cpp
        Source/Mixer.cpp
        Source/FvsInsert.cpp
        Source/FxChain.cpp
        Source/PresetManager.cpp
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Ui/PadPage.cpp
        Source/Ui/SequencerPage.cpp
        Source/Ui/MixerPage.cpp
        Source/Ui/FxPage.cpp
        Source/Ui/PresetsPage.cpp
)

target_include_directories(DangerZone
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Ui
)

target_compile_definitions(DangerZone
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        DZ_PLUGIN_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(DangerZone
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

if (MSVC)
    target_compile_definitions(DangerZone PRIVATE _USE_MATH_DEFINES)
    target_compile_options(DangerZone PRIVATE /bigobj /MP /utf-8 /wd4244 /wd4267 /wd4100)
else()
    target_compile_options(DangerZone PRIVATE
        -Wall
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-unused-function
        -Wno-overloaded-virtual
        -Wno-sign-compare
    )
endif()

# Placeholder multi-samples travel with the artefact so Standalone and VST3
# load Resources/Samples without a hard-coded path. Real Linn takes replace
# these files in place (same folder and *_vN.wav names).
function(dz_stage_samples target_name)
    if (NOT TARGET ${target_name})
        return()
    endif()
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Resources/Samples"
            "$<TARGET_FILE_DIR:${target_name}>/Resources/Samples"
        COMMENT "Stage Danger Zone samples next to ${target_name}"
        VERBATIM)
endfunction()

dz_stage_samples(DangerZone_VST3)
dz_stage_samples(DangerZone_Standalone)

message(STATUS "Danger Zone ${PROJECT_VERSION}: Arcades instrument (VST3 + Standalone), no FVS_Host")
