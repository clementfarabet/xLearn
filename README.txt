
================================================================================
What's in this package ??

Torch 5     Torch5 provides a Matlab-like environment for state-of-the-art 
            machine learning algorithms. It is easy to use and provides a 
            very efficient implementation, thanks to an easy and fast 
            scripting language (Lua) and a underlying C implementation.
            
            The Torch 5 library is re-distributed here for simplicity of 
            installation.
            The original package can be found here:
            http://torch5.sourceforge.net

            The distribution has been slightly modified, in particular, 
            the original Lua kernel has been patched for multi-threaded 
            applications.
            
            Torch is licensed under a BSD license:
            http://torch5.sourceforge.net/manual/License.html

xLearn      xLearn is an extension library for torch. It provides dozens 
            of tools/modules for vision, image processing, and machine 
            learning for vision.

luaFlow     luaFlow is a unified flow-graph description environment for
[beta]      vision / image-processing types of applications. One of its
            primary objectives is to abstract computing platforms, by 
            providing a unified, high-level description flow.

xFlow       a serializing language for luaFlow, that allows algorithms to
[beta]      be imported/exported from/to other software frameworks

neuFlow     neuFlow is the compiler toolkit for the neuFlow processor, 
            developped at New York University / Yale University.
            The neuFlow processor is dataflow computer optimized for
            vision and bio-inspired models of vision. 
            The neuFlow compiler currently converts xLearn/torch algorithms
            to native neuFlow's bytecode.
            Soon to appear is a luaFlow>neuFlow compiler, which would   
            simplify retargetting.
            It is quite important to have access to a neuFlow device to 
            be able to experiment with it: for more info/support, to get
            a neuFlow-enabled board, please contact clement.farabet@gmail.com


================================================================================
INSTALL

(1-LINUX) 
    install dependencies (compilation tools, cmake, QT4):
    $ sudo apt-get install gcc g++ cmake libqt4-core libqt4-dev libqt4-gui

    recommended (better shell)
    $ sudo apt-get install libreadline5-dev

    optionally, install OpenCV 2.1, to get access to your webcam:
    http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.1/

    optionally, install PCAP for the ethernet driver (neuFlow)
    $ sudo apt-get install libpcap-dev

(1-MACOS)
    install dependencies (readline, cmake, QT4)
    $ sudo port install cmake readline-5 cmake qt4-mac-devel

    you might want to use a prebuilt version of QT4, to avoid the 2 hour
    build time... (I still don't understand
    why MacPort relies on sources rather than binaries...)
    I'm keeping a working version here (installs in 5mins):
    http://data.clement.farabet.net/qt/qt-mac-cocoa-opensource-4.5.3.dmg

    optionally, install libcamiface to interface to your webcam, 
    simply run:
    $ ./install-camera-macos
    that compiles libcamiface, and installs a
    little program called frame_grabber-*** that acts as a frame server.

(2-COMMON) 
    once the dependencies are installed, just run:
    $ [sudo] make install
    for the default install

    or just
    $ make
    for more info about the options/submodules

    example of a local install, with one optional module:
    $ make install INSTALL_PREFIX=~/local USE_OPENCV=1
