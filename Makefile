debug:
	mkdir -p build/Debug && cd build/Debug && cmake -DCMAKE_BUILD_TYPE=Debug ../.. && make -j 8

release:
	mkdir -p build/Release && cd build/Release && cmake -DCMAKE_BUILD_TYPE=Release ../.. && make -j 8

clean:
	rm -rf build

cleanInstall:
	sudo rm -rf /usr/local/include/sdl_core
	sudo rm -rf /usr/local/lib/libsdl_core.so

r: release

d: debug

copyRelease:
	sudo cp build/Release/lib/libsdl_core.so /usr/local/lib

copyDebug:
	sudo cp build/Debug/lib/libsdl_core.so /usr/local/lib

copyHeaders:
	sudo mkdir -p /usr/local/include/sdl_core
	sudo cp src/*.hh /usr/local/include/sdl_core
	sudo cp src/*.hxx /usr/local/include/sdl_core

install: r copyRelease copyHeaders

installD: d copyDebug copyHeaders
