
all: libnolua clean

lua: liblua clean

liblua: etherflow.c
	@echo "--> making shared lib with Lua wrappers"
	@gcc -fPIC -g -c -Wall etherflow.c
	@gcc -shared -Wl etherflow.o -lpcap -lc -llua -lluaT -lTH -o libetherflow.so

libnolua: etherflow.c
	@echo "--> making shared lib"
	@gcc -fPIC -D__NO_LUA__ -g -c -Wall etherflow.c
	@gcc -shared -Wl etherflow.o -lpcap -lc -o libetherflow.so

clean:
	@echo "--> cleaning"
	@rm -f *.o
	@rm -f *~
