message(STATUS "RTNeural -- Using Eigen backend")
target_compile_definitions(RTNeural PUBLIC RTNEURAL_USE_EIGEN=1)
target_include_directories(RTNeural PUBLIC modules/Eigen)
