//Testing on temperature sensing and LCD display
#include <LiquidCrystal.h>    //LCD library
#include <OneWire.h>          //sensing library
#include <DallasTemperature.h>//sensing library
#include <IRremote.h>         //IR remote control library

//Temperature sensing initialization
#define ONE_WIRE_BUS 6 //sigital sensor is connected with Pin D6
OneWire oneWire (ONE_WIRE_BUS);
DallasTemperature sensors (&oneWire);
//LCD intialization
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);//LCD object to control LCD

int settemp = 23;    //Initialize temperature setting to 23 degree
int mode = 9;        //Initialize Mode to be OFF
int fan = 0;         //Initialize fan to be OFF
int screen = 0;      //Initialize screen to 0
int timer = 0;       //Initialize timer for timing mode, change fan each second

const int EEPin[3] = {8,9,10};    //set Communication Pins Array to 8,9,10
const int CurrentPin = 2;    //set DC current reading A2
int data[10] = {0};       //variable to store the value read
int index = 0;       //variable to store the times of reading
float finalcurrent = 0; //variable to store final current value calculated in 10 cycles

//IR remote setting 
const int receiver = 13;          //set IR remote Control Pin to Pin 13
IRrecv irrecv(receiver);           // create instance of 'irrecv'
decode_results results;            // create instance of 'decode_results'

void setup()
{
  //Setup LCD: Turnoff & screen0 defalut
  lcd.begin(40,7); //setup LCD's number of rows and cols
  lcd.setCursor(0,0);
  lcd.print("ModeNow: OFF");
  lcd.setCursor(-4,1);
  lcd.print("mTemp:0   mFan:1");
  lcd.setCursor(-4,2);
  lcd.print("aTime:2 aLight:3");
  lcd.setCursor(-4,3);
  lcd.print("aSave:4  aTemp:5");
  
  //Setup serial
  Serial.begin(9600);
  sensors.begin();      //start sensor
  irrecv.enableIRIn();  // Start the IR receiver
  
}


void loop()
{
  //Read DC current of System Current Transducer
  ReadDCCurrent();
  //set pin mode to communicate with EE
  pinMode (EEPin[0], OUTPUT);
  pinMode (EEPin[1], OUTPUT);
  pinMode (EEPin[2], OUTPUT);
  
  //get temperature of the environment
  sensors.requestTemperatures();
  float currenttemp = sensors.getTempCByIndex(0);
  
  //IR Remote control and command operation with EE
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    translateIR(mode, currenttemp); 
    irrecv.resume(); // receive the next value
  }
  
  //Timer motivation
  if(mode==2)
  {
    if(timer==1000)
    {
     if(fan == 3){fan=0;}
     else {fan++;}
    }
    MotivateFan();
  }
  
  //Print message on LCD
  PrintOnLCD(mode,currenttemp);
  
  //update timer
  if (timer<1000){timer+=100;}
  else {timer=0;}
}

void ReadDCCurrent()
{
  data[index] = analogRead(CurrentPin);
  delay(10);
  index++;
  data[index] = analogRead(CurrentPin);
  if (index<9){index++;}
  else 
  {
    index = 0;
    int sum=0; for(int i=0; i<10; i++){sum+=data[i];}
    finalcurrent = (sum/10-513.25)/63.75;
    if (finalcurrent<0){finalcurrent=0;}
    //Serial.println(finalcurrent);
  }
}

void translateIR(int& mode, float currenttemp)//operation with EE and change Mode
{
  switch(results.value)
  {
    case 0xFFC23D: //PLAY/PAUSE, OFF MODE, message 000 (OFF)
      mode = 9;
      digitalWrite(EEPin[0],LOW);digitalWrite(EEPin[1],LOW);digitalWrite(EEPin[2],LOW);
      break;
      
    case 0xFFE01F: //VOL-, Manual Temp Mode, turn down temp setting
      if (mode == 1)
      {
        if (fan!=0){fan--;}
        MotivateFan();
      }
      else  {settemp--;TempControl(currenttemp, mode);mode=0;}//default: manual temp
      break;
    
    case 0xFFA857: //VOL+, Manual Temp Mode, turn up temp setting
      if (mode == 1)
      {
        if (fan!=3){fan++;}
        MotivateFan();
      }
      else  {settemp++;TempControl(currenttemp, mode);mode=0;}//default: manual temp
      break;
      
    case 0xFF6897: //0, Manual Temp Control
      mode = 0;
      MotivateFan(); 
      break;

    case 0xFF30CF: //1, Manual Fan Control 
      mode = 1;
      MotivateFan(); 
      break;
      
    case 0xFF18E7: //2, Time Auto Mode, message 101(TIME)
      mode = 2;
      //fan will be motivated in the end of the loop
      break;
   
    case 0xFF7A85: //3, Light Auto Mode, message 110 (LIGHT)
      mode = 3;
      digitalWrite(EEPin[0],HIGH);digitalWrite(EEPin[1],HIGH);digitalWrite(EEPin[2],LOW);
      break;
    
    case 0xFF10EF: //4, Energy Saving, message 100 (saving)
      mode = 4;
      digitalWrite(EEPin[0],HIGH);digitalWrite(EEPin[1],LOW);digitalWrite(EEPin[2],LOW);
      break;
      
    case 0xFF38C7: //5, Temp Auto Mode, use function to send message
      mode = 5;
      TempControl(currenttemp, mode);
      break;
    
    case 0xFF22DD: //PREV, screen 0 to choose screen
      screen = 0;
      MotivateFan();
      break;
      
    case 0xFF02FD: //NEXT, screen 1 to choose screen
      screen = 1;
      MotivateFan();
      break;
  }
  
}

void MotivateFan()
{
  if (fan == 3) //message 011 (HIGH)
    {digitalWrite(EEPin[0],LOW);digitalWrite(EEPin[1],HIGH);digitalWrite(EEPin[2],HIGH);}
  else if (fan == 2)//message 010 (MID)
    {digitalWrite(EEPin[0],LOW);digitalWrite(EEPin[1],HIGH);digitalWrite(EEPin[2],LOW);}
  else if (fan == 1)//message 001 (LOW)
    {digitalWrite(EEPin[0],LOW);digitalWrite(EEPin[1],LOW);digitalWrite(EEPin[2],HIGH);}
  else if (fan == 0)//message 000 (OFF)
    {digitalWrite(EEPin[0],LOW);digitalWrite(EEPin[1],LOW);digitalWrite(EEPin[2],LOW);} 
}

void TempControl(float currenttemp, int mode)
{
  float reference; 
  
  if (mode == 0){reference = settemp;}
  else if(mode == 5){reference = 22.5;}//use 22.5 as auto mode reference
  
  float diff = currenttemp-reference;
  if (diff>=4)       {fan = 3;}//message 011 (HIGH)
  else if (diff>=2)  {fan = 2;}//message 010 (MID)
  else if (diff>=0)  {fan = 1;}//message 001 (LOW)
  else               {fan = 0;}//message 000 (OFF)
  
  MotivateFan();  
}


void PrintOnLCD(int mode, float temperature)
{
  lcd.clear();
  lcd.setCursor(0,0);
  if (mode == 9){lcd.print("ModeNow: OFF");}
  else if (mode == 0){lcd.print("ModeNow: mTemp");}
  else if (mode == 1){lcd.print("ModeNow: mFan");}
  else if (mode == 2){lcd.print("ModeNow: aTime");}
  else if (mode == 3){lcd.print("ModeNow: aLight");}
  else if (mode == 4){lcd.print("ModeNow: aSave");}
  else if (mode == 5){lcd.print("ModeNow: aTemp");}
  
  if (screen == 0) {PrintModeChoice(mode);}
  else if (screen ==1 ){PrintStatus(mode, temperature);}
}

void PrintModeChoice(int mode)
{
  lcd.setCursor(-4,1);
  lcd.print("mTemp:0   mFan:1");
  lcd.setCursor(-4,2);
  lcd.print("aTime:2 aLight:3");
  lcd.setCursor(-4,3);
  lcd.print("aSave:4  aTemp:5");
}

void PrintStatus(int mode, float temperature)
{ 
  lcd.setCursor(-4,1);
  if(mode==0||mode == 9)
  {
    lcd.print("Temp Set: ");
    lcd.print(settemp);
  }
  else if (mode == 1||mode==2)
  {
    if(fan==0){lcd.print("Fan Set: OFF");}
    else if(fan==1){lcd.print("Fan Set: LOW");}
    else if(fan==2){lcd.print("Fan Set: MID");}
    else if(fan==3){lcd.print("Fan Set: HIGH");}
  }
  //else if (mode == 2){lcd.print("Time Counting...");}
  else if (mode == 3){lcd.print("Light Coming....");}
  else if (mode == 4){lcd.print("Energy Saving...");}
  else if (mode == 5){lcd.print("Temperature Auto");}
  
  
  lcd.setCursor(-4,2);
  lcd.print ("Temp Now: ");
  lcd.print(temperature);
  lcd.setCursor(-4,3);
  lcd.print("DC current: ");
  lcd.print(finalcurrent);
}
