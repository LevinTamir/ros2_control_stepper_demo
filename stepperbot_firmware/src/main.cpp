#include <Arduino.h>

// STEP: D5, DIR: D6
const int STEP_PIN = 5;
const int DIR_PIN  = 6;

const long MAX_STEPS_PER_SECOND        = 800;
const unsigned long STEP_INTERVAL_US   = 1000000UL / MAX_STEPS_PER_SECOND;
const unsigned int STEP_PULSE_WIDTH_US = 4;

volatile long current_steps = 0;
volatile long target_steps  = 0;

unsigned long last_step_time_us = 0;

static const int INPUT_BUFFER_SIZE = 64;
char input_buffer[INPUT_BUFFER_SIZE];
int input_pos = 0;

void handleSerial();
void processCommand(const char * line);
void updateMotion();

void setup()
{
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  current_steps = 0;
  target_steps  = 0;

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
  if (line[0] == 'P' || line[0] == 'p') {
    long steps = 0;
    int matched = sscanf(line + 1, "%ld", &steps);
    if (matched == 1) {
      target_steps = steps;
    }
  } else if (line[0] == 'Z' || line[0] == 'z') {
    current_steps = 0;
    target_steps  = 0;
  }
}

void updateMotion()
{
  if (current_steps == target_steps) {
    return;
  }

  unsigned long now_us = micros();
  if (now_us - last_step_time_us < STEP_INTERVAL_US) {
    return;
  }

  bool dir = (target_steps > current_steps);
  digitalWrite(DIR_PIN, dir ? HIGH : LOW);

  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(STEP_PULSE_WIDTH_US);
  digitalWrite(STEP_PIN, LOW);

  if (dir) {
    current_steps++;
  } else {
    current_steps--;
  }

  last_step_time_us = now_us;
}
