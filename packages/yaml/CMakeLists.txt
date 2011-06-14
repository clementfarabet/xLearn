 SET(CMAKE_C_FLAGS "-O2 -Wall -shared -fPIC -fexceptions")
 SET(CMAKE_CXX_FLAGS "-O2 -Wall -shared -fPIC -fexceptions")
 SET(src lyaml.c api.c dumper.c emitter.c loader.c parser.c reader.c 
         scanner.c writer.c b64.c) 
 SET(luasrc init.lua) 

ADD_TORCH_PACKAGE(yaml "${src}" "${luasrc}")
TARGET_LINK_LIBRARIES(yaml luaT TH)


