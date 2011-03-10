SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/contrib/etherflow
                      ${CMAKE_MODULE_PATH})

SET(src etherflow.c etherflow.h) 
SET(luasrc init.lua) 

FIND_PACKAGE(PCAP)
IF(PCAP_FOUND) 
    INCLUDE_DIRECTORIES (${PCAP_INCLUDE_DIRS})
    LINK_DIRECTORIES (${PCAP_LIBRARIES})

    IF(APPLE)
        FIND_PACKAGE(DNET)
        IF(DNET_FOUND)
            INCLUDE_DIRECTORIES (${DNET_INCLUDE_DIRS})
            LINK_DIRECTORIES (${DNET_LIBRARIES})
            ADD_TORCH_PACKAGE(etherflow "${src}" "${luasrc}")
            TARGET_LINK_LIBRARIES(etherflow luaT TH ${PCAP_LIBRARIES} ${DNET_LIBRARIES})
        ELSE(DNET_FOUND)
            MESSAGE("WARNING: Could not find LibDNET, etherflow [NeuFlow driver] will not be installed")
        ENDIF(DNET_FOUND)
    ELSE(APPLE)
         SET(_LINUX_ = 1)
         SET(CMAKE_C_FLAGS "-D_LINUX_=1")
         ADD_TORCH_PACKAGE(etherflow "${src}" "${luasrc}")
         TARGET_LINK_LIBRARIES(etherflow luaT TH ${PCAP_LIBRARIES})
    ENDIF(APPLE)

ELSE(PCAP_FOUND)
    MESSAGE("WARNING: Could not find LibPCAP, etherflow [NeuFlow driver] will not be installed")
ENDIF(PCAP_FOUND) 

