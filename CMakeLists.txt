cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(OISB LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH $ENV{OGRE_SDK}/CMake)

add_definitions(-D_LIB -D_CRT_SECURE_NO_WARNINGS -DOISB_STATIC_LIB)

add_library(OISB
        src/OISBAction.cpp
        src/OISBActionSchema.cpp
        src/OISBAnalogAxisAction.cpp
        src/OISBAnalogAxisState.cpp
		src/OISBAnalogEmulation.cpp
        src/OISBBindable.cpp
        src/OISBBinding.cpp
        src/OISBDevice.cpp
        src/OISBDigitalState.cpp
        src/OISBJoyStick.cpp
        src/OISBKeyboard.cpp
        src/OISBMouse.cpp
        src/OISBPropertySet.cpp
        src/OISBSequenceAction.cpp
        src/OISBState.cpp
        src/OISBSystem.cpp
        src/OISBTriggerAction.cpp
        )

target_include_directories(OISB PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

target_compile_features(OISB
        PUBLIC cxx_auto_type
        PRIVATE cxx_variadic_templates
        )

target_link_libraries(OISB
		#${COCOA_LIBRARY}
		#${IOKIT_LIBRARY}
		OIS
		)

install(TARGETS OISB
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		RUNTIME DESTINATION bin
		)
install(DIRECTORY include/ DESTINATION include)

export(TARGETS OISB FILE OISBConfig.cmake)


