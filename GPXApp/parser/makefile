all: parser

parser:
	gcc -Wall -std=c11 -g `xml2-config --cflags --libs` -Iinclude -fpic -c ./src/LinkedListAPI.c -o ./bin/LinkedListAPI.o
	gcc -Wall -std=c11 -g `xml2-config --cflags --libs` -Iinclude -fpic -c ./src/GPXParser.c -o ./bin/GPXParser.o
	gcc -Wall -std=c11 -g `xml2-config --cflags --libs` -Iinclude -fpic -shared ./bin/LinkedListAPI.o ./bin/GPXParser.o -o ../libgpxparse.so

clean:
	rm ../*.so ./bin/*.o
