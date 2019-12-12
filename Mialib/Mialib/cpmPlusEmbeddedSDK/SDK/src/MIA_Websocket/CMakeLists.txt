set(MIA_WEBSOCKET_SOURCES
	WS_Error.c
	WS_Socket.c
	WS_Util.c
	WS_Websocket.c
)

add_library(MIA_Websocket STATIC ${MIA_WEBSOCKET_SOURCES})

target_link_libraries(MIA_Websocket 
	ssl 
	crypto
	)

install(TARGETS MIA_Websocket DESTINATION lib)	
install(FILES 	WS_Error.h 
				WS_Socket.h
				WS_Util.h
				WS_Websocket.h
		DESTINATION include		 				
)