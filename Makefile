CC=g++-11
CFLAGS=-ISorensenCompression -flto -Ofast
OBJ = SorensenCompression/main.cpp

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sorensen: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm sorensen
