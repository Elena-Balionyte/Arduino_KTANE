#include "SevSeg.h"
SevSeg sevseg;

//TODO Add all the game words into level 2
//TODO Add solved LED
//TODO Read and comment code

//TODO Make pictures of the breadboard
//TODO Solder the stuff onto a perf-board

//TODO design morse lamp 3d print
//TODO desgin box for morse

//TODO make diagram of wiring

//TODO Audio output for morse signal


const int module_id = 3;
bool rerandomize = false;
byte level = 0;
byte to_tx[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //The message being sent, it is written in numbers (A = 0, B = 1, ...). The length of a message must not exceed 6
byte current_frequency = 0; //The frequency we are currently displaying
byte possible_frequencies[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //The frequencies in the list you can scroll through
byte number_of_words_in_level = 0;

byte WPM = 5; //Words per minute
byte DITS_IN_PARIS = 50; //The word "paris" is commonly used to measure the sending speed of morse code
int dit_time_ms = 1000 / ((WPM*DITS_IN_PARIS) / 60);
int dash_time_ms = dit_time_ms * 3;
byte spacing_WPM = 5; // The following three variables are used for calculation of the time between characters and words (Farnsworth speed) In official morse this is equal to words per minute.
int unit_gap_time_ms = 1000 / ((spacing_WPM*DITS_IN_PARIS) / 60);
int char_gap_time_ms = unit_gap_time_ms * 3;
int word_gap_time_ms = unit_gap_time_ms * 4;

// Variables will change:
bool ledState[3] = {false, false, false};        // the current state of the output pin
bool buttonState[3] = {false, false, false};        // the current reading from the input pin
bool lastButtonState[3] = {false, false, false};  // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime[3] = {0, 0, 0}; // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers


//Define dots and dashes sequences for each letter in the alphabet. False is a dit, true is a dash
bool morse_table[ 26 ][ 4 ] = {
  {false, true}, {true, false, false, false},            // A B
  {true, false, true, false}, {true, false, false},      // C D
  {false}, {false, false, true, false},                  // E F
  {true, true, false}, {false, false, false, false},     // G H
  {false, false}, {false, true, true, true},             // I J
  {true, false, true}, {false, true, false, false},      // K L
  {true, true}, {true, false},                           // M N
  {true, true, true}, {false, true, true, false},        // O P
  {true, true, false, true}, {false, true, false},       // Q R
  {false, false, false}, {true},                         // S T
  {false, false, true}, {false, false, false, true},     // U V
  {false, true, true}, {true, false, false, true},       // W X
  {true, false, true, true}, {true, true, false, false}  // Y Z
};
//The number of elements in each letter
byte morse_len_table[26] =
{ 2, 4, 4, 3, // A B C D
  1, 4, 3, 4, // E F G H
  2, 4, 3, 4, // I J K L
  2, 2, 3, 4, // M N O P
  4, 3, 3, 1, // Q R S T
  3, 4, 3, 4, // U V W X
  4, 4        // Y Z
};

//Words that can be transmitted, first dimension is the level (1-4), second is the list of words, third is the letters in a word, and signal frequency (First 5 are the letters, the sixth one is the frequency)
const static byte words[4][20][8] = {
  { //Start of level 1


    {5, 3, 18,  7, 4, 11, 11}, //Shell
    {5, 6,  7,  0, 11, 11, 18}, //Halls
    {5, 14, 19, 17, 8,  2, 10}, //Trick
    {5, 19, 1, 14, 23, 4, 18}, //Boxes
    {5, 20, 11,  4, 0, 10, 18}, //Leaks
    {5, 24, 5, 11, 8,  2, 10}, //Flick
    {5, 28, 3, 17, 8, 21, 4},  //Drive
    {5, 34, 0, 22, 0, 17, 4},  //Aware
    {5, 46, 25,  4, 1, 17, 0}, //Zebra
    {5, 52, 2, 11, 8,  2, 10}, //Click
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
  }, //End of level 1

  { //Start of level 2


    {5, 13, 18, 7, 4, 11, 11}, //Shell
    {5, 16, 7, 0, 11, 11, 18}, //Halls
    {5, 18, 19, 17, 8, 2, 10}, //Trick
    {5, 19, 1, 14, 23, 4, 18}, //Boxes
    {5, 20, 11, 4, 0, 10, 18}, //Leaks
    {5, 24, 5, 11, 8, 2, 10}, //Flick
    {5, 28, 3, 17, 8, 21, 4}, //Drive
    {5, 34, 0, 22, 0, 17, 4}, //Aware
    {5, 46, 19, 17, 0, 8, 13}, //Train
    {5, 52, 2, 11, 8, 2, 10}, //Click
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
    {0, 0, 0, 0, 0, 0, 0}, //Null
  }, //End of level 2

  { //Start of level 3

    {6, 100, 0, 12, 1, 20, 18, 7},  // Ambush
    {2, 103, 2, 22, 0, 0, 0, 0},  // CW
    {5, 106, 19, 17, 8, 2, 10, 0},  // Trick
    {5, 110, 17, 14, 6, 4, 17, 0},  // Roger
    {3, 113, 2, 0, 17, 0, 0, 0},  // Car
    {5, 117, 17, 0, 3, 8, 14, 0},  // Radio
    {5, 119, 1, 17, 8, 2, 10, 0},  // Brick
    {4, 121, 13, 20, 11, 11, 0, 0},  // Null
    {6, 129, 0, 1, 17, 20, 15, 19},  // Abrupt
    {6, 137, 0, 1, 18, 14, 17, 1},  // Absorb
    {5, 138, 25, 4, 1, 17, 0, 0},  // Zebra
    {4, 144, 2, 0, 17, 19, 0, 0},  // Cart
    {4, 151, 25, 4, 17, 14, 0, 0},  // Zero
    {5, 158, 1, 17, 8, 13, 4, 0},  // Brine
    {5, 167, 19, 17, 0, 2, 10, 0},  // Track
    {6, 176, 0, 11, 15, 0, 2, 0},  // Alpaca
    {5, 185, 1, 17, 4, 0, 10, 0},  // Break
    {5, 187, 19, 17, 0, 8, 13, 0},  // Train
    {5, 194, 25, 8, 11, 2, 7, 0},  // Zilch
    {6, 200, 17, 4, 2, 0, 11, 11},  // Recall

  }, //End of level 3

  { //Start of level 4

    {6, 100, 0, 12, 1, 20, 18, 7},  // Ambush
    {2, 103, 2, 22, 0, 0, 0, 0},  // CW
    {5, 106, 19, 17, 8, 2, 10, 0},  // Trick
    {5, 110, 17, 14, 6, 4, 17, 0},  // Roger
    {3, 113, 2, 0, 17, 0, 0, 0},  // Car
    {5, 117, 17, 0, 3, 8, 14, 0},  // Radio
    {5, 119, 1, 17, 8, 2, 10, 0},  // Brick
    {4, 121, 13, 20, 11, 11, 0, 0},  // Null
    {6, 129, 0, 1, 17, 20, 15, 19},  // Abrupt
    {6, 137, 0, 1, 18, 14, 17, 1},  // Absorb
    {5, 138, 25, 4, 1, 17, 0, 0},  // Zebra
    {4, 144, 2, 0, 17, 19, 0, 0},  // Cart
    {4, 151, 25, 4, 17, 14, 0, 0},  // Zero
    {5, 158, 1, 17, 8, 13, 4, 0},  // Brine
    {5, 167, 19, 17, 0, 2, 10, 0},  // Track
    {6, 176, 0, 11, 15, 0, 2, 0},  // Alpaca
    {5, 185, 1, 17, 4, 0, 10, 0},  // Break
    {5, 187, 19, 17, 0, 8, 13, 0},  // Train
    {5, 194, 25, 8, 11, 2, 7, 0},  // Zilch
    {6, 200, 17, 4, 2, 0, 11, 11},  // Recall

  } //End of level 4
};

void setup() {
  randomSeed(analogRead(A2)); //Generating a random seed by reading floating analog pin A2

  //We are using analog pins as digital in some cases, because of the lack of pins
  pinMode(A5, OUTPUT); //LED for morse
  pinMode(A4, OUTPUT); //LED for module solved
  pinMode(A2, INPUT_PULLUP); //Up button
  pinMode(A1, INPUT_PULLUP); //Down button
  pinMode(A0, INPUT_PULLUP); //Send (TX) button

  Serial.begin(9600);

  byte data_length = 0;
  byte data_type = 0;

  byte difficulty = 0;

  while (true) {
    if (Serial.available() > 1) {
      data_length = Serial.read();
      data_type = Serial.read();
      if (data_type == 0x02) { //If the signal for the end of the startup phase is received, respond with hello
        Serial.write(0x03);
        Serial.write(0x15);
        Serial.write(module_id);
      } else if (data_type == 0x03) { //If the difficulty is received
        while (!Serial.available()) {}
        difficulty = Serial.read();
      } else if (data_type == 0x04) { //If the signal for the end of the startup phase is received
        break;
      } else {
        while (data_length > 2) {
          while (!Serial.available()) {}
          Serial.read();
          data_length--;
        }
      }
    }
  }

  setup_game(difficulty); //Passing the difficulty, 1-4

  //Setup for the 7 segment display
  byte numDigits = 4;
  byte digitPins[] = {2, 3, 4, 5};
  byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_ANODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
               updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(100);
  sevseg.setNumber(possible_frequencies[current_frequency] + 3500);
}
//Light
void on() {
  digitalWrite(A5, HIGH); //Turn the LED on
}
//Light
void off() {
  digitalWrite(A5, LOW);
}

void handle_inputs() {
  byte pins[3] = {A0, A1, A2};
  bool reading[3] = {false, false, false};
  for (int i = 0; i < 3; i++) {
    // read the state of the switch into a local variable:
    reading[i] = digitalRead(pins[i]);

    // check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH), and you've waited long enough
    // since the last press to ignore any noise:

    // If the switch changed, due to noise or pressing:
    if (reading[i] != lastButtonState[i]) {
      // reset the debouncing timer
      lastDebounceTime[i] = millis();
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      // whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:

      // if the button state has changed:
      if (reading[i] != buttonState[i]) {
        buttonState[i] = reading[i];
        if (buttonState[i] == LOW) {
          ledState[i] = true;
        }
      }
    }


    // save the reading. Next time through the loop, it'll be the lastButtonState:
    lastButtonState[i] = reading[i];
  }

  if (ledState[2] == true) { // Up button
    if (current_frequency < number_of_words_in_level - 1) {
      current_frequency = current_frequency + 1;
      sevseg.setNumber(possible_frequencies[current_frequency] + 3500);
    }
    ledState[2] = false;
  }

  if (ledState[1] == true) { // Down button
    if (current_frequency > 0) {
      current_frequency = current_frequency - 1;
      sevseg.setNumber(possible_frequencies[current_frequency] + 3500);
    }
    ledState[1] = false;
  }

  if (ledState[0] == true) { // Send button
    if (possible_frequencies[current_frequency] == to_tx[1]) {
      //Send confirmation to the master that the module is solved
      Serial.write(0x03);
      Serial.write(0x14);
      Serial.write(module_id);
      on();
      sevseg.blank();
      delay(10000);
    } else {
      //Send information that a strike occured
      Serial.write(0x02);
      Serial.write(0x12);
      int index = random(0, number_of_words_in_level);
      for (int i = 0; i < 8; i++) {
        to_tx[i] = words[level][index][i]; //Copying a random word from the list of words, to the variable that stores the word we are transmitting
      }
      off();
      sevseg.blank();
      delay(10000);
      rerandomize = true;
    }
    ledState[0] = false;
  }
}


void smart_delay(int ms) {
  unsigned long timer = millis();
  byte data_length = 0;
  byte data_type = 0;
  while (true) {
    if (Serial.available() > 1) {
      data_length = Serial.read();
      data_type = Serial.read();
      if (data_type == 0x00) { //If the bomb exploded message is received
        all_on();
        smart_delay(3000);
        all_off();
        while (true) {}
      }
      if (data_type == 0x01) { //If the signal for the end of the startup phase is received
        all_off();
        while (true) {}
      }
      while (data_length > 2) {
        while (!Serial.available()) {}
        Serial.read();
        data_length--;

      }
    }


    sevseg.refreshDisplay();

    handle_inputs();


    if (millis() - timer >= ms) {
      return;
    }
  }
}


void setup_game(int difficulty) {

  level = difficulty - 1;
  for (int i = 0; i < 20; i++) {
    possible_frequencies[i] = words[level][i][1];

    if (possible_frequencies[i] == 0) {
      number_of_words_in_level = i;
      break;
    }
  }
  Serial.write(0x0b);
  Serial.write(0x0b);
  Serial.write(0x02);
  int index = random(0, number_of_words_in_level);
  for (int i = 0; i < 8; i++) {
    to_tx[i] = words[level][index][i]; //Copying a random word from the list of words, to the variable that stores the word we are transmitting
    if (i > 1) {
      Serial.write(words[level][index][i] + 97);
    } else {
      Serial.write(words[level][index][i]);
    }
  }





  if (level == 3) {
    WPM = 10; //Words per minute
    dit_time_ms = 1000 / ((WPM * DITS_IN_PARIS) / 60);
    dash_time_ms = dit_time_ms * 3;
    spacing_WPM = 10; // The following three variables are used for calculation of the time between characters and words (Farnsworth speed) In official morse this is equal to words per minute.
    unit_gap_time_ms = 1000 / ((spacing_WPM * DITS_IN_PARIS) / 60);
    char_gap_time_ms = unit_gap_time_ms * 3;
    word_gap_time_ms = unit_gap_time_ms * 4;
  }
}

void all_on() { //Turn on the LED and set seven segment display to all 8, for detonation
  sevseg.setNumber(8888);
  on();
}

void all_off() { //Turn all LEDs and seven segment display off, for defused state
  sevseg.blank();
  off();
}


void loop() {



  sevseg.setNumber(possible_frequencies[current_frequency] + 3500);
  for (int i = 2; i < to_tx[0] + 2; i++) { //Go through the letters in the word. The word starts at positon 2 (since the first two
    int letter = to_tx[i]; //Get the current letter
    for (int j = 0; j < morse_len_table[letter]; j++) { //Go through each dit and dash in the letter
      if (morse_table[letter][j] == false) { // If it's a dit, turn on for the amount of time needed for a dit
        on();
        smart_delay(dit_time_ms);
      } else { // If it's not a dit, it's a dash, turn on for the amount of time needed for a dash
        on();
        smart_delay(dash_time_ms);
      }
      off(); //Turn off the LED and buzzer

      if (j != morse_len_table[letter] - 1) {
        smart_delay(dit_time_ms); //Wait between dits and dashes
      }
    }
    smart_delay(char_gap_time_ms); //Wait between letters
  }
  smart_delay(word_gap_time_ms); //Wait between words
  if (rerandomize) {
    setup_game(level + 1);
    rerandomize = false;
  }
}
