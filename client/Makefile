CC = gcc
vpath %.c src
vpath %.h include
CPPFLAGS = -I include

#Dependency
client: sock_connect.o
client.o: sock_connect.h
sock_connect.o: sock_connect.h

#Production rule
%.o: %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

%: %.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

#Pseudo target
.PHONY: install clean
install:

clean:
	rm *.o
