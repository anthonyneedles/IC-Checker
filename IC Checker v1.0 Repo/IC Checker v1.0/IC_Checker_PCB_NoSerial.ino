/*************************************************************
 * IC_Checker Arduino Program
 * Checks IC fidelity by brute-force comparing input/outputs 
 * versus pre-programmed parameters. 
 ************************************************************/

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

//(IC input pins)
#define IC_PIN_1 2
#define IC_PIN_2 3
#define IC_PIN_3 4
#define IC_PIN_4 5
#define IC_PIN_5 6
#define IC_PIN_6 7
#define IC_PIN_8 8
#define IC_PIN_9 9
#define IC_PIN_10 10
#define IC_PIN_11 11
#define IC_PIN_12 12
#define IC_PIN_13 13
#define SWITCH_INPUT_PIN 0

//Bitmasks for passed test of certain ICs   
#define NOT_PASSED 0x00
#define PASSED_00 0x01
#define PASSED_02 0x02
#define PASSED_04 0x04
#define PASSED_08 0x08
#define PASSED_10 0x10
#define PASSED_20 0x20
#define PASSED_27 0x40
#define PASSED_86 0x80

//Value in ms to wait after writing the Arduino output pins
#define WRITE_DELAY 1
#define LCD_I2C_ADDR 0x27
#define BUTTON_THRESH 1000
#define VERF_REPEATS 10
#define POSITIVE 1

const int Input00[] = {IC_PIN_1, IC_PIN_2, IC_PIN_4, IC_PIN_5, IC_PIN_9, IC_PIN_10, IC_PIN_12, IC_PIN_13}; //Input and output pin # arrays (for the IC)
const int Output00[] = {IC_PIN_3, IC_PIN_6, IC_PIN_8, IC_PIN_11};
const int Input02[] = {IC_PIN_2, IC_PIN_3, IC_PIN_5, IC_PIN_6, IC_PIN_8, IC_PIN_9, IC_PIN_11, IC_PIN_12}; //Input and output pin # arrays (for the IC)
const int Output02[] = {IC_PIN_1, IC_PIN_4, IC_PIN_10, IC_PIN_13};
const int Input04[] = {IC_PIN_1, IC_PIN_3, IC_PIN_5, IC_PIN_9, IC_PIN_11, IC_PIN_13};   //Input and output pin # arrays
const int Output04[] = {IC_PIN_2, IC_PIN_4, IC_PIN_6, IC_PIN_8, IC_PIN_10, IC_PIN_12};
const int Input08[] = {IC_PIN_1, IC_PIN_2, IC_PIN_4, IC_PIN_5, IC_PIN_9, IC_PIN_10, IC_PIN_12, IC_PIN_13}; //Input and output pin # arrays
const int Output08[] = {IC_PIN_3, IC_PIN_6, IC_PIN_8, IC_PIN_11};
const int Input10[] = {IC_PIN_1, IC_PIN_2, IC_PIN_13, IC_PIN_3, IC_PIN_4, IC_PIN_5, IC_PIN_9, IC_PIN_10, IC_PIN_11}; //Input and output pin # arrays3
const int Output10[] = {IC_PIN_12, IC_PIN_6, IC_PIN_8};
const int Input20[] = {IC_PIN_1, IC_PIN_2, IC_PIN_4, IC_PIN_5, IC_PIN_9, IC_PIN_10, IC_PIN_12, IC_PIN_13}; //Input and output pin # arrays
const int Output20[] = {IC_PIN_6, IC_PIN_8};
const int Input27[] = {IC_PIN_1, IC_PIN_2, IC_PIN_13, IC_PIN_3, IC_PIN_4, IC_PIN_5, IC_PIN_9, IC_PIN_10, IC_PIN_11}; //Input and output pin # arrays
const int Output27[] = {IC_PIN_12, IC_PIN_6, IC_PIN_8};
const int Input86[] = {IC_PIN_1, IC_PIN_2, IC_PIN_4, IC_PIN_5, IC_PIN_9, IC_PIN_10, IC_PIN_12, IC_PIN_13}; //Input and output pin # arrays
const int Output86[] = {IC_PIN_3, IC_PIN_6, IC_PIN_8, IC_PIN_11}; 

const String Passed00 = "Pass for 74HC00";
const String Passed02 = "Pass for 74HC02";
const String Passed04 = "Pass for 74HC04";
const String Passed08 = "Pass for 74HC08";
const String Passed10 = "Pass for 74HC10";
const String Passed20 = "Pass for 74HC20";
const String Passed27 = "Pass for 74HC27";
const String Passed86 = "Pass for 74HC86";
const String NotPassed = "No IC pass";
const String MultiPassed = "ERROR: MULTIPASS";
//const String SemiPassed1 = "ERROR: VERF FAIL";
//const String SemiPassed2 = "REPEATED TO: ";

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

int RepeatCount;

void setup() {   
  Serial.begin(9600);
  lcd.begin(16,2); 
  lcd.print(Passed02);
}

void loop() {
  int test_pass = 0;
  
  while(analogRead(SWITCH_INPUT_PIN) < BUTTON_THRESH);    //block until button press
  
  test_pass = (check00() | check02() | check04() | check08() | check10() | check20() | check27() | check86()); //Comes out with count of repeats
  
  switch(test_pass){
    case NOT_PASSED:
      lcd.clear();    
      lcd.print(NotPassed);
      break;
    case PASSED_00:
      lcd.clear();    
      lcd.print(Passed00);
      break;
    case PASSED_02:
      lcd.clear();    
      lcd.print(Passed02);
      break;
    case PASSED_04:
      lcd.clear();    
      lcd.print(Passed04);
      break;
    case PASSED_08:
      lcd.clear();    
      lcd.print(Passed08);
      break;
    case PASSED_10:
      lcd.clear();    
      lcd.print(Passed10);
      break;
    case PASSED_20:
      lcd.clear();    
      lcd.print(Passed20);
      break;
    case PASSED_27:
      lcd.clear();    
      lcd.print(Passed27);
      break;
    case PASSED_86:
      lcd.clear();    
      lcd.print(Passed86);
      break;
    default:
      lcd.clear();    
      lcd.print(MultiPassed);
      break;  
}
}

int check00() { //Run to check for a 74HC00 
  int test_output;
  int gate_start_index;

  for (int input_pin_num = 0; input_pin_num < 8; input_pin_num++) { //Pinmode for ardunio board
    pinMode(Input00[input_pin_num], OUTPUT);
  }

  for (int output_pin_num = 0; output_pin_num < 4; output_pin_num++) {
    pinMode(Output00[output_pin_num], INPUT);
  }
    
  RepeatCount = 0;
  
  for(RepeatCount; RepeatCount < VERF_REPEATS; RepeatCount++){
    gate_start_index = 0;
    
    for (int gate_num = 0; gate_num < 4; gate_num++) { //gate_num loop is ALWAYS for each gate
      for (int gate_input_A = 0; gate_input_A < 2; gate_input_A++) { //gate_input_A, j, n, m loops are for inputs to each gate
        for (int gate_input_B = 0; gate_input_B < 2; gate_input_B++) {
          digitalWrite(Input00[gate_start_index], gate_input_A);
          digitalWrite(Input00[gate_start_index + 1], gate_input_B);
          
          delay(WRITE_DELAY);
          
          test_output = digitalRead(Output00[gate_num]);
          
          if ((gate_input_A&&gate_input_B) != !test_output){
            return NOT_PASSED;
          }
        }
      }
    gate_start_index += 2;  
    }
  }
  return PASSED_00;
}

int check02() { //Run to check for a 74HC02 
  int test_output;
  int gate_start_index;
  
  for (int input_pin_num = 0; input_pin_num <8; input_pin_num++) { //Pinmode for ardunio board
    pinMode(Input02[input_pin_num], OUTPUT);
  }

  for (int output_pin_num = 0; output_pin_num < 4; output_pin_num++) {
    pinMode(Output02[output_pin_num], INPUT);
  }

  RepeatCount = 0;

  for(RepeatCount; RepeatCount < VERF_REPEATS; RepeatCount++){
    gate_start_index = 0;
    
    for (int gate_num = 0; gate_num < 4; gate_num++) {      
      for (int gate_input_A = 0; gate_input_A < 2; gate_input_A++) {
        for (int gate_input_B  = 0; gate_input_B < 2; gate_input_B++) {
          digitalWrite(Input02[gate_start_index], gate_input_A);
          digitalWrite(Input02[gate_start_index + 1], gate_input_B);
          
          delay(WRITE_DELAY);
          
          test_output = digitalRead(Output02[gate_num]);
          
          if ((gate_input_A||gate_input_B)!= !test_output){
            return NOT_PASSED;
          }
        }
      }
    gate_start_index += 2;  
    }
  }
  return PASSED_02;
}

int check04() { //Run to check for a 74HC04
  int test_output;
  int gate_start_index;

  for (int input_pin_num = 0; input_pin_num < 6; input_pin_num++) { //Pinmode for ardunio board
    pinMode(Input04[input_pin_num], OUTPUT);
  }

  for (int output_pin_num = 0; output_pin_num < 6; output_pin_num++) {
    pinMode(Output04[output_pin_num], INPUT);
  }

  RepeatCount = 0;
  
  for(RepeatCount; RepeatCount < VERF_REPEATS; RepeatCount++){
    for (int gate_num = 0; gate_num < 6; gate_num++) {
      for (int gate_input_A = 0; gate_input_A < 2; gate_input_A++) {
        digitalWrite(Input04[gate_num], gate_input_A);
        
        delay(WRITE_DELAY);
        
        test_output = digitalRead(Output04[gate_num]);
        
        if (test_output == gate_input_A) {
          return NOT_PASSED;
        }    
      }
    }
  }
  return PASSED_04;
}

int check08() { //Run to check for a 74HC08
  int test_output;
  int gate_start_index;

  for (int input_pin_num = 0; input_pin_num < 8; input_pin_num++) { //Pinmode for ardunio board
    pinMode(Input08[input_pin_num], OUTPUT);
  }

  for (int output_pin_num = 0; output_pin_num < 4; output_pin_num++) {
    pinMode(Output08[output_pin_num], INPUT);
  }

  RepeatCount = 0;

  for(RepeatCount; RepeatCount < VERF_REPEATS; RepeatCount++){
    gate_start_index = 0; 
     
    for (int gate_num = 0; gate_num < 4; gate_num++) {
      for (int gate_input_A = 0; gate_input_A < 2; gate_input_A++) {
        for (int gate_input_B = 0; gate_input_B < 2; gate_input_B++) {
          digitalWrite(Input08[gate_start_index], gate_input_A);
          digitalWrite(Input08[gate_start_index + 1], gate_input_B);
          
          delay(WRITE_DELAY);
          
          test_output = digitalRead(Output08[gate_num]);
          
          if ((gate_input_A&&gate_input_B)!= test_output){
            return NOT_PASSED;
          }
        }
      }    
      gate_start_index += 2;
    }
  }
  return PASSED_08;
}

int check10() { //Run to check for a 74HC10
  int test_output;
  int gate_start_index;

  for (int input_pin_num = 0; input_pin_num < 9; input_pin_num++) { //Pinmode for ardunio board
    pinMode(Input10[input_pin_num], OUTPUT);
  }

  for (int output_pin_num = 0; output_pin_num < 3; output_pin_num++) {
    pinMode(Output10[output_pin_num], INPUT);
  }

  RepeatCount = 0;

  for(RepeatCount; RepeatCount < VERF_REPEATS; RepeatCount++){
    gate_start_index = 0;
      
    for (int gate_num = 0; gate_num < 3; gate_num++) {
      for (int gate_input_A = 0; gate_input_A < 2; gate_input_A++) {
        for (int gate_input_B = 0; gate_input_B < 2; gate_input_B++) {
          for (int gate_input_C = 0; gate_input_C < 2; gate_input_C++) {
            digitalWrite(Input10[gate_start_index], gate_input_A);
            digitalWrite(Input10[gate_start_index + 1], gate_input_B);
            digitalWrite(Input10[gate_start_index + 2], gate_input_C);
            
            delay(WRITE_DELAY);

            test_output = digitalRead(Output10[gate_num]);
            
            if ((gate_input_A&&gate_input_B&&gate_input_C)!= !test_output){
              return NOT_PASSED;
            }
          }
        }
      }      
      gate_start_index += 3;
    }
  }
  return PASSED_10;
}
  
int check20() { //Run to check for a 74HC20 
  int test_output;
  int gate_start_index;
  
  for (int input_pin_num = 0; input_pin_num < 8; input_pin_num++) { //Pinmode for ardunio board
    pinMode(Input20[input_pin_num], OUTPUT);
  }

  for (int output_pin_num = 0; output_pin_num < 2; output_pin_num++) {
    pinMode(Output20[output_pin_num], INPUT);
  }

  RepeatCount = 0;

  for(RepeatCount; RepeatCount < VERF_REPEATS; RepeatCount++){
    gate_start_index = 0;  
    
    for (int gate_num = 0; gate_num < 2; gate_num++) {
      for (int gate_input_A = 0; gate_input_A < 2; gate_input_A++) {
        for (int gate_input_B = 0; gate_input_B < 2; gate_input_B++) {
          for (int gate_input_C = 0; gate_input_C < 2; gate_input_C++) {
            for (int gate_input_D = 0; gate_input_D < 2; gate_input_D++) {
              digitalWrite(Input20[gate_start_index], gate_input_A);
              digitalWrite(Input20[gate_start_index + 1], gate_input_B);
              digitalWrite(Input20[gate_start_index + 2], gate_input_C);            
              digitalWrite(Input20[gate_start_index + 3], gate_input_D);
              
              delay(WRITE_DELAY);
              
              test_output = digitalRead(Output20[gate_num]);
              
              if ((gate_input_A&&gate_input_B&&gate_input_C&&gate_input_D)!= !test_output){
                return NOT_PASSED;
              }
            }
          }
        }
      }    
      gate_start_index += 4;
    }
  }
  return PASSED_20;
}

int check27() { //Run to check for a 74HC27
  int test_output;
  int gate_start_index;

  for (int input_pin_num = 0; input_pin_num < 9; input_pin_num++) { //Pinmode for ardunio board
    pinMode(Input27[input_pin_num], OUTPUT);
  }

  for (int output_pin_num = 0; output_pin_num < 3; output_pin_num++) {
    pinMode(Output27[output_pin_num], INPUT);
  }

  RepeatCount = 0;

  for(RepeatCount; RepeatCount < VERF_REPEATS; RepeatCount++){
    gate_start_index = 0;
    
    for (int gate_num = 0; gate_num < 3; gate_num++) {
      for (int gate_input_A = 0; gate_input_A < 2; gate_input_A++) {
        for (int gate_input_B = 0; gate_input_B < 2; gate_input_B++) {
          for (int gate_input_C = 0; gate_input_C < 2; gate_input_C++) {
            digitalWrite(Input27[gate_start_index], gate_input_A);
            digitalWrite(Input27[gate_start_index + 1], gate_input_B);
            digitalWrite(Input27[gate_start_index + 2], gate_input_C);
            
            delay(WRITE_DELAY);
            
            test_output = digitalRead(Output27[gate_num]);
            
            if ((gate_input_A||gate_input_B||gate_input_C)!= !test_output){
              return NOT_PASSED;
            }
          }
        }
      }      
      gate_start_index += 3;
    }
  }
  return PASSED_27;
}

int check86() { //Run to check for a 74HC86  
  int test_output;
  int gate_start_index;

  for (int input_pin_num = 0; input_pin_num < 8; input_pin_num++) { //Pinmode for ardunio board
    pinMode(Input86[input_pin_num], OUTPUT);
  }

  for (int output_pin_num = 0; output_pin_num < 4; output_pin_num++) {
    pinMode(Output86[output_pin_num], INPUT);
  }

  RepeatCount = 0;

  for(RepeatCount; RepeatCount < VERF_REPEATS; RepeatCount++){
    gate_start_index = 0;  
    
    for (int gate_num = 0; gate_num < 4; gate_num++) {
      for (int gate_input_A = 0; gate_input_A < 2; gate_input_A++) {
        for (int gate_input_B = 0; gate_input_B < 2; gate_input_B++) {
          digitalWrite(Input86[gate_start_index], gate_input_A);
          digitalWrite(Input86[gate_start_index + 1], gate_input_B);
          
          delay(WRITE_DELAY);
          
          test_output = digitalRead(Output86[gate_num]);
          
          if ((gate_input_A^gate_input_B)!= test_output){
            return NOT_PASSED;
          }
        }
      }      
      gate_start_index += 2;
    }
  }
  return PASSED_86;
}
