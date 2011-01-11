
ADD_LIBRARY( csparse STATIC
        cs.h
        cs_add.c
        cs_amd.c
        cs_chol.c
        cs_cholsol.c
        cs_compress.c
        cs_counts.c
        cs_cumsum.c
        cs_dfs.c
        cs_etree.c
        cs_fkeep.c
        cs_ipvec.c
        cs_leaf.c
        cs_lsolve.c
        cs_lu.c
        cs_lusol.c
        cs_malloc.c
        cs_multiply.c
        cs_norm.c
        cs_permute.c
        cs_post.c
        cs_print.c
        cs_reach.c
        cs_scatter.c
        cs_spsolve.c
        cs_sqr.c
        cs_tdfs.c
        cs_transpose.c
        cs_usolve.c
        cs_util.c
       )

SET(src cs.h
        mccodimage.h
        larith.h
        mcimage.h
        mcindic.h
        mclifo.h
        mcrbt.h
        mcutil.h
        larith.cpp
        lMSF.cpp
        mccodimage.cpp
        mcimage.cpp
        mcindic.cpp
        mclifo.cpp
        mcrbt.cpp
        cccodimage.cpp
        cccodimage.h
        ccsort.cpp
        ccsort.h
        gageodesic.cpp
        gageodesic.h
        image_toolbox.cpp
        image_toolbox.h
        lMSF.h
        MSF_RW.cpp
        MSF_RW.h
        powerwatersegm.cpp
        powerwatersegm.h
        random_walker.cpp
        random_walker.h
        union_find.cpp
        union_find.h )

SET(luasrc init.lua)

ADD_TORCH_PACKAGE(powerwatersegm "${src}" "${luasrc}")
TARGET_LINK_LIBRARIES(powerwatersegm luaT TH csparse)
