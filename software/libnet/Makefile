MMDIR=../..
include $(MMDIR)/software/include.mak

OBJECTS=net.o tftp.o bootp.o rarp.o arp.o

all: libnet.a

libnet.a: $(OBJECTS)
	$(AR) clr libnet.a $(OBJECTS)
	$(RANLIB) libnet.a

.PHONY: clean depend

depend:
	makedepend -Y -- $(CFLAGS) -- *.c

clean:
	rm -f *.o libnet.a .*~ *~ Makefile.bak

# DO NOT DELETE

net.o: ../../software/include/net/net.h ../../software/include/net/arp.h ../../software/include/net/bootp.h ../../software/include/net/rarp.h 
net.o: ../../software/include/net/tftp.h ../../software/include/uart.h 
arp.o: ../../software/include/net/net.h ../../software/include/net/arp.h ../../software/include/net/bootp.h ../../software/include/net/tftp.h
bootp.o: ../../software/include/net/net.h ../../software/include/net/arp.h ../../software/include/net/bootp.h ../../software/include/net/tftp.h
rarp.o: ../../software/include/net/net.h ../../software/include/net/bootp.h ../../software/include/net/rarp.h ../../software/include/net/tftp.h
tftp.o: ../../software/include/net/net.h ../../software/include/net/bootp.h ../../software/include/net/tftp.h


