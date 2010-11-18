.PHONY: all install help camiface luajit clean

INSTALL_PREFIX=/usr/local
USE_OPENCV=0
USE_CAMIFACE=0
USE_NEUFLOW=0
USE_JPEG=0
USE_MPEG2=0
USE_LUAJIT=0
USE_THREAD=1
USE_BIT=1
EXPORT=xLearn-beta

help:
	@-echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@-echo "usage (Linux/MacOS)"
	@-echo "  make [options] all       ? builds all the base/optional packages"
	@-echo "  make [options] install   ? builds + installs all the packages"
	@-echo "  make [options] export    ? exports the entire project into EXPORT.tgz"
	@-echo "  make [options] luajit    ? builds/installs LuaJIT, with Torch bindings (speeds up Lua interpretation by a lot!!)"
	@-echo "  make update              ? just updates the install, without reconfiguring (useful for dev)"
	@-echo ""
	@-echo "options:"
	@-echo "  INSTALL_PREFIX=/path/to/install/dir [default = /usr/local]"
	@-echo "  EXPORT=filename  [default=xLearn-beta]"
	@-echo ""
	@-echo "  USE_OPENCV=1   [default=0]   ? OpenCV 2.x Lua wrapper (Linux webcam), requires OpenCV 2.x"
	@-echo "  USE_CAMIFACE=1 [default=0]   ? LibCamiface + Lua wrapper (MacOS webcam)"
	@-echo "  USE_NEUFLOW=1  [default=0]   ? Compiler + DevTools for the NeuFlow arch"
	@-echo "  USE_JPEG=1     [default=0]   ? LibJpeg wrapper"
	@-echo "  USE_MPEG2=1    [default=0]   ? LibMpeg2 wrapper"
	@-echo "  USE_BITOP=0    [default=1]   ? LuaBitOP library (bitwise operators for Lua)"
	@-echo "  USE_THREAD=0   [default=1]   ? Multithreaded Lib for Lua, allows threads sharing the same Lua stack"
	@-echo "  USE_LUAJIT=1   [default=0]   ? Replaces Lua by LuaJIT (Linux only)"
	@-echo ""
	@-echo "notes:"
	@-echo "  LuaJIT is very experimental, and is not intended for development."
	@-echo "  Installing it erases regular Lua libs, and QLua uses it by default."
	@-echo "  (a make clean is also required when switching from one to the other)"
	@-echo ""
	@-echo "examples:"
	@-echo "  local install, with OpenCV wrapper and JPEG lib wrapper:"
	@-echo "    $ make INSTALL_PREFIX=~/local USE_OPENCV=1 USE_JPEG=1 install"
	@-echo ""
	@-echo "  root install, with OpenCV wrapper:"
	@-echo "    $ sudo make USE_OPENCV=1 install"
	@-echo ""
	@-echo "  dev build (the project is already built, just updating):"
	@-echo "    $ [sudo] make update"
	@-echo ""
	@-echo "  root install, including all tools for the neuFlow arch [linux only]"
	@-echo "    $ sudo make USE_NEUFLOW=1 USE_OPENCV=1 install"
	@-echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

all: camiface luajit
	@-mkdir -p torch/scratch
	cd torch/scratch && PATH=${INSTALL_PREFIX}/bin:${PATH}  && cmake .. -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DWITH_CONTRIB_XLearn=1 -DHTML_DOC=1 -DWITH_CONTRIB_debugger=1 -DUSE_LUAJIT=${USE_LUAJIT} -DWITH_CONTRIB_thread=${USE_THREAD} -DWITH_CONTRIB_bit=${USE_BIT} -DWITH_CONTRIB_camiface=${USE_CAMIFACE} -DWITH_CONTRIB_NeuFlow=${USE_NEUFLOW} -DWITH_CONTRIB_opencv=${USE_OPENCV} -DWITH_CONTRIB_etherflow=${USE_NEUFLOW} -DWITH_CONTRIB_jpeg=${USE_JPEG} -DWITH_CONTRIB_mpeg2=${USE_MPEG2} && make

camiface:
ifeq (${USE_CAMIFACE},1)
	@-mkdir -p camiface/scratch
	cd camiface/scratch && cmake .. -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} && make
endif

luajit:
ifeq (${USE_LUAJIT},1)
	cd luajit-torch && make && make install PREFIX=${INSTALL_PREFIX}
endif

extrabins:
	@-echo "qlua -ide -lXLearn -i" ${INSTALL_PREFIX}"/share/lua/5.1/XLearn/luaX.lua" > ${INSTALL_PREFIX}"/bin/luaX"
	@-chmod +x ${INSTALL_PREFIX}"/bin/luaX"
	@-cp "packages/debugger/luaD" ${INSTALL_PREFIX}"/bin/luaD"
	@-chmod +x ${INSTALL_PREFIX}"/bin/luaD"

install: all extrabins
	@-cd torch/scratch && make install
ifeq (${USE_CAMIFACE},1)
	@-cd camiface/scratch && make install
endif
	@-echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@-echo "Install complete, you should have access to:"
ifeq (${USE_LUAJIT},1)
	@-echo "lua (plain Lua interpreter)"
	@-echo "luajit (LuaJIT compiler)"
	@-echo "qlua (LuaJIT+QT bindings)"
else
	@-echo "lua (plain Lua interpreter)"
	@-echo "qlua (Lua+QT bindings)"
	@-echo "luaD (run Lua in debug mode)"
	@-echo "luaX (gui/matlab-like environment)"
endif
	@-echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

update:
	@-echo "+++ updating install +++"
	@-cd torch/scratch && make install
	@-echo "+++ install updated +++"

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

clean:
	@-echo "+++ cleaning up +++"
	@-rm -rf torch/scratch
	@-cd luajit-torch; make clean
	@-rm -rf camiface/scratch
	@-echo "+++ cleanup done +++"