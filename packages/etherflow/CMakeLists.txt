SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/contrib/etherflow
                      ${CMAKE_MODULE_PATH})

SET(src etherflow.c
	etherflow.h) 
SET(luasrc init.lua) 

FIND_PACKAGE(PCAP)

#  ADD_TORCH_PACKAGE(etherflow "${src}" "${luasrc}")
#  TARGET_LINK_LIBRARIES(etherflow luaT TH ${PCAP_LIBRARIES})

IF(PCAP_FOUND) 
      INCLUDE_DIRECTORIES (${PCAP_INCLUDE_DIRS})
      LINK_DIRECTORIES (${PCAP_LIBRARIES})        
     
      IF(APPLE)
	FIND_PACKAGE(DNET)
        IF(DNET_FOUND)
           INCLUDE_DIRECTORIES (${DNET_INCLUDE_DIRS})
           LINK_DIRECTORIES (${DNET_LIBRARIES})
           ENDIF(DNET_FOUND)
       ENDIF(APPLE)
	
       ADD_TORCH_PACKAGE(etherflow "${src}" "${luasrc}")
       IF(APPLE)
           TARGET_LINK_LIBRARIES(etherflow luaT TH ${PCAP_LIBRARIES} ${DNET_LIBRARIES})
       ELSE(APPLE)
       IF(UNIX)
	 SET(_LINUX_ = 1)
	 SET(CMAKE_C_FLAGS "-D_LINUX_=1")
         TARGET_LINK_LIBRARIES(etherflow luaT TH ${PCAP_LIBRARIES})
       ENDIF(UNIX)
       ENDIF(APPLE)
ENDIF(PCAP_FOUND) 

