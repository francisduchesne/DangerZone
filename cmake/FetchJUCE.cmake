# Fetch JUCE 8. Do not vendor JUCE in this repo.
# Danger Zone is a standalone instrument. It does not use FVS_Host.
include(FetchContent)

if (NOT DEFINED DZ_JUCE_GIT_TAG)
    set(DZ_JUCE_GIT_TAG "8.0.8")
endif()

set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        ${DZ_JUCE_GIT_TAG}
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(JUCE)
