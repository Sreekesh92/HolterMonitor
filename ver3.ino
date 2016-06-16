#include <SPI.h>
#include <SD.h>
#include <TimerOne.h>
#include <MsTimer2.h>

#define ECG_CHANNEL 0
#define SAMPLE_INTERVAL 10000 //sampling interval for EKG in microsecond
#define DEBOUNCE_INTERVAL 50 // 50ms

#define BUFF_SIZE  128

#define chipSelect  4 //CS pin of SD card

/* variables shared by ISR:callback() and loop()*/
volatile int buff1[BUFF_SIZE];
volatile int buff2[BUFF_SIZE];
volatile int buff_count = 0;
volatile boolean is_buff1=true;

//volatile boolean buff1_read_done, buff2_read_done;

int i=0;  // loop variable for writing buff1 or buff2 into SD card
boolean buff1_write_done=false,buff2_write_done=false;

byte hours, mins, secs; //variables holding time
String timestring;

// variables used for alert button connected to pin 2 as an interrupt
int buttonPin = 2;       
volatile int buttonState;
boolean alertButton = 0;

// variables used for leads off detection
int leadsOffPin = 3;
boolean leadsOffState = 0;
int buzzerPin = 8;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.print(F("Initializing SD card..."));

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    // don't do anything more:
    return;
  }
  Serial.println(F("card initialized."));
  File dataFile = SD.open("logg.csv", FILE_WRITE);
  dataFile.println("ECG");
  dataFile.close();
  
  /* ECG module gives 0 to 3.3V signal */
  analogReference(EXTERNAL);
  
  /* Initialise I2C devices - ADXL and RTC*/
  I2C_init();  
  
  /* Timer-1 Setup : interrupt every 10ms */
  Timer1.initialize(SAMPLE_INTERVAL); 
  Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt

  // Initialisation of alert button interrupt
  pinMode(buttonPin, INPUT);  // alert button is connected to pin 2
  // attaches buttonPin_ISR when there is a change in the buttonPin
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPin_ISR, CHANGE);
  MsTimer2::set(DEBOUNCE_INTERVAL, debounceCallback); // 50ms period
}

void loop()
{
  File dataFile = SD.open("logg.csv", FILE_WRITE);
  
  while(digitalRead(leadsOffPin)==1)
  {
    beep(1000);
    delay(1000);
  }
  
  if(is_buff1==true) // write buff2 to SD card if file opened successfully
  {
    /*  convert data from buff2 to String and write to file*/
    for(i=0;i<BUFF_SIZE-1;i++)
    {
      dataFile.print(String(buff2[i]));
      dataFile.println(F(","));
    }
   
    /* write RTC value to SD card*/
    get_time(&hours,&mins,&secs);
    timestring = String(hours)+":"+String(mins)+":"+String(secs)+",";
    Serial.println(timestring);
    buff2_write_done=true;
    if(!alertButton)
    {
      dataFile.println(String(buff2[BUFF_SIZE-1]) + "," + timestring + ",");
    }
    else
    {
      dataFile.println(String(buff2[BUFF_SIZE-1]) + "," + timestring + String(alertButton) + ",");
      alertButton = 0;
    }
  }
  
  else if(dataFile)  //write buff1 to SD card
  {
    /*  convert data from buff1 to String and write to file*/
    for(i=0;i<BUFF_SIZE;i++)
    {
      dataFile.println(String(buff1[i]));
    }
      
    buff1_write_done=true;
    Serial.println(F("buff1 -> SD"));
  }
  
  else
  {
    Serial.println(F("File opening failed"));
  }

  dataFile.close();

  /*wait until buff2 is filled by ISR */
  if(buff1_write_done==true)
  {
    //wait until is_buff1 goes false
    while(!is_buff1);
  }
  
  /* wait until buff1 is filled by ISR */
  if(buff2_write_done==true)
  {
    //wait until is_buff1 goes true
    while(is_buff1);
  }
  buff1_write_done=false;
  buff2_write_done=false;
}

void callback()
{
  int sensor = analogRead(ECG_CHANNEL);
  if(is_buff1) //write to buff1
  {
    if(buff_count==(BUFF_SIZE - 1)) //toggle to buff2 if this sample is the last sample in buff1
    {
      is_buff1 = false;
      buff_count=0;
    }
    buff1[buff_count++]=sensor;  //write current value to buff1
  } 
  else // write to buff2
  {
    if(buff_count==(BUFF_SIZE - 1)) //toggle to buff2 if this sample is the last sample in buff1
    {
      is_buff1 = true;
      buff_count=0;
    }
    buff2[buff_count++]=sensor;  //write current value to buff2
  } 
}

void buttonPin_ISR()
{
    buttonState = digitalRead(buttonPin);
    detachInterrupt(digitalPinToInterrupt(buttonPin));
    MsTimer2::start();
}

void debounceCallback()
{
    int buttonState1 = digitalRead(buttonPin);
    if(buttonState == buttonState1)
    alertButton = 1;
    attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPin_ISR, CHANGE);
    MsTimer2::stop();
}

void beep(unsigned char delayms)
{
  analogWrite(8, 130);      // Almost any value can be used except 0 and 255

  delay(delayms);          // wait for a delayms ms
  analogWrite(8, 0);       // 0 turns it off
  delay(delayms);          // wait for a delayms ms   
}
