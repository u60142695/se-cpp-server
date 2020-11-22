cmake_minimum_required(VERSION 3.16)

project("se-cpp-server")

set(LAST_DIR "")
function(GROUP_TARGET_SOURCE target_name)
    foreach(src_file ${${target_name}_SRC})
        file(RELATIVE_PATH relative_path ${CMAKE_CURRENT_SOURCE_DIR}/src/${target_name} ${src_file})
        get_filename_component(dir_name ${relative_path} DIRECTORY)
        if(NOT dir_name STREQUAL LAST_DIR)
            file(GLOB this_files "src/${target_name}/${dir_name}/*.*")
            string(REPLACE "/" "\\" group_name ${dir_name})
            source_group(${group_name} FILES ${this_files})
            set(LAST_DIR ${dir_name})
        endif()
    endforeach()
endfunction()

# Steamworks SDK
include_directories("3rdparty/steamworks_sdk_150/include")

if(NOT WIN32 AND NOT UNIX)
	message(FATAL_ERROR "Unsupported platform")
endif()

# Server Executable
if(UNIX)
    link_directories("3rdparty/steamworks_sdk_150/lib/linux64")
endif()

file(GLOB_RECURSE se-server_SRC "src/se-server/**.h" "src/se-server/**.cpp")
add_executable(se-server ${se-server_SRC})
GROUP_TARGET_SOURCE(se-server)

if(WIN32)
	target_link_libraries(se-server "../3rdparty/steamworks_sdk_150/lib/win64/steam_api64" ws2_32)
elseif(UNIX)
    target_link_libraries(se-server libsteam_api.so pthread)
endif()

# Copy Steamworks files to executable location.
if(WIN32)
	add_custom_command(TARGET se-server POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "../3rdparty/steamworks_sdk_150/lib/win64/steam_api64.dll" $<TARGET_FILE_DIR:se-server>)
    add_custom_command(TARGET se-server POST_BUILD
    	COMMAND ${CMAKE_COMMAND} -E copy "../steam_appid.txt" $<TARGET_FILE_DIR:se-server>)
elseif(UNIX)
	add_custom_command(TARGET se-server POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/3rdparty/steamworks_sdk_150/lib/linux64/libsteam_api.so" $<TARGET_FILE_DIR:se-server>)
    add_custom_command(TARGET se-server POST_BUILD
    	COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/steam_appid.txt" $<TARGET_FILE_DIR:se-server>)
endif()


