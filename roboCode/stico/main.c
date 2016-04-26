// Header files for robot functionality
#include "p30F6014A.h"
#include "DataEEPROM.h"
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


extern char buffer[BUFFER_SIZE]; //stores image information


int main() {
  //variable declarations
  int cameraMode, cameraWidth, cameraHeight, cameraZoom, cameraSize;
  int i;
  int stateLoop = 1;
  int domStep   = 10;
  int subStep   = 1;
  unsigned char *bufferPointer, pixel, lightest;
  unsigned int left, right, lightPos;

  /*read HW version from the eeprom (last word)*/
  int HWversion=0xFFFF;
  int temp = 0;
  temp = ReadEE(0x7F,0xFFFE,&HWversion, 1);
  temp = temp & 0x03; // get the camera rotation from the HWversion byte

  //camera configuration
  cameraMode=GREY_SCALE_MODE;
  if ((temp == 3) || (temp == 0) ) { // 0' and 180' camera rotation
    cameraWidth  = 1;
    cameraHeight = 60;
  } else {
    cameraWidth  = 60;
    cameraHeight = 1;
  }
  cameraZoom = 8;
  cameraSize = cameraWidth * cameraHeight;
  e_poxxxx_init_cam();
  e_poxxxx_config_cam((ARRAY_WIDTH -cameraWidth*cameraZoom)/2,(ARRAY_HEIGHT-cameraHeight*cameraZoom)/2,cameraWidth*cameraZoom,cameraHeight*cameraZoom,cameraZoom,cameraZoom,cameraMode);
  e_poxxxx_set_mirror(1,1);
  e_poxxxx_write_cam_registers();


while(stateLoop) { //while loop forever
    e_poxxxx_launch_capture(&buffer[0] ); // start cam.
    e_led_clear();
    e_set_body_led(0);
    e_set_front_led(0);
    e_set_led(4,1);

    while(!e_poxxxx_is_img_ready());  // wait until image is captured
    bufferPointer = (unsigned char*)&buffer[0];
    left     = 0;
    right    = 0;
    lightPos = 0;
    lightest = 0;
    for (i=0; i<30; i++) {  //left
      pixel =* bufferPointer;
      bufferPointer++;
      left += pixel;
      if (pixel > lightest) {
        lightest = pixel;
        lightPos = i;
      }
    }
    
    for (; i < cameraHeight; i++) { //right
      pixel =* bufferPointer;
      bufferPointer++;
      right += pixel;
      if (pixel > lightest) {
        lightest = pixel;
        lightPos = i;
      }
    }
    
    if (lightPos < 20) {  //led on at lightest position
      e_set_led(7,1);
    } else if (lightPos < 40) {
      e_set_led(0,1);
    } else {
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
