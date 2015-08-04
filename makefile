all: build

run: build
	@./bin/ymlPipe examples/testPipe.yml

clean:
	@rm -rf bin

build:
	@mkdir -p bin
	@g++ src/ymlPipe.cpp -o bin/ymlPipe -L/usr/local/lib -I/usr/local/include \
	-lyaml-cpp
