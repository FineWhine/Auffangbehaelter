#include <Arduino.h>
#include <Stepper.h>
#include<Wire.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>
#include<Adafruit_I2CDevice.h>
#include <EEPROM.h>

/******************************************/
//        Constant declarations
/******************************************/

//Interrupt Pins: 2, 3, 18, 19, 20, 21
#define INTERRUPT_1 2
#define INTERRUPT_2 3
#define INTERRUPT_3 18
#define INTERRUPT_4 19

//Stepper settings
#define STEPS 2070
#define STEPPER_SPEED 7

#define TOTALPARTS_ADDRESS 0
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels


/******************************************/
//        Variable declarations
/******************************************/

//Steppers
Stepper* stepper1 = NULL;
Stepper* stepper2 = NULL;

//Flags for interrupts
volatile bool interrupt1;
volatile bool interrupt2;
volatile bool interrupt3;
volatile bool interrupt4;

//Part_Counter
uint16_t partsInBox;
uint32_t totalParts;

//Display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/******************************************/
//        Function declarations
/******************************************/

void ISR1();
void ISR2();
void ISR3();
void ISR4();
void partIncoming(volatile bool& interrupt);
void resetBoxCounter(volatile bool& interrupt);
void resetTotalParts(volatile bool& interrupt);
void countPart(volatile bool& interrupt);
void disableSteppers(int stepper_num);



void setup() {

  //Initialize Serial Connection
  Serial.begin(115200);

  //Set Part Counter since reboot
  partsInBox = 0;

  /******************************************/
  //        Setup Display Stuff
  /******************************************/
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(1000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Hello, Worker 1!");
  display.display(); 
  delay(1000);

  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Total Parts: ");
  display.println(EEPROM.read(TOTALPARTS_ADDRESS));
  display.print("Parts in Box: ");
  display.println(partsInBox);
  display.display();

  /******************************************/
  //        Setup Button interrupt
  /******************************************/

  //Setting up interrupt Pins
  pinMode(INTERRUPT_1, INPUT);
  pinMode(INTERRUPT_2, INPUT);
  pinMode(INTERRUPT_3, INPUT);
  pinMode(INTERRUPT_4, INPUT);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_1), ISR1, RISING);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_2), ISR2, RISING);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_3), ISR3, RISING);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_4), ISR4, RISING);

  //Setting flags to false
  interrupt1 = false;
  interrupt2 = false;
  interrupt3 = false;
  interrupt4 = false;


  /******************************************/
  //        Setup Stepper stuff
  /******************************************/

  //Setting up stepper objects
  stepper1 = new Stepper(STEPS, 8, 10, 9, 11);
  stepper2 = new Stepper(STEPS, 4, 6, 5, 7);

  //Set Stepper speed
  stepper1->setSpeed(STEPPER_SPEED);
  stepper2->setSpeed(STEPPER_SPEED);

}

void loop() {

  if(interrupt1 == true)
  {
    countPart(interrupt1);
  }
  if(interrupt2 == true)
  {
    resetBoxCounter(interrupt2);
  }
  if(interrupt3 == true)
  {
    resetTotalParts(interrupt3);
  }
  if(interrupt4 == true)
  {
    partIncoming(interrupt4);
  }


}


void ISR1(){ interrupt1 = true; }
void ISR2(){ interrupt2 = true; }
void ISR3(){ interrupt3 = true; }
void ISR4(){ interrupt4 = true; }

void partIncoming(volatile bool& interrupt)
{
  stepper1->step(-(STEPS/8));
  delay(1000);
  stepper1->step(STEPS/8);
  disableSteppers(1);
  stepper2->step(STEPS/8);
  disableSteppers(2);

  interrupt = false;
}

void resetBoxCounter(volatile bool& interrupt)
{
  partsInBox = 0;
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Total Parts: ");
  display.println(EEPROM.read(TOTALPARTS_ADDRESS));
  display.print("Parts in Box: ");
  display.println(partsInBox);
  display.display();

  interrupt = false;
}

void resetTotalParts(volatile bool& interrupt)
{
  EEPROM.update(TOTALPARTS_ADDRESS, 0);

  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Total Parts: ");
  display.println(EEPROM.read(TOTALPARTS_ADDRESS));
  display.print("Parts in Box: ");
  display.println(partsInBox);
  display.display();

  interrupt3 = false;
}

void countPart(volatile bool& interrupt)
{

  partsInBox++;
  EEPROM.update(TOTALPARTS_ADDRESS, (EEPROM.read(TOTALPARTS_ADDRESS)+1));

  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Total Parts: ");
  display.println(EEPROM.read(TOTALPARTS_ADDRESS));
  display.print("Parts in Box: ");
  display.println(partsInBox);
  display.display();

  delay(200);

  interrupt = false;

}

void disableSteppers(int stepper_num)
{
  switch(stepper_num){
    case(2):
      digitalWrite(4, LOW);
      digitalWrite(5, LOW);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
      break;
    case(1):
      digitalWrite(8, LOW);
      digitalWrite(9, LOW);
      digitalWrite(10, LOW);
      digitalWrite(11, LOW);
      break;
  }
}