cmake_minimum_required(VERSION 3.7)

project( LuaTutorial )

set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLUA_TUTORIAL_DEBUG" )	#so we can add the LUA_TUTORIAL_DEBUG preprocessor define and other flags to stay in debug mode - see https://cmake.org/Wiki/CMake_Useful_Variables#Compilers_and_Tools
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DLUA_TUTORIAL_DEBUG" )

if(MSVC)
	add_compile_options(/MP)				#Use multiple processors when building
	add_compile_options(/W4 /wd4201 /WX)	#Warning level 4, all warnings are errors
else()
	add_compile_options(-W -Wall -Werror) #All Warnings, all warnings are errors
endif()

# source for the test executable
set  (LUA_TUTORIAL_SOURCES
		"main.cpp"
		"ArenaAllocator.h" )
		
source_group("src" FILES ${LUA_TUTORIAL_SOURCES})
		
add_executable( LuaTutorial
	${LUA_TUTORIAL_SOURCES} 
	)

target_link_libraries( LuaTutorial PUBLIC LuaLib )

find_package(RTTR CONFIG REQUIRED Core)
target_link_libraries(LuaTutorial PUBLIC RTTR::Core_Lib)     # rttr as static library