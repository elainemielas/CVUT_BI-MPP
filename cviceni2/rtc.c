#include <stdio.h>
#include <sys/io.h>

int main() {
	iopl(3);
	unsigned char ret, h, s, m, d, mo, r, dt;
	char den[2];
	outb(0xA, 0x74);
	ret = inb(0x75);
	while (ret & 0x80 == 1);
	outb(0x7, 0x74);
	d = inb(0x75);
	outb(0x8, 0x74);
	mo = inb(0x75);
	outb(0x9, 0x74);
	r = inb(0x75);
	outb(0x4, 0x74);
	h = inb(0x75);
	outb(0x2, 0x74);
	m = inb(0x75);
	outb(0x0, 0x74);
	s = inb(0x75);
	outb(0x6, 0x74);
	dt = inb(0x75);
	int idt = (int)dt;
	switch(idt){
		case 7: {den[0] = 'N'; den[1] = 'e'; break;}
		case 1: {den[0] = 'P'; den[1] = 'o'; break;}
		case 2: {den[0] = 'U'; den[1] = 't'; break;}
		case 3: {den[0] = 'S'; den[1] = 't'; break;}
		case 4: {den[0] = 'C'; den[1] = 't'; break;}
		case 5: {den[0] = 'P'; den[1] = 'a'; break;}
		case 6: {den[0] = 'S'; den[1] = 'o'; break;}
	}

	if (s<10){printf("%x.%x.%x, %c%c %x:%x:0%x \n", d, mo, r, den[0], den[1], h, m, s);}
	else {printf("%x.%x.%x, %c%c %x:%x:%x \n", d, mo, r, den[0], den[1], h, m, s);}
	return 0;
}
