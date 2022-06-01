add_subdirectory(CalDict)

add_executable(SabreCal)
target_include_directories(SabreCal
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/CalDict
	SYSTEM PUBLIC ${ROOT_INCLUDE_DIRS}
	)
target_sources(SabreCal PUBLIC
	ChannelMap.h
	ChannelMap.cpp
	GainMatcher.h
	GainMatcher.cpp
	GainMap.h
	GainMap.cpp
	Calibrator.h
	Calibrator.cpp
	CalibrationMap.h
	CalibrationMap.cpp
	CalibrationStructs.h
	DataOrganizer.h
	DataOrganizer.cpp
	main.cpp
	)
target_link_libraries(SabreCal
	CalDict
	${ROOT_LIBRARIES}
	)
set_target_properties(SabreCal PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${SABRECAL_BINARY_DIR}
	)
	
