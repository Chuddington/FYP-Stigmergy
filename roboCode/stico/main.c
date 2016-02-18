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
  //move in a circle
  //while loop forever
  while(stateLoop) {
    leftMotor.setSpeed( motorAStrength);
    rightMotor.setSpeed(motorBStrength);
    leftMotor.forward() ;
    rightMotor.forward();

    //place light-based pheromones
    LED light  = new LED(LED_POS)   ;
    Camera cam = new Camera(CAM_POS);

    light.setStrength(LedStrength);
    //use camera to check for pheromones
    Image i = cam.getImage();
    //if statement utilising the camera check
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
