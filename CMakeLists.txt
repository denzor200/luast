cmake_minimum_required(VERSION 2.8)

set(SOURCE_FILES
        luast/ast.hpp
        luast/ast_adapted.hpp
        luast/common.hpp
        luast/config.hpp
        luast/error_handler.hpp
        luast/expression.cpp
        luast/expression.hpp
        luast/expression_def.hpp
        luast/main.cpp
        luast/visitors.hpp
        )


##########################################################
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package( Boost 1.72.0 COMPONENTS filesystem REQUIRED)
if(NOT Boost_FOUND)
	message(SEND_ERROR "Failed to find boost::filesystem.")
	return()
else()
	include_directories(${Boost_INCLUDE_DIRS})
endif()
##########################################################

add_executable(luast ${SOURCE_FILES})
target_link_libraries(luast ${Boost_LIBRARIES})


