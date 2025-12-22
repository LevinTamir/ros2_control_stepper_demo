#include <Arduino.h>

// STEP1: D5, DIR1: D6
// STEP2: D9, DIR2: D10

const int STEP_PIN_1 = 5;
const int DIR_PIN_1  = 6;
const int STEP_PIN_2 = 9;
const int DIR_PIN_2 = 10;

const long MAX_STEPS_PER_SECOND        = 200;
const unsigned long STEP_INTERVAL_US   = 1000000UL / MAX_STEPS_PER_SECOND;
const unsigned int STEP_PULSE_WIDTH_US = 4;

volatile long current_steps_1 = 0;
volatile long current_steps_2 = 0;
volatile long target_steps_1  = 0;
volatile long target_steps_2  = 0;

unsigned long last_step_time_us = 0;

static const int INPUT_BUFFER_SIZE = 64;
char input_buffer[INPUT_BUFFER_SIZE];
int input_pos = 0;


void handleSerial();
void processCommand(const char * line);
void updateMotion();

void setup()
{
  pinMode(STEP_PIN_1, OUTPUT);
  pinMode(STEP_PIN_2, OUTPUT);
  pinMode(DIR_PIN_1, OUTPUT);
  pinMode(DIR_PIN_2, OUTPUT);

  digitalWrite(STEP_PIN_1, LOW);
  digitalWrite(STEP_PIN_2, LOW);
  digitalWrite(DIR_PIN_1, LOW);
  digitalWrite(DIR_PIN_2, LOW);

  current_steps_1 = 0;
  current_steps_2 = 0;
  target_steps_1  = 0;
  target_steps_2  = 0;

  Serial.begin(115200);
  while (!Serial) {
    ; // needed only for some boards
  }
}

void loop()
{
  handleSerial();
  updateMotion();
}

void handleSerial()
{
  while (Serial.available() > 0)
  {
    char c = static_cast<char>(Serial.read());

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      input_buffer[input_pos] = '\0';
      processCommand(input_buffer);
      input_pos = 0;
    } else {
      if (input_pos < INPUT_BUFFER_SIZE - 1) {
        input_buffer[input_pos++] = c;
      } else {
        input_pos = 0;
      }
    }
  }
}

void processCommand(const char * line)
{
  // Parse format: "P1 steps1 P2 steps2"
  long steps1 = 0, steps2 = 0;
  int matched = sscanf(line, "P1 %ld P2 %ld", &steps1, &steps2);
  
  if (matched == 2) {
    target_steps_1 = steps1;
    target_steps_2 = steps2;
    return;
  }
  
  // Legacy format: P1steps or P2steps
  if (line[0] == 'P' || line[0] == 'p') {
    if (line[1] == '1') {
      long steps = 0;
      int matched = sscanf(line + 2, "%ld", &steps);
      if (matched == 1) {
        target_steps_1 = steps;
      }
    } else if (line[1] == '2') {
      long steps = 0;
      int matched = sscanf(line + 2, "%ld", &steps);
      if (matched == 1) {
        target_steps_2 = steps;
      }
    }
  } else if (line[0] == 'Z' || line[0] == 'z') {
    current_steps_1 = 0;
    target_steps_1  = 0;
    current_steps_2 = 0;
    target_steps_2  = 0;
  }
}

void updateMotion()
{
  unsigned long now_us = micros();
  if (now_us - last_step_time_us < STEP_INTERVAL_US) {
    return;
  }

  // Motor 1
  if (current_steps_1 != target_steps_1) {
    bool dir = (target_steps_1 > current_steps_1);
    digitalWrite(DIR_PIN_1, dir ? HIGH : LOW);

    digitalWrite(STEP_PIN_1, HIGH);
    delayMicroseconds(STEP_PULSE_WIDTH_US);
    digitalWrite(STEP_PIN_1, LOW);

    if (dir) {
      current_steps_1++;
    } else {
      current_steps_1--;
    }
  }

  // Motor 2
  if (current_steps_2 != target_steps_2) {
    bool dir = (target_steps_2 > current_steps_2);
    digitalWrite(DIR_PIN_2, dir ? HIGH : LOW);

    digitalWrite(STEP_PIN_2, HIGH);
    delayMicroseconds(STEP_PULSE_WIDTH_US);
    digitalWrite(STEP_PIN_2, LOW);

    if (dir) {
      current_steps_2++;
    } else {
      current_steps_2--;
    }
  }

  last_step_time_us = now_us;
}
