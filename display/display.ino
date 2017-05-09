#include <UTFT.h>

UTFT myGLCD(9,3,4,6,8, 9);

extern uint8_t SmallFont[];

void setup() {
  // put your setup code here, to run once:
  myGLCD.InitLCD(PORTRAIT);
  myGLCD.setFont(SmallFont);
}

void loop() {
  // put your main code here, to run repeatedly:
  //myGLCD.clrScr();
  myGLCD.setContrast(64);

  myGLCD.setColor(255, 0, 0);
  myGLCD.fillRect(0,0,127,12); 
  myGLCD.print("Ana are mere", 0,20, 0);
  

  
}
