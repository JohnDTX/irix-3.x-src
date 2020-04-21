.SUFFIXES: .b .ls
OBJS = serial.b buttons.b getline.b mch.b
INCLUDES = -I$$INCLUDE -I$$IRIS/include

iolib.a: $(OBJS)
	rm -f iolib.a
	ar cr iolib.a $(OBJS)
	ranlib68 iolib.a

clean:
	rm -f *.b
	rm -f iolib.a

.c.b: 
	cc68 $(INCLUDES) -c $< 
.c.ls: 
	cc68 $(INCLUDES) -c -L $< 

mch.b: mch.s
	cc68 -c mch.s 

test: test.c
	cc68 test.c -r -o test iolib.a
