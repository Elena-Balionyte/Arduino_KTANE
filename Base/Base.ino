const int module_id = 3;

bool ledState[3] = {false, false, false};        // the current state of the output pin
bool buttonState[3] = {false, false, false};        // the current reading from the input pin
bool lastButtonState[3] = {false, false, false};  // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime[3] = {0, 0, 0}; // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers



void setup() {
  randomSeed(analogRead(A2)); //Generating a random seed by reading floating analog pin A2

  //We are using analog pins as digital in some cases, because of the lack of pins
  pinMode(A2, INPUT_PULLUP); //A button
  pinMode(A1, INPUT_PULLUP); //A button
  pinMode(A0, INPUT_PULLUP); //A button

  Serial.begin(9600);

  byte data_length = 0;
  byte data_type = 0;


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
  // ledState now contains the readings of the buttons
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



    handle_inputs();


    if (millis() - timer >= ms) {
      return;
    }
  }
}


void setup_game(int difficulty) {


}




void loop() {
  smart_delay(1);
}

void all_on() {

}

void all_off() {

}
