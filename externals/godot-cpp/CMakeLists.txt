# Godot CPP Bindings
FetchContent_Declare(
        godot-cpp
        GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
        GIT_TAG master # Waiting for 4.6 official bindings
)
FetchContent_MakeAvailable(godot-cpp)