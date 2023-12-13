all:
	gcc ../main.c ../glad_gl.c -I ../inc -Ofast -lglfw -lm -o tuxscape

install:
	cp tuxscape $(DESTDIR)

uninstall:
	rm $(DESTDIR)/tuxscape

clean:
	rm tuxscape