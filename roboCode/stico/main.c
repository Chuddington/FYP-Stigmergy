#include "p30F6014A.h"
#include "e_po3030k.h" //camera fast two timers header file
#include "stdio.h"
#include "string.h"

#include <motor_led/e_init_port.h>
#include <motor_led/advance_one_timer/e_led.h>
#include <motor_led/advance_one_timer/e_motors.h>
#include <motor_led/advance_one_timer/e_agenda.h>
#include <camera/fast_2_timer/e_poxxxx.h>
#include <uart/e_uart_char.h>
#include <a_d/advance_ad_scan/e_ad_conv.h>
#include <a_d/advance_ad_scan/e_prox.h>
#include <a_d/advance_ad_scan/e_acc.h>
#include <a_d/advance_ad_scan/e_micro.h>
#include "math.h"
#include <utility/utility.h>

#include "memory.h"
extern char buffer[BUFFER_SIZE];


int main() {
	int cam_mode, cam_width, cam_heigth, cam_zoom, cam_size;
	int i;
  int stateLoop = 1;
  int domStep   = 10;
  int subStep   = 1;
	unsigned char *buf_ptr, pixel, lightest;
	unsigned int left, right, lightPos;

	#include "DataEEPROM.h"
	/*read HW version from the eeprom (last word)*/
	int HWversion=0xFFFF;
	int temp = 0;
	temp = ReadEE(0x7F,0xFFFE,&HWversion, 1);
	temp = temp & 0x03;	// get the camera rotation from the HWversion byte

	/*Cam default parameter*/
	cam_mode=GREY_SCALE_MODE;
	if ((temp == 3) || (temp == 0) ) { // 0' and 180' camera rotation
		cam_width  = 1;
		cam_heigth = 60;
	} else {
		cam_width  = 60;
		cam_heigth = 1;
	}
	cam_zoom = 8;
	cam_size = cam_width * cam_heigth;
	e_poxxxx_init_cam();
	e_poxxxx_config_cam((ARRAY_WIDTH -cam_width*cam_zoom)/2,(ARRAY_HEIGHT-cam_heigth*cam_zoom)/2,cam_width*cam_zoom,cam_heigth*cam_zoom,cam_zoom,cam_zoom,cam_mode);
	e_poxxxx_set_mirror(1,1);
	e_poxxxx_write_cam_registers();


while(stateLoop) { //while loop forever
		e_poxxxx_launch_capture(&buffer[0] );	// start cam.
		e_led_clear();
		e_set_body_led(0);
		e_set_front_led(0);
    e_set_led(4,1);

  	while(!e_poxxxx_is_img_ready());	// wait until image is captured
		buf_ptr = (unsigned char*)&buffer[0];
		left     = 0;
    right    = 0;
    lightPos = 0;
    lightest = 0;
		for (i=0; i<30; i++) {	//left
			pixel =* buf_ptr;
			buf_ptr++;
			left += pixel;
			if (pixel > lightest) {
				lightest = pixel;
				lightPos = i;
			}
		}
		
    for (; i < cam_heigth; i++) {	//right
			pixel =* buf_ptr;
			buf_ptr++;
			right += pixel;
			if (pixel > lightest) {
				lightest = pixel;
				lightPos = i;
			}
		}
		
    if (lightPos < 20) {	//led on at lightest position
			e_set_led(7,1);
    }	else if (lightPos < 40) {
			e_set_led(0,1);
    }	else {
			e_set_led(1,1); 
    }

		if ( (temp == 3) || (temp == 2 ) ) { // 0' and 90' camera rotation
			e_set_speed_left( domStep * (lightPos - 30) );  // motor speed in steps/s
			e_set_speed_right(subStep * (lightPos - 30) );
		} else {
			e_set_speed_left( subStep * (lightPos - 30) );  // motor speed in steps/s
			e_set_speed_right(domStep * (lightPos - 30) );
		}

		sprintf(buffer, "left %u, right %u, lightest %u, lightPos %u\r\n", left, right, lightest, lightPos);
		e_send_uart1_char(buffer, strlen(buffer));

  }
  
  return 0;
}
