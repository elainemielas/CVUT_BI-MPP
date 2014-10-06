#include "log.h"
#include "led.h"
#include "usb_lib.h"
#include "touchpad.h"
#include "cpu.h"
#include "stdlib.h"

#ifndef min
	#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

typedef enum
{
ATTACHED, POWERED, DEFAULT
} devstate;

//
// Delky bufferu
#define EP0_OUT_BUF_SIZE 64
#define EP0_IN_BUF_SIZE 64
#define EP1_IN_BUF_SIZE 64
#define EP2_OUT_BUF_SIZE 64
//
// Deklarace koncov�ch bod�
//
DECLARE_EP_BEGIN
    DECLARE_EP(ep0);   // IN/OUT ��d�c� koncov� bod
    DECLARE_EP(ep1);   // IN koncov� bod
    DECLARE_EP(ep2);   // OUT koncov� bod
DECLARE_EP_END
//
// Deklarace buffer�
//
DECLARE_BUFFER_BEGIN
    DECLARE_BUFFER(ep0_buf_out, EP0_OUT_BUF_SIZE);
    DECLARE_BUFFER(ep0_buf_in, EP0_IN_BUF_SIZE);
    DECLARE_BUFFER(ep1_buf_in, EP1_IN_BUF_SIZE);
    DECLARE_BUFFER(ep2_buf_out, EP2_OUT_BUF_SIZE);
DECLARE_BUFFER_END

const device_descr_t devdsc __attribute__((space(auto_psv))) = {
    sizeof(device_descr_t),
    1,
    0x0110,
    0,
    0,
    0,
    64,
    0x1111,
    0x2222,
    0x0100,
    0,
    0,
    0,
    1
};

const struct config {
    config_descriptor_t config_descr;
    interf_descriptor_t interf_descr;
    endpoint_descriptor_t endp_descr1;
} CONFIG __attribute__((space(auto_psv))) = {
    {
        sizeof(config_descriptor_t),
        2,               // bDescriptorType
        sizeof(CONFIG),
        1,               // bNumInterfaces
        0x01,            // bConfigurationValue
        0,               // iConfiguration
        0x80,            // bmAttributes
        50               // bMaxPower
    },
    {
        sizeof(interf_descriptor_t),
        4,               // bDescriptorType
        0,               // bInterfaceNumber
        0x0,             // bAlternateSetting
        3,               // bNumEndpoints
        0xFF,            // bInterfaceClass
        0x0,            // bInterfaceSubClass
        0x0,            // bInterfaceProtocol
        0                // iInterface
    },
    {
        sizeof(endpoint_descriptor_t),
        5,               // bDescriptorType
        0x81,             // bEndpointAddress
        3,               // bmAttributes
        10,              // wMaxPacketSize
        1                // bInterval
    },
	{
        sizeof(endpoint_descriptor_t),
        5,               // bDescriptorType
        0x02,             // bEndpointAddress
        3,               // bmAttributes
        10,              // wMaxPacketSize
        1                // bInterval
    },
};

int trn_state = 0;
int dev_adr;


void process_control_transfer(int ep_num){
    usb_device_req_t req;
	int setup = is_setup(ep_num, EP(ep0));
	if (setup) {
	    log_str("setup package\n");
		copy_from_buffer(ep0_buf_out, &req, sizeof(req));
		log_int("rt: %lx\n", req.bmRequestType);
		log_int("rr: %lx\n", req.bRequest);
		log_int("rv: %lx\n", req.wValue);
		log_int("ri: %lx\n", req.wIndex);
		log_int("rl: %lx\n", req.wLength);
		if (req.bRequest == 6 && ((req.wValue>>8) & 0xFF) == 1){
			copy_to_buffer(ep0_buf_in, &devdsc, sizeof(devdsc));
			usb_ep_transf_start(&__bdt.ep0, USB_TRN_DATA1_IN, ep0_buf_in, sizeof(devdsc));
			trn_state = 1;
		}
		if (req.bRequest == 6 && ((req.wValue>>8) & 0xFF) == 2){
			copy_to_buffer(ep0_buf_in, &CONFIG, min(req.wLength, sizeof(CONFIG)));
			usb_ep_transf_start(&__bdt.ep0, USB_TRN_DATA1_IN, ep0_buf_in, min(req.wLength, sizeof(CONFIG)));
			trn_state = 1;
		}
		if (req.bRequest == 5){
			dev_adr = req.wValue;
			usb_ep_transf_start(&__bdt.ep0, USB_TRN_DATA1_IN, ep0_buf_in, 0);
			trn_state = 3;
		}
     	return;
		}
	
	switch (trn_state) {
		case 1: usb_ep_transf_start(&__bdt.ep0, USB_TRN_DATA1_OUT, ep0_buf_out, 0); trn_state = 2; break;
		case 2: usb_ep_transf_start(&__bdt.ep0, USB_TRN_SETUP, ep0_buf_out, EP0_OUT_BUF_SIZE); trn_state = 0; break;
		case 3: usb_set_address(dev_adr); usb_ep_transf_start(&__bdt.ep0, USB_TRN_SETUP, ep0_buf_out, EP0_OUT_BUF_SIZE); trn_state = 0; break;
	}
}

int ep1_data = 0;
int ep2_data = 0;
char buf[11];


void process_ep_transfer(int ep_num, int ep_dir) {
	switch (ep_num) {
		case 0x81: 
		case 0x02: copy_from_buffer(ep2_buf_out, buf, EP(ep2)->cnt); buf[EP(ep2)->cnt] = '\0'; log_str(buf); 
		usb_ep_transf_start(&__bdt.ep2, USB_TRN_DATA0_OUT, ep2_buf_out, EP2_OUT_BUF_SIZE); break;
	}
    
}


int main() {
	// inicializujte USB podsyst�m
	cpu_init();
	disp_init();
	touchpad_init();
	log_init();
	led_init();
	log_str("starting\n");
 	//while(1) log_main_loop();
	usb_init();
	// vy�kejte dokud nen� za��zen� p�ipojeno k PC (attached)
	//while (!is_attached());
	// zapn�te USB podsyst�m
	usb_enable();
	// indikujte stav attached
	devstate device_state;
	device_state = ATTACHED;
	// po�kejte dokud se USB subsyst�m nedostane do stavu powered
	while (!is_powered());
	// indikujte stav powered
	device_state = POWERED;
	log_str("powered\n");
	while (1) {
		log_main_loop();
        // testujte USB reset
	    if (is_reset()) {
			{
			log_str("reset\n");
	    	}
	        usb_reset();
	        // inicializujte endpoint 0 (p��padn� dal�� endpointy)
	        usb_init_ep(0, EP_SETUP_INOUT, &__bdt.ep0);
			usb_init_ep(1, EP_SETUP_INOUT, &__bdt.ep1);
			usb_init_ep(2, EP_SETUP_INOUT, &__bdt.ep2);
		    // nastartujte p�enos p�es endpoint 0 (dopl�te)
			usb_ep_transf_start(&__bdt.ep0, USB_TRN_SETUP, ep0_buf_out, EP0_OUT_BUF_SIZE);
            usb_ep_transf_start(&__bdt.ep2, USB_TRN_DATA0_OUT, ep2_buf_out, EP2_OUT_BUF_SIZE);
			ep1_data = 0;
			ep2_data = 0;
	        // po resetu p�ejd�te do stavu DEFAULT
	        device_state = DEFAULT;
	    }
	    // testujte na dokon�en� p�enosu
	    if (is_transfer_done()) {
			log_str("huraa\n");
	        // zjist�te ��slo koncov�ho bodu, kde do�lo k dokon�en� p�enosu
	        int ep_num = get_trn_status();
	        // detekujte ��d�c� p�enos (control transfer)
	        if (ep_num == 0 || ep_num == 0x80) {
	            // volejte funkci, kde obslou��te ��d�c� p�enosy
	            process_control_transfer(ep_num);
	        }
			else process_ep_transfer(ep_num);
		} 

	}
	return 0;
}