set(MIA_DRIVER_SOURCES
	Mia_Base_LINUX.h
	Mia_Base_OS.cpp
	Mia_Base.cpp
	Mia_Base.h
	Mia_Device.cpp
	Mia_Device.h
	Mia_DriverBase.cpp
	Mia_DriverBase.h
	Mia_Driver.cpp
	Mia_Driver.h
	Mia_Equipment.cpp
	Mia_Equipment.h
	Mia_Exception.cpp
	Mia_Exception.h
)

include_directories(${CPMPLUS_SDK_INCLUDE_DIRS})
add_library(cpmplus-sdk STATIC ${MIA_DRIVER_SOURCES})

target_link_libraries(cpmplus-sdk
	MIA_Websocket 
	pthread 
	rt
	)

install(TARGETS cpmplus-sdk DESTINATION lib)
install(FILES 	Mia_Base.h 
				Mia_Device.h
				Mia_Driver.h
				Mia_Exception.h
				Mia_Equipment.h
				Mia_Base_LINUX.h
				Mia_DriverBase.h
		DESTINATION include		 				
)

