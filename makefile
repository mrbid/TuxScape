all:
	mkdir -p release
	cc main.c glad_gl.c -I inc -Ofast -lglfw -lm -o release/tuxscape
	strip --strip-unneeded release/tuxscape
	upx --lzma --best release/tuxscape

test:
	cc main.c glad_gl.c -I inc -Ofast -lglfw -lm -o /tmp/tuxscape_test
	/tmp/tuxscape_test
	rm /tmp/tuxscape_test

web:
	emcc main.c glad_gl.c -DWEB -O3 --closure 1 -s FILESYSTEM=0 -s USE_GLFW=3 -s ENVIRONMENT=web -s TOTAL_MEMORY=128MB -I inc -o bin/index.html --shell-file t.html

run:
	emrun bin/index.html

clean:
	rm -f -r release
	rm -f bin/index.html
	rm -f bin/index.js
	rm -f bin/index.wasm
