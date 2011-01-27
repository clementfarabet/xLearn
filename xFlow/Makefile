.PHONY: install

INSTALL_PREFIX=/usr/local

help:
	@-echo "usage (Linux/MacOS)"
	@-echo "  make [path] install   ? installs xflow tools"
	@-echo ""
	@-echo "options:"
	@-echo "  INSTALL_PREFIX=/path/to/install/dir [default = /usr/local]"

install:
	@-echo "+++ installing xFlow tools +++"
	@-mkdir -p ${INSTALL_PREFIX}/share/xFlow/
	@-mkdir -p ${INSTALL_PREFIX}/bin/
	cp -r m4 emacs examples libraries  ${INSTALL_PREFIX}/share/xFlow/
	cp xflow-translate ${INSTALL_PREFIX}/bin/
	@-echo "--- installing xFlow tools ---"
