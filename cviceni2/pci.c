#include <stdio.h>
#include <sys/io.h>
#include <stdint.h>



uint32_t checkDevice(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t ven, dev;
    uint32_t ret;
    outl (0x80000000 | (bus << 16) | (device << 11) | (function <<  8) | 0x00, 0xCF8 );
        ret = inl (0xCFC);
	dev = (uint16_t)((ret >> 16) & 0xffff);
	ven = (uint16_t)((ret >> 0 ) & 0xffff);
	if(ret != 0xFFFFFFFF) printf("VID: 0x%X DID: 0x%X\n", ven, dev);
	return ret;
}

uint32_t findDevice(uint8_t bus, uint8_t device, uint8_t function) {
    uint16_t ven, dev;
    uint32_t ret;
    outl (0x80000000 | (bus << 16) | (device << 11) | (function <<  8) | 0x00, 0xCF8 );
        ret = inl (0xCFC);
	dev = (uint16_t)((ret >> 16) & 0xffff);
	ven = (uint16_t)((ret >> 0 ) & 0xffff);
	if(ret != 0xFFFFFFFF) {
	   if(ven == 0x8086 && (dev == 0x1e00 || dev == 0x1e08)){
	      outl (0x80000000 | (bus << 16) | (device << 11) | (function <<  8) | 0x18 & 0xfc, 0xCF8 );
	      ret = inl (0xCFC);
	    }
           else ret = 0xFFFFFFFF;
	}
	return ret;
}


void checkAllBuses() {
     uint8_t bus;
     uint8_t device;
     uint8_t function;
     uint32_t got;
     
     for(bus = 0; bus < 256; bus++) {
         for(device = 0; device < 32; device++) {
	     //function = 0;
	     //got = checkDevice(bus, device, function);
	     //if (got == 0xFFFFFFFF) { continue;} 
             for(function = 0; function < 8; function++){
                got = findDevice(bus, device, function);  
             }
         }
     }
 }
 
 int main() {
    iopl(3);
    checkAllBuses();
    return 0;
}
