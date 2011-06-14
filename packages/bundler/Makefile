DIRS	= imagelib matrix sba-1.5 sfm-driver
OBJLIBS	= libimage.a  libmatrix.a  libsba.v1.5.a  libsfmdrv.a

all : force_look
	-for d in $(DIRS); do (cd $$d; make); done

clean :
	-echo cleaning up in .
	-rm -f $(OBJLIBS)
	-for d in $(DIRS); do (cd $$d; make clean ); done

force_look :
	true