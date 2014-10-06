#include <stdio.h>
#include <sys/io.h>
#include <stdint.h>



uint32_t checkDevice(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t ret;
    outl (0x80000000 | (bus << 16) | (device << 11) | (function <<  8) | 0x00, 0xCF8 );
    ret = inl (0xCFC);
	return ret;
}



void checkAllBusesPrint() {
     int bus;
     int device;
     int function;
     uint32_t got;
     uint16_t ven, dev;
     
     for(bus = 0; bus < 256; bus++) {
         for(device = 0; device < 32; device++) {
    	     function = 0;
    	     got = checkDevice(bus, device, function);
    	     if (got == 0xFFFFFFFF) continue;
    	     dev = (uint16_t)((got >> 16) & 0xffff);
             ven = (uint16_t)((got >> 0 ) & 0xffff);
	     if(got != 0xFFFFFFFF) printf("VID: 0x%X DID: 0x%X\n", ven, dev);
             for(function = 1; function < 8; function++){
                   got = checkDevice(bus, device, function);
                   dev = (uint16_t)((got >> 16) & 0xffff);
	               ven = (uint16_t)((got >> 0 ) & 0xffff);
	               if(got != 0xFFFFFFFF) printf("VID: 0x%X DID: 0x%X\n", ven, dev);
             }
         }
     }
 }
 
int f(uint32_t number)
{
    int count = 0;
    for ( ; number; number >>= 1)
        if (number & 1)
           count++;
    return count;
}
 
 int checkAllBusesWright() {
     int bus;
     int device;
     int function;
     uint32_t got, bar2, base;
     uint16_t ven, dev;

     for(bus = 0; bus < 256; bus++) {
         for(device = 0; device < 32; device++) {
    	     for(function = 0; function < 8; function++){
                   //printf("%d %d %d\n", bus, device, function);
                   got = checkDevice(bus, device, function);
	           if(got != 0xFFFFFFFF) {
                       dev = (uint16_t)((got >> 16) & 0xffff);
	               ven = (uint16_t)((got >> 0 ) & 0xffff);
                       if(ven == 0x8086 && (dev == 0x1e00)){
			      printf("@ %d %d %d\n", bus, device, function);
                	      outl (0x80000000 | (bus << 16) | (device << 11) | (function <<  8) | 0x18 & 0xfc, 0xCF8 );
                	      bar2 = inl (0xCFC);
			      printf("%X\n", bar2);
                	      base = bar2 & 0xFFFC;
			      printf("%X\n", base);
                              outb(0x5A, base+2);
			      printf("%X\n", inb(base+2));
                              if (inb(base+2) == 0x5A) {printf("register pritomen\n");}
                              outb(0xE0, base+6);
			      printf("%X\n", inb(base+7));
                	      while ((inb(base+7) & 0x80) != 0);
                	      outb(0xE0, base+6);
                	      outb(0x01, base+2);
                	      outb(0x00, base+3);
                	      outb(0x00, base+4);
                	      outb(0x00, base+5);
                	      outb(0xE0, base+6);
                	      outb(0x20, base+7);
                	      while ((inb(base+7) & 0x80) != 0);
                	      if ((inb(base+7) & 0x80) == 0 /*&& (inb(base+7) & 0x04) != 0*/) {
                                    int i;
                                    for(i = 0; i < 256; i++){printf("0x%X\n", inw(base+0));}
                              }
                	      
                	} 
                    }
	      }
         }
     }
return 0;
 }
 
 int main() {
    iopl(3);
    checkAllBusesPrint();
    checkAllBusesWright();
    return 0;
}


//outb(0xFFFFFFFF, bar2);
//printf("velikost bloku registru = %d\n", f((inb(bar2) & 0xFFFFFFFC)^((inb(bar2)));
//outb(0x0d00, bar2);
//if (inb(bar2) == 0x0d00) {printf("register pritomen\n");}
                	      
