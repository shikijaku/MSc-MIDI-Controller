// Arduino Code written by Liam GRAS for the MSc thesis at Staffordshire University
// 20/08/2021 - Stoke On Trent
// Permission to keep the rights and therefore share this work granted by the University

// Arduino Micro MIDIusb Controller 

  // Include MIDIUSB library
#include "MIDIUSB.h"
#include "pitches.h"

  // Define the number of components
#define NUMBER_BUTTONS  8
#define NUMBER_POTS  6

  // Set the pins
const int potPin0 = 0;
const int potPin1 = 1;
const int potPin2 = 2;
const int potPin3 = 3;
const int potPin4 = 4;
const int potPin5 = 5;

const int buttonPin2 = 2;
const int buttonPin3 = 3;
const int buttonPin4 = 4;
const int buttonPin5 = 5;
const int buttonPin6 = 6;
const int buttonPin7 = 7;
const int buttonPin8 = 8;
const int buttonPin9 = 9;
const int buttonPin10 = 10;

  // Set the notes, buttons and pots
const uint8_t buttons[NUMBER_BUTTONS] = {buttonPin2, buttonPin3, buttonPin4, buttonPin5, buttonPin6, buttonPin7, buttonPin8, buttonPin9};
int notePitches[NUMBER_BUTTONS] = {48, 49, 50, 51, 52, 53, 54, 55};
int notes[64] = {48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111};
const uint8_t pots[NUMBER_POTS] = {potPin0, potPin1, potPin2, potPin3, potPin4, potPin5};

uint8_t pressedButtons = 0x00;
uint8_t previousButtons = 0x00;

  // Initialize values
int bank = 1;
uint8_t val[NUMBER_POTS] = {0,0,0,0,63,63};
uint8_t previousVal[NUMBER_POTS] = {0,0,0,0,63,63};
uint8_t filteredVal[NUMBER_POTS] = {0,0,0,0,63,63};
int buttonState = 0;

  // Set up filters for filtered potentiometers values
float filterX = 0.9;
float filterY = 0.1;

  // Testing the frequency
float timeStart;
float timeEnd;
float countFq = 0;

// Set Up Pins as Inputs
void setup() {
  //Serial.begin(31250);
  Serial.begin(115200);
  for (int i = 0; i < NUMBER_BUTTONS; i++)
    pinMode(buttons[i], INPUT_PULLUP);
  pinMode(buttonPin10, INPUT_PULLUP);
  timeStart = millis();
} 

void loop() {
  // put your main code here, to run repeatedly:
  timeEnd = millis();
  readButtons();
  readPots();
  
  if (buttonState == 0)
  {
  playNotes();
  }
  
  sendChange();

  countFq = countFq + 1;
  if (timeEnd - timeStart == 10000)
  {
    Serial.print(countFq/10);
  }
  /* DEBUGGING
Serial.print("Analog0 : ");
Serial.println(val0);
Serial.print("Analog1 : ");
Serial.println(val1);
Serial.print("Analog2 : ");
Serial.println(val2);
Serial.print("Digital 2 :");
Serial.println(button2State);
Serial.print("Digital 3 :");
Serial.println(button3State);
Serial.print("Digital 4 :");
Serial.println(button4State);
Serial.println(bitRead(pressedButtons, 0));
Serial.println(val[0]);
Serial.println(val[1]);
Serial.println(val[2]);
Serial.println(notePitches[1]);
  */
  
  // Select bank menu
if (buttonState == 1)
  {
   for (int k = 0; k < NUMBER_BUTTONS; k++)
   {
    if (bitRead(pressedButtons, k) == 1)
      { 
      bank = k+1;
      changeBank(bank);
      //Serial.print("You have chosen Bank");
      //Serial.println(bank);
      }
   }
  }
}

  // change Bank Menu
void changeBank(int number)
  {
   for (int h = 0; h<NUMBER_BUTTONS;h++)
        {
          notePitches[h] = notes[h+(number-1)*NUMBER_BUTTONS];
        }
  }

  // Read buttons
void readButtons()
  {
  for (int r = 0; r < NUMBER_BUTTONS; r++)
    {
      if (digitalRead(buttons[r]) == HIGH)
      {
        bitWrite(pressedButtons, r, 1);
        delay(5);
        //Serial.println(r);
      }
      else
        bitWrite(pressedButtons, r, 0);
    }
  buttonState = digitalRead(buttonPin10);
  }

  // Read the pots
void readPots()
{
  for (int j = 0; j < NUMBER_POTS;j++)
    {
    val[j] = analogRead(pots[j])/8;
    filteredVal[j] = filterX * val[j] + filterY * previousVal[j];
    }
}

  // Send digital values (buttons)
void playNotes()
{
  for (int y = 0; y < NUMBER_BUTTONS; y++)
  {
    if (bitRead(pressedButtons, y) != bitRead(previousButtons, y))
    {
      if (bitRead(pressedButtons, y))
      {
        bitWrite(previousButtons, y , 1);
        noteOn(0, notePitches[y], 80);
        MidiUSB.flush();
      }
      else
      {
        bitWrite(previousButtons, y , 0);
        noteOff(0, notePitches[y], 0);
        MidiUSB.flush();
      }
    }
  }
}

  // Send analog values
void sendChange()
{
  for (int m = 0;m<NUMBER_POTS;m++)
  {
    if (abs(filteredVal[m] - previousVal[m]) > 2)
    {
      controlChange(0, 1 + m, filteredVal[m]);
      MidiUSB.flush();
      previousVal[m] = filteredVal[m];
    }
  }
}

  // MIDI USB functions

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}
