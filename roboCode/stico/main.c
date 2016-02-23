#include "e_po3030k.h" //camera fast two timers header file


int main() {
  int stateLoop = 1; //variable controlling global while loop
  int motorAStrength = 80 ; //originally left  motor strength level
  int motorBStrength = 20 ; //originally right motor strength level
  int motorTStrength = 0  ; //variable stores strengths when swapping values
  int LedStrength    = 100; //stores the strength of the LED light on the robot
  int lightLimit     = 30 ; //variable to store conditional light level

  //placeholder variables (probably written in Java for the moment)
  Motor leftMotor  = new Motor(leftSide );
  Motor rightMotor = new Motor(rightSide);


  /* Basic StiCo implementation */
  //prepare the camera for use:
  e_po3030k_init_cam(); //expose camera interfaces
  e_po3030k_config_cam((ARRAY_WIDTH -160)/2,(ARRAY_HEIGHT-160)/2,
		       160,160,4,4,GREY_SCALE_MODE); //40x40 greyscale image
  e_po3030k_write_cam_registers(); //save the configuration in an array

  //prepare the LED for use:

  //move in a circle
  //while loop forever
  while(stateLoop) {
    leftMotor.setSpeed( motorAStrength);
    rightMotor.setSpeed(motorBStrength);
    leftMotor.forward() ;
    rightMotor.forward();

    //place light-based pheromones
    LED light  = new LED(LED_POS)   ;

    light.setStrength(LedStrength);
    //use camera to check for pheromones
    e_po3030k_launch_capture(buffer);
    while(!e_po3030k_is_img_ready() );
    //if statement utilising the image
    if(i.lightLevel() > lightLimit) {
      //swap motor strengths if true; nothing otherwise
      motorTStrength = motorAStrength;
      motorAStrength = motorBStrength;
      motorBStrength = motorTStrength;
    } else {
    }
  }
  
  return 0;
}
