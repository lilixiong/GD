
#SFML:
IF(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SFML")
	message( "Downloading SFML... (can take a bit of time, be patient)" )
	execute_process(COMMAND curl -o SFML.7z http://www.compilgames.net/code/GameDevelopSDK/SFML.7z -\# WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ )
	message( "Extracting SFML in ExtLibs/SFML... (can take a bit of time, be patient)" )
	execute_process(COMMAND 7za x -y SFML.7z WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ OUTPUT_QUIET)
ENDIF()
IF(NOT EMSCRIPTEN) #Don't build SFML binaries when compiling with emscripten (but keep include files!)
	ADD_SUBDIRECTORY(SFML)
	set(sfml_lib_dir ${CMAKE_BINARY_DIR}/ExtLibs/SFML/lib PARENT_SCOPE)
ENDIF()
set(sfml_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/SFML/include PARENT_SCOPE)

#wxWidgets:
IF(NOT NO_GUI)
	IF(WIN32)
		#Precompiled wxWidgets on Windows:
		IF(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wxwidgets")
			message( "Downloading precompiled version of wxWidgets... (can take a bit of time, be patient)" )
			execute_process(COMMAND curl -o wxwidgets.7z http://www.compilgames.net/code/GameDevelopSDK/wxwidgets.7z -\# WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ )
			message( "Extracting wxWidgets in ExtLibs/wxWidgets... (can take a bit of time, be patient)" )
			execute_process(COMMAND 7za x -y wxwidgets.7z WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ OUTPUT_QUIET)
		ENDIF()
	ELSE()
		#Use wx-config on *nix:
		find_package(wxWidgets REQUIRED)
		IF (NOT APPLE)
			find_package(PkgConfig REQUIRED)
			EXEC_PROGRAM(${wxWidgets_CONFIG_EXECUTABLE} ARGS --query-toolkit OUTPUT_VARIABLE WXWIDGETS_GTK_VERSION)
			IF(${WXWIDGETS_GTK_VERSION} MATCHES "gtk2")
				pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
				set(GTK_INCLUDE_DIRS "${GTK2_INCLUDE_DIRS}" PARENT_SCOPE)
				set(GTK_LIBRARY_DIRS "${GTK2_LIBRARY_DIRS}" PARENT_SCOPE)
				set(GTK_CFLAGS_OTHER "${GTK2_CFLAGS_OTHER}" PARENT_SCOPE)
			ELSEIF(${WXWIDGETS_GTK_VERSION} MATCHES "gtk3")
				pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
				set(GTK_INCLUDE_DIRS "${GTK3_INCLUDE_DIRS}" PARENT_SCOPE)
				set(GTK_LIBRARY_DIRS "${GTK3_LIBRARY_DIRS}" PARENT_SCOPE)
				set(GTK_CFLAGS_OTHER "${GTK3_CFLAGS_OTHER}" PARENT_SCOPE)
			ELSE()
				message(WARNING "Unable to identify the GTK+ version used by wxWidgets")
				set(GTK_INCLUDE_DIRS "" PARENT_SCOPE)
				set(GTK_LIBRARY_DIRS "" PARENT_SCOPE)
				set(GTK_CFLAGS_OTHER "" PARENT_SCOPE)
			ENDIF()
		ENDIF()
	ENDIF()
ENDIF()
