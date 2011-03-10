.PHONY: build install help camiface luajit clean

INSTALL_PREFIX=/usr/local
USE_OPENCV=1
USE_V4L2=1
USE_CAMIFACE=1
USE_NEUFLOW=1
USE_3RDPARTY=1
USE_XFLOW=1
USE_JPEG=1
USE_MPEG2=1
USE_KINECT=1
USE_PINK=1
USE_VLFEAT=1
USE_MINCUT=1
USE_LUAJIT=0
USE_THREAD=1
USE_BIT=1
DOC=1
EXPORT=xLearn-beta

UNAME := $(shell uname)

all: help build

help:
	@-echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@-echo "usage (Linux/MacOS):"
	@-echo "  make [options] build     ? builds all the base/optional packages"
	@-echo "  make [options] install   ? builds + installs all the packages"
	@-echo "  make [options] export    ? exports the entire project into ${EXPORT}.tgz"
	@-echo "  make [options] luajit    ? builds/installs LuaJIT, with Torch bindings (speeds up Lua interpretation by a lot!!)"
	@-echo "  make update              ? just updates the install, without reconfiguring (useful for dev)"
	@-echo ""
	@-echo "options:"
	@-echo "  INSTALL_PREFIX=/path/to/install/dir [current = /usr/local]"
	@-echo "  EXPORT=filename  [current=xLearn-beta]"
	@-echo ""
	@-echo "  DOC=0|1           [current=${DOC}]   ? makes Documentation"
	@-echo ""
	@-echo "  USE_OPENCV=1|0    [current="${USE_OPENCV}"]   ? OpenCV 2.x Lua wrapper (Linux webcam), requires OpenCV 2.x"
	@-echo "  USE_V4L2=1|0      [current="${USE_V4L2}"]   ? video4linux2 Lua wrapper (Linux webcam), requires V4L2"
	@-echo "  USE_CAMIFACE=1|0  [current="${USE_CAMIFACE}"]   ? LibCamiface + Lua wrapper (MacOS webcam)"
	@-echo "  USE_NEUFLOW=1|0   [current="${USE_NEUFLOW}"]   ? Compiler + DevTools for the NeuFlow arch"
	@-echo "  USE_XFLOW=1|0     [current="${USE_XFLOW}"]   ? xFlow tools (xFlow parser/compiler + luaFlow framework)"
	@-echo "  USE_3RDPARTY=1|0  [current="${USE_3RDPARTY}"]   ? 3rd party packages (stereo, segmentation, optical flow)"
	@-echo "  USE_JPEG=1|0      [current="${USE_JPEG}"]   ? LibJpeg wrapper"
	@-echo "  USE_KINECT=1|0    [current="${USE_KINECT}"]   ? Libfreenect wrapper"
	@-echo "  USE_VLFEAT=1|0    [current="${USE_VLFEAT}"]   ? VLFeat + Lua wrapper (SIFT, MSER, k-means, hierarchical k-means, ...)"
	@-echo "  USE_PINK=1|0      [current="${USE_PINK}"]   ? Pink + Lua wrapper (morpho math library)"
	@-echo "  USE_MPEG2=1|0     [current="${USE_MPEG2}"]   ? LibMpeg2 wrapper"
	@-echo "  USE_BITOP=1|0     [current="${USE_BIT}"]   ? LuaBitOP library (bitwise operators for Lua)"
	@-echo "  USE_THREAD=1|0    [current="${USE_THREAD}"]   ? Multithreaded Lib for Lua, allows threads sharing the same Lua stack"
	@-echo "  USE_LUAJIT=1|0    [current="${USE_LUAJIT}"]   ? Replaces Lua by LuaJIT (Linux only)"
	@-echo ""
	@-echo "notes:"
	@-echo "  + LuaJIT is very experimental, and is not intended for development."
	@-echo "    Installing it erases regular Lua libs, and QLua uses it by default."
	@-echo "    (a make clean is also required when switching from one to the other)"
	@-echo "  + All packages are now included by default (USE_***=1), and properly"
	@-echo "    auto-detected by cmake. For instance, Camiface won't be installed"
	@-echo "    on Linux, as it's only compatible with MacOS, while V4L2 will only"
	@-echo "    be installed on Linux."
	@-echo ""
	@-echo "examples:"
	@-echo "  local install:"
	@-echo "    $ make INSTALL_PREFIX=~/local install"
	@-echo ""
	@-echo "  root install, with no OpenCV wrapper:"
	@-echo "    $ sudo make USE_OPENCV=0 install"
	@-echo ""
	@-echo "  update (the project is already built, just updating):"
	@-echo "    $ git pull"
	@-echo "    $ [sudo] make update"
	@-echo ""
	@-echo "  root install, replacing Lua by LuaJIT for improved perfs [linux only]"
	@-echo "    $ sudo make USE_LUAJIT=1 install"
	@-echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

build: camiface luajit
	@-mkdir -p torch/scratch
	cd torch/scratch && PATH=${INSTALL_PREFIX}/bin:${PATH}  && cmake .. -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DWITH_CONTRIB_XLearn=1 -DHTML_DOC=${DOC} -DWITH_CONTRIB_debugger=1 -DUSE_LUAJIT=${USE_LUAJIT} -DWITH_CONTRIB_thread=${USE_THREAD} -DWITH_CONTRIB_opticalFlow=${USE_3RDPARTY} -DWITH_CONTRIB_mstsegm=${USE_3RDPARTY} -DWITH_CONTRIB_stereo=${USE_3RDPARTY} -DWITH_CONTRIB_powerwatersegm=${USE_3RDPARTY} -DWITH_CONTRIB_bit=${USE_BIT} -DWITH_CONTRIB_camiface=${USE_CAMIFACE} -DWITH_CONTRIB_luaFlow=${USE_XFLOW} -DWITH_CONTRIB_xFlow=${USE_XFLOW} -DWITH_CONTRIB_NeuFlow=${USE_NEUFLOW} -DWITH_CONTRIB_opencv=${USE_OPENCV} -DWITH_CONTRIB_video4linux=${USE_V4L2} -DWITH_CONTRIB_etherflow=${USE_NEUFLOW} -DWITH_CONTRIB_jpeg=${USE_JPEG} -DWITH_CONTRIB_vlfeat=${USE_VLFEAT} -DWITH_CONTRIB_pink=${USE_PINK} -DWITH_CONTRIB_Kinect=${USE_KINECT} -DWITH_CONTRIB_mincut=${USE_MINCUT} -DWITH_CONTRIB_mpeg2=${USE_MPEG2} && make
	@-echo ""
	@-echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@-echo "Build succesful, install with admin rights (in /usr/local):"
	@-echo "$ [sudo] make install"
	@-echo ""
	@-echo "in any other dir:"
	@-echo "$ make install INSTALL_PREFIX=/path/to/local/dir"
	@-echo ""
	@-echo "for more options:"
	@-echo "$ make help"
	@-echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

camiface:
ifeq (${USE_CAMIFACE},1)
ifeq (${UNAME},Darwin)
	@-echo "+++ installing camiface +++"
	@-mkdir -p camiface/scratch
	cd camiface/scratch && cmake .. -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} && make
else
	@-echo "+++ bypassing camiface [MacOS only] +++"
endif
endif

luajit:
ifeq (${USE_LUAJIT},1)
	@-echo "+++ installing LuaJIT +++"
	cd luajit-torch && make && make install PREFIX=${INSTALL_PREFIX}
endif

ixflow:
ifeq (${USE_XFLOW},1)
	@-echo "+++ installing xFlow tools +++"
	cd xFlow && make install INSTALL_PREFIX=${INSTALL_PREFIX}
endif

extrabins:
	@-echo "qlua -ide -i" ${INSTALL_PREFIX}"/share/lua/5.1/XLearn/luaX.lua" > ${INSTALL_PREFIX}"/bin/luaX"
	@-chmod +x ${INSTALL_PREFIX}"/bin/luaX"
	@-echo "qlua -lXLearn $1" > ${INSTALL_PREFIX}"/bin/xlua"
	@-chmod +x ${INSTALL_PREFIX}"/bin/xlua"
	@-cp "packages/debugger/luaD" ${INSTALL_PREFIX}"/bin/luaD"
	@-chmod +x ${INSTALL_PREFIX}"/bin/luaD"

install: installLua installLuaDoc report
	@-echo ${INSTALL_PREFIX} > torch/scratch/PREFIX

installLuarock:
	@-echo "+++ installing luarock +++"
	cd luarocks && ./configure && make && make install PREFIX=${INSTALL_PREFIX}

installLuaDoc: installLuarock
	luarocks install luadoc

doc: force
	@-echo "+++ generating documentation +++"
	cd doc && ./gendoc.lua *

force: ;

report:
	@-echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@-echo "Install complete, you should have access to:"
	@-echo "lua (plain/original Lua interpreter)"
ifeq (${USE_LUAJIT},1)
	@-echo "luajit (LuaJIT compiler)"
	@-echo "qlua (LuaJIT+QT bindings)"
else
	@-echo "qlua (Lua+QT bindings)"
	@-echo "luaD (run Lua in debug mode)"
endif
	@-echo "xlua (qlua + preloaded XLearn)"
	@-echo "luaX (gui/matlab-like environment)"
	@-echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

installLua: build ixflow extrabins	
	@-cd torch/scratch && make install
ifeq (${USE_CAMIFACE},1)
ifeq (${UNAME},Darwin)
	@-cd camiface/scratch && make install
endif
endif

update:
	@-echo "+++ updating install +++"
	@-cd torch/scratch && make install
	@-echo "+++ install updated +++"

devlinks:
	@-echo "+++ creating soft links for easier devel +++"
	@-rm -rf ${INSTALL_PREFIX}/share/lua/5.1/XLearn
	ln -sf `pwd`/torch/contrib/XLearn ${INSTALL_PREFIX}/share/lua/5.1/XLearn
	@-rm -rf ${INSTALL_PREFIX}/share/lua/5.1/NeuFlow
	ln -sf `pwd`/torch/contrib/NeuFlow ${INSTALL_PREFIX}/share/lua/5.1/NeuFlow
	@-rm -rf ${INSTALL_PREFIX}/share/lua/5.1/luaFlow
	ln -sf `pwd`/torch/contrib/luaFlow ${INSTALL_PREFIX}/share/lua/5.1/luaFlow
	@-echo "+++ creating soft links for easier devel +++"

export:
	@-echo "+++ exporting project to" ${EXPORT}.tgz "+++"
	@-rm -rf ${EXPORT}
	@-mkdir ${EXPORT}
	@-cp -r torch ${EXPORT}/
	@-rm -rf ${EXPORT}/torch/contrib
	@-cp -r packages ${EXPORT}/torch/contrib
ifeq (${USE_CAMIFACE},1)
	@-cp -r camiface ${EXPORT}/
endif
	@-cp -r luajit-torch ${EXPORT}/
	@-cp Makefile ${EXPORT}/
	@-cp -r demos ${EXPORT}/
	@-cp -r trained-nets ${EXPORT}/
	@-cp get-dataset ${EXPORT}/
	@-cp README.txt ${EXPORT}/
	@-cp COPYRIGHT.txt ${EXPORT}/
ifeq (${USE_NEUFLOW},1)
	@-cp -r demos-neuflow ${EXPORT}/
endif
	@-rm -rf ${EXPORT}/*/scratch
	@-tar -czvf ${EXPORT}.tgz ${EXPORT}
	@-rm -rf ${EXPORT}
	@-echo "+++ export complete +++"

cleandoc:
	@-echo "+++ cleaning doc +++"
	@-rm -rf doc/*html doc/files doc/*.css doc/modules
	@-echo "+++ cleanup doc done +++"

clean: 
	@-echo "+++ cleaning up +++"
	@-rm -rf torch/scratch
	@-cd luajit-torch; make clean
	@-cd luarocks; make clean
	@-rm -rf camiface/scratch
	@-echo "+++ cleanup done +++"

cleanall: cleandoc
	@-echo "+++ deep cleanup +++"
	@-rm -rf trained-nets
	@-rm -rf datasets
	@-rm -rf scratch
	@-rm -rf */scratch
	@-rm -rf */*/scratch
	@-rm -rf */*/*/scratch
	@-rm -rf */*/*/*/scratch
	@-rm -rf videos
	@-rm -rf */videos
	@-rm -rf */*/videos
	@-rm -rf */*/*/videos
	@-rm -rf */*/*/*/videos
	@-cd luajit-torch; make clean
	@-rm -rf camiface/scratch
	@-echo "+++ deep cleanup done +++"

INSTALLED := $(shell if [ -f torch/scratch/PREFIX ]; then cat torch/scratch/PREFIX; fi;) \

uninstall: 
	@-if test -f torch/scratch/PREFIX; then\
		echo --- uninstalling ---; \
		rm -rf `cat torch/scratch/PREFIX`/share/lua;\
		rm -rf `cat torch/scratch/PREFIX`/share/luajit*;\
		rm -rf `cat torch/scratch/PREFIX`/lib/lua;\
		rm -rf `cat torch/scratch/PREFIX`/lib/luarocks;\
		rm -rf `cat torch/scratch/PREFIX`/lib/liblua*;\
		rm -rf `cat torch/scratch/PREFIX`/lib/libqlua*;\
		rm -rf `cat torch/scratch/PREFIX`/lib/libqtlua*;\
		rm -rf `cat torch/scratch/PREFIX`/bin/lua;\
		rm -rf `cat torch/scratch/PREFIX`/bin/luac;\
		rm -rf `cat torch/scratch/PREFIX`/bin/luaD;\
		rm -rf `cat torch/scratch/PREFIX`/bin/luadoc;\
		rm -rf `cat torch/scratch/PREFIX`/bin/luajit*;\
		rm -rf `cat torch/scratch/PREFIX`/bin/luarocks*;\
		rm -rf `cat torch/scratch/PREFIX`/bin/luaX;\
		rm -rf `cat torch/scratch/PREFIX`/bin/qlua;\
		rm -rf `cat torch/scratch/PREFIX`/bin/summary.lua;\
		rm -rf `cat torch/scratch/PREFIX`/bin/xlua;\
	else\
		echo  --- not PREFIX file, installation was not completed properly ---;\
	fi