//Base header files for understanding the e-puck robot
#include "p30F6014A.h"
#include "DataEEPROM.h"
#include "stdio.h"
#include "string.h"

//Header files to access robot functionality
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
    //camera-based variables
  int cameraMode, cameraWidth, cameraHeight, cameraZoom, cameraSize;
  int loopVar;        //loop variable for 'for' loops
  int stateLoop = 1 ; //
  int domStep   = 10; //strength factor of the dominant wheel
  int subStep   = 1 ; //strength factor of the weaker   wheel

    //variables for processing the image information
  unsigned char *bufferPointer, pixel, lightest;
  unsigned int left, right, lightPos;

  /*read HW version from the eeprom (last word)*/
  int HWversion=0xFFFF;
  int temp = 0;
  temp = ReadEE(0x7F,0xFFFE,&HWversion, 1);
  temp = temp & 0x03; // get the camera rotation from the HWversion byte

  //camera configuration
  cameraMode=GREY_SCALE_MODE;        //greyscale to improve performance
  if ((temp == 3) || (temp == 0) ) { //if the camera is mounted sideways
    cameraWidth  = 1 ;
    cameraHeight = 60;
  } else {                           //if the camera is mounted correctly
    cameraWidth  = 60;
    cameraHeight = 1 ;
  }
  
  cameraZoom = 8;                          //the zoom level of the camera
  cameraSize = cameraWidth * cameraHeight; //pixels that the camera covers
  e_poxxxx_init_cam();                     //initialise the camera

  //configure the camera using defined variables
  e_poxxxx_config_cam( (ARRAY_WIDTH - cameraWidth * cameraZoom) / 2, (ARRAY_HEIGHT - cameraHeight * cameraZoom) / 2, cameraWidth * cameraZoom, cameraHeight * cameraZoom, cameraZoom, cameraZoom, cameraMode);
  
  e_poxxxx_set_mirror(1, 1);      //set the mirror level of the camera
  e_poxxxx_write_cam_registers(); //save the camera configuration

  //move clockwise to begin with
  e_set_speed_left( domStep * (10) );
  e_set_speed_right(subStep * (10) );
      
while(stateLoop) { //while loop forever
    e_poxxxx_launch_capture(&buffer[0] ); // start camera and store in buffer
    
    e_set_body_led(1); //turn on body   LED
    e_set_led(4,   1); //turn on bottom LEDs

    while(!e_poxxxx_is_img_ready() );  // wait until image is captured
    
    bufferPointer = (unsigned char*)&buffer[0]; //set pointer to the start

      //reset the processing variables for the current iteration
    left     = 0;
    right    = 0;
    lightPos = 0;
    lightest = 0;
    
    for (loopVar = 0; loopVar < 30; loopVar++) {  //left side of the image
        //access the correct pixel
      pixel *= bufferPointer;
      bufferPointer++;
      left  += pixel ;

      //if the pixel is lighter than the current highest value
      if (pixel > lightest) {
        lightest = pixel  ; //re-define the lightest pixel
        lightPos = loopVar; //set the position of the lightest
      }
    }
    
    for (; loopVar < cameraHeight; loopVar++) { //right side of the image
        //access the correct pixel
      pixel =* bufferPointer;
      bufferPointer++;
      right += pixel ;

        //if the pixel is lighter than the current highest value
      if (pixel > lightest) {
        lightest = pixel  ; //re-define the lightest pixel
        lightPos = loopVar; //set the position of the lightest
      }
    }

      //if the camera is rotated 0 or 90 degrees
    if ( (temp == 3) || (temp == 2 ) ) {
      //move counter clockwise away from the lightest pixel
      e_set_speed_left( subStep * (lightPos - 30) );
      e_set_speed_right(domStep * (lightPos - 30) );

    } else {
      //move clockwise away from the lightest pixel
      e_set_speed_left( domStep * (lightPos - 30) );
      e_set_speed_right(subStep * (lightPos - 30) );
    }

  }
  return 0;
  
}
