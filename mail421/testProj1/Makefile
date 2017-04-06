all: testMailBox testMessage

testMailBox:
	gcc -o testMailBox.o testMailBox.c
	$(info *** Run as root ***)	

testMessage:
	gcc -o testMessage.o testMessage.c

clean:
	$(RM) count *.o *~
