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

// Délky bufferù
#define EP0_OUT_BUF_SIZE 64
#define EP0_IN_BUF_SIZE 64


// Deklarace endpointù
DECLARE_EP_BEGIN
    DECLARE_EP(ep0);   // IN/OUT Control EP
DECLARE_EP_END

// Deklarace bufferù
DECLARE_BUFFER_BEGIN
    DECLARE_BUFFER(ep0_buf_out, EP0_OUT_BUF_SIZE);
    DECLARE_BUFFER(ep0_buf_in, EP0_IN_BUF_SIZE);
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
    }
};

int trn_state = 0;
int dev_adr;

/*int max (int x, int y){
	int m = x;
	if (x < y) m = y;
	return m;
}*/

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


int main() {
	// inicializujte USB podsystém
	cpu_init();
	disp_init();
	touchpad_init();
	log_init();
	led_init();
	log_str("starting\n");
 	//while(1) log_main_loop();
	usb_init();
	// vyèkejte dokud není zaøízení pøipojeno k PC (attached)
	//while (!is_attached());
	// zapnìte USB podsystém
	usb_enable();
	// indikujte stav attached
	devstate device_state;
	device_state = ATTACHED;
	// poèkejte dokud se USB subsystém nedostane do stavu powered
	while (!is_powered());
	// indikujte stav powered
	device_state = POWERED;
	log_str("powered\n");
	//while(1) log_main_loop();
    while (1) {
		log_main_loop();
        // testujte USB reset
	    if (is_reset()) {
			{
			log_str("reset\n");
	    	}
	        usb_reset();
	        // inicializujte endpoint 0 (pøípadnì další endpointy)
	        usb_init_ep(0, EP_SETUP_INOUT, &__bdt.ep0);
		    // nastartujte pøenos pøes endpoint 0 (doplòte)
			usb_ep_transf_start(&__bdt.ep0, USB_TRN_SETUP, ep0_buf_out, EP0_OUT_BUF_SIZE);
            
	        // po resetu pøejdìte do stavu DEFAULT
	        device_state = DEFAULT;
	    }
	    // testujte na dokonèení pøenosu
	    if (is_transfer_done()) {
			log_str("huraa\n");
	        // zjistìte èíslo koncového bodu, kde došlo k dokonèení pøenosu
	        int ep_num = get_trn_status();
	        // detekujte øídící pøenos (control transfer)
	        if (ep_num == 0 || ep_num == 0x80) {
	            // volejte funkci, kde obsloužíte øídící pøenosy
	            process_control_transfer(ep_num);
	        }
	        //zpracujte pøenosy na ostatní koncové body
	        //process_ep_transfer(ep_num);
		} 

	}
	return 0;
}