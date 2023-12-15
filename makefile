all:
	cc main.c glad_gl.c -I inc -Ofast -lglfw -lm -o release/TuxScape_linux_high
	strip --strip-unneeded release/TuxScape_linux_high
	upx --lzma --best release/TuxScape_linux_high

med:
	cc main.c -DMED glad_gl.c -I inc -Ofast -lglfw -lm -o release/TuxScape_linux_med
	strip --strip-unneeded release/TuxScape_linux_med
	upx --lzma --best release/TuxScape_linux_med

low:
	cc main.c -DLOW glad_gl.c -I inc -Ofast -lglfw -lm -o release/TuxScape_linux_low
	strip --strip-unneeded release/TuxScape_linux_low
	upx --lzma --best release/TuxScape_linux_low

test:
	cc main.c glad_gl.c -I inc -Ofast -lglfw -lm -o /tmp/tuxscape_test
	/tmp/tuxscape_test
	rm /tmp/tuxscape_test

web:
	emcc main.c glad_gl.c -DWEB -O3 --closure 1 -s FILESYSTEM=0 -s USE_GLFW=3 -s ENVIRONMENT=web -s TOTAL_MEMORY=128MB -I inc -o bin/index.html --shell-file t.html

webmed:
	emcc -DMED main.c glad_gl.c -DWEB -O3 --closure 1 -s FILESYSTEM=0 -s USE_GLFW=3 -s ENVIRONMENT=web -s TOTAL_MEMORY=128MB -I inc -o bin/med/index.html --shell-file t.html

weblow:
	emcc -DLOW main.c glad_gl.c -DWEB -O3 --closure 1 -s FILESYSTEM=0 -s USE_GLFW=3 -s ENVIRONMENT=web -s TOTAL_MEMORY=128MB -I inc -o bin/low/index.html --shell-file t.html

run:
	emrun bin/index.html

runmed:
	emrun bin/med/index.html

runlow:
	emrun bin/low/index.html

win:
	i686-w64-mingw32-gcc main.c glad_gl.c -Ofast -I inc -L. -lglfw3dll -lm -o release/TuxScape_windows_high.exe
	strip --strip-unneeded release/TuxScape_windows_high.exe
	upx --lzma --best release/TuxScape_windows_high.exe

winmed:
	i686-w64-mingw32-gcc -DMED main.c glad_gl.c -Ofast -I inc -L. -lglfw3dll -lm -o release/TuxScape_windows_med.exe
	strip --strip-unneeded release/TuxScape_windows_med.exe
	upx --lzma --best release/TuxScape_windows_med.exe

winlow:
	i686-w64-mingw32-gcc -DLOW main.c glad_gl.c -Ofast -I inc -L. -lglfw3dll -lm -o release/TuxScape_windows_low.exe
	strip --strip-unneeded release/TuxScape_windows_low.exe
	upx --lzma --best release/TuxScape_windows_low.exe

clean:
	rm -f release/TuxScape_linux_high
	rm -f release/TuxScape_linux_med
	rm -f release/TuxScape_linux_low
	rm -f release/TuxScape_windows_high.exe
	rm -f release/TuxScape_windows_med.exe
	rm -f release/TuxScape_windows_low.exe
	rm -f TuxScape.exe
	rm -f bin/index.html
	rm -f bin/index.js
	rm -f bin/index.wasm
	rm -f bin/med/index.html
	rm -f bin/med/index.js
	rm -f bin/med/index.wasm
	rm -f bin/low/index.html
	rm -f bin/low/index.js
	rm -f bin/low/index.wasm
