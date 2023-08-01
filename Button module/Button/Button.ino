const int module_id = 4;
int difficulty = 1;
bool time_read = false;
int timer_minutes = 0;
int timer_raw_seconds = 0;
bool button_pushed = false;
float max_tap_length = 1000; //Milliseconds
float time_tap_started = 0; //Milliseconds

const int RED = 0;
const int WHITE = 1;
const int BLUE = 2;
const int YELLOW = 3;
const int GREEN = 4;
const int BLACK = 5; //Not illuminated


const int colors[6][3] {
  {255, 0, 0}, //Red
  {255, 100, 100}, //White
  {0, 0, 255}, //Blue
  {255, 80, 0}, //Yellow
  {0, 255, 0}, //Green
  {0, 0, 0}, //Black (Not illuminated)
};

int chosen_word = 0; //ABORT, DETONATE, HOLD, PRESS
int chosen_button_color = 0; //Red, White, Blue, Yellow, Green
int chosen_strip_color = 0; //Red, White, Blue, Yellow, Green (Not used for strip)

const int ABORT = 0;
const int DETONATE = 1;
const int HOLD = 2;
const int PRESS = 3;

//Inidcator constants (IDs)
const int CAR = 1;
const int FRK = 4;

//Indicators from the PI
int battery_quantity = 0;
int indicators = 0; //Bitmask of the indicators (TRN, BOB, MSA, NSA, FRQ, FRK, CLR, CAR), Most Significant Bit first

void setup() {
  randomSeed(analogRead(A5)); //Generating a random seed by reading floating analog pin A5

  pinMode(3, OUTPUT); //Red Strip LED
  pinMode(5, OUTPUT); //Green Strip LED
  pinMode(6, OUTPUT); //Blue Strip LED

  pinMode(9,  OUTPUT); //Red Button LED
  pinMode(10, OUTPUT); //Green Button LED
  pinMode(11, OUTPUT); //Blue Button LED
  pinMode(12, OUTPUT); //Red status LED
  pinMode(13, OUTPUT); //Green status LED

  pinMode(7, INPUT_PULLUP); //Button


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

        //Ask for the bitmask
        Serial.write(0x02);
        Serial.write(0x0e);

        //Ask for number of batteries
        Serial.write(0x02);
        Serial.write(0x0f);

      } else if (data_type == 0x03) { //If the difficulty is received
        while (!Serial.available()) {}
        difficulty = Serial.read();
        setup_game(difficulty); //Passing the difficulty, 1-4
      } else if (data_type == 0x07) { //If the LED indicators are received
        while (!Serial.available()) {}
        indicators = Serial.read();
      } else if (data_type == 0x08) { //If the battery quantity is received
        while (!Serial.available()) {}
        int raw_batteries = Serial.read();
        int D_batteries =  (raw_batteries & 0x0f) >> 0; //Get the lower nibble from raw batteries, we are shifting by zero so the code looks conistent with the bottom line
        int AA_batteries = (raw_batteries & 0xf0) >> 4; //Get the upper nibble from raw batteries, we need to bitshift by 4 here
        battery_quantity = D_batteries + AA_batteries;
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
  set_strip_color(BLACK);//Reset the strip color to black, after it was used to indicate the word to place
  set_button_color(chosen_button_color);//Setting the button color
}

void handle_inputs() {
  if (digitalRead(7) == LOW) {
    if (button_pushed == false) {
      button_pushed = true;
      time_tap_started = millis();
    }
    if ((millis() - time_tap_started) > 300) {
      set_strip_color(chosen_strip_color);
    }

  }
  else if (digitalRead(7) == HIGH and button_pushed and ((millis() - time_tap_started) < 50)) {
    button_pushed = false;
  }
  else if (digitalRead(7) == HIGH and button_pushed and ((millis() - time_tap_started) > 50)) {

    button_pushed = false;
    set_strip_color(BLACK);

    float pushed_time = millis() - time_tap_started;

    //Ask for the time
    Serial.write(0x02);
    Serial.write(0x0c);

    while (!time_read) {
      smart_delay(1); //We are delaying in case the bomb explodes while we are asking for the time
    }
    time_read = false;
    int timer_first_digit_seconds = (timer_raw_seconds & 0xf0) >> 4; //Get the upper nibble from raw seconds, we need to bitshift by 4 here
    int timer_second_digit_seconds =  (timer_raw_seconds & 0x0f) >> 0; //Get the lower nibble from raw seconds, we are shifting by zero so the code looks conistent with the bottom line

    bool button_must_be_held = false;

    if (chosen_button_color == BLUE and chosen_word == ABORT) {
      button_must_be_held = true;
    } else if (battery_quantity > 1 and chosen_word == DETONATE) {
      button_must_be_held = false;
    } else if (chosen_button_color == WHITE and indicators & CAR) {
      button_must_be_held = true;
    } else if (battery_quantity > 2 and indicators & FRK) {
      button_must_be_held = false;
    } else if (chosen_button_color == YELLOW) {
      button_must_be_held = true;
    } else if (chosen_button_color == RED and chosen_word == HOLD) {
      button_must_be_held = false;
    } else {
      button_must_be_held = true;
    }


    bool button_was_held = pushed_time > max_tap_length; //Was the button pushed for long enough to be considered not a tap?
    int target_number = 0;

    int minutes_plus_tollerance = timer_minutes;
    int seconds_first_plus_tollerance = timer_first_digit_seconds;
    int seconds_second_plus_tollerance = timer_second_digit_seconds + 1;
    int minutes_minus_tollerance = timer_minutes;
    int seconds_first_minus_tollerance = timer_first_digit_seconds;
    int seconds_second_minus_tollerance = timer_second_digit_seconds - 1;

    if (seconds_second_plus_tollerance > 9) { //Second digit of seconds digit is X 0:0X
      seconds_second_plus_tollerance = 0;
      seconds_first_plus_tollerance += 1;
    }

    if (seconds_second_minus_tollerance < 0) {
      seconds_second_plus_tollerance = 9;
      seconds_first_plus_tollerance -= 1;
    }

    if (seconds_first_plus_tollerance > 5) {
      seconds_first_plus_tollerance = 0;
      minutes_plus_tollerance += 1;
    }

    if (seconds_first_minus_tollerance < 0) {
      seconds_first_plus_tollerance = 5;
      minutes_plus_tollerance -= 1;
    }


    if (not button_must_be_held and not button_was_held) {
      solved();
    } else if (button_must_be_held and  button_was_held) {
      if (chosen_strip_color == BLUE) {
        target_number = 4;
      } else if (chosen_strip_color == YELLOW) {
        target_number = 5;
      } else {
        target_number = 1;
      }

      bool target_number_is_in_minutes = (minutes_plus_tollerance == target_number or timer_minutes == target_number or minutes_minus_tollerance == target_number);
      bool target_number_is_in_first_digit_of_seconds = (seconds_first_plus_tollerance == target_number or timer_first_digit_seconds == target_number or seconds_first_minus_tollerance == target_number);
      bool target_number_is_in_second_digit_of_seconds = (seconds_second_plus_tollerance == target_number or timer_second_digit_seconds == target_number or seconds_second_minus_tollerance == target_number);
      if (target_number_is_in_minutes or target_number_is_in_first_digit_of_seconds or target_number_is_in_second_digit_of_seconds) {
        solved();
      } else {
        strike();
      }
    } else {
      strike();
    }
    //delay(100); //We can put this in as a debugging step (for debounce)
  }
}

void solved() {
  //Send confirmation to the master that the module is solved
  Serial.write(0x03);
  Serial.write(0x14);
  Serial.write(module_id);
  set_strip_color(BLACK);
  chosen_strip_color = random(0, 4);
  digitalWrite(12, LOW);//Set status LED to NOT red
  digitalWrite(13, HIGH);//Set status LED to green
}

void strike() {
  //Send information that a strike occured
  Serial.write(0x02);
  Serial.write(0x12);
  chosen_strip_color = random(0, 4);
  digitalWrite(13, LOW);//Set status LED to NOT green
  digitalWrite(12, HIGH);//Set status LED to red
}


void set_button_color(int color) {
  analogWrite(9, colors[color][0]); //Red
  analogWrite(10, colors[color][1]); //Green
  analogWrite(11, colors[color][2]); //Blue
}

void set_strip_color(int color) {
  analogWrite(3, colors[color][0]); //Red
  analogWrite(5, colors[color][1]); //Green
  analogWrite(6, colors[color][2]); //Blue
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
      if (data_type == 0x05) {
        while (!Serial.available()) {}
        timer_minutes = Serial.read();
        while (!Serial.available()) {}
        timer_raw_seconds = Serial.read();
        time_read = true;
        data_length -= 2;

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
  chosen_word = random(0, 4);
  chosen_button_color = random(0, 5);
  chosen_strip_color = random(0, 4);

  while (chosen_button_color == chosen_strip_color) {
    chosen_strip_color = random(0, 4);
  }

  //Indicate with the LED what word the person setting up needs to put on
  set_button_color(chosen_word);
  set_strip_color(chosen_word);
  //Sending this for the souvenir module
  Serial.write(0x06);
  Serial.write(0x0b);
  Serial.write(0x02);
  Serial.write(chosen_word);
  Serial.write(chosen_button_color);
  Serial.write(chosen_strip_color);
}




void loop() {
  smart_delay(1);
}

void all_on() {
  set_button_color(WHITE);
  set_strip_color(WHITE);
}

void all_off() {
  set_button_color(BLACK);
  set_strip_color(BLACK);
}
