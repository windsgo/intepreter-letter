all: configure build

configure:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE

build: configure
	cmake --build build -j8

clean:
	rm -rf build && rm -rf bin

.PHONY: all configure build clean
