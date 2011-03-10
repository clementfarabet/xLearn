
# build pink lib
#FILE( GLOB lib_src_c *.c *.cc)
FILE( GLOB lib_src_c jcimage.c mcimage.c mccodimage.c jccodimage.c
                     lhierarchie.c lppm2GA.c lwshedtopo.c lderiche.c
                     lminima.c lga2khalimsky.c jcgraphes.c llpeGA.c
                     jccomptree.c mcunionfind.c mcfifo.c mcfifo.c
                     jcgraphes.c mcrbt.c jclabelextrema.c mclifo.c
                     mcindic.c mcfahsalembier.c mccomptree.c
                     llabelextrema.c)
SET(CMAKE_C_FLAGS "-fpic -g -DUNIXIO")
SET(CMAKE_CXX_FLAGS "-fpic -g -DUNIXIO")
ADD_LIBRARY(pinkcore STATIC ${lib_src_c})

# build wrapper
SET(src pink.cpp)
SET(luasrc init.lua)
SET(CMAKE_CXX_FLAGS "")
ADD_TORCH_PACKAGE(pink "${src}" "${luasrc}" "Graphs")
TARGET_LINK_LIBRARIES(pink luaT TH pinkcore)
