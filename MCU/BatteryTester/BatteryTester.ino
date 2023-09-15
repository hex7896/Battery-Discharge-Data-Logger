#include <SPI.h>

#define V_SENSE_PIN A0
#define I_SENSE_PIN A1
#define MOSFET_PIN 2
#define OPCODE_START 0x11
#define OPCODE_POLL_V 0x22
#define OPCODE_POLL_I 0x33
#define OPCODE_STOP 0x36

unsigned char incoming_byte = 0x00;
int  read_buf = 4095;
int  min_read = 0;
int  max_read = 4095;

void setup() {
  analogReadResolution(12);
  Serial.begin(9600);
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, LOW);
  pinMode(V_SENSE_PIN, INPUT);
  pinMode(I_SENSE_PIN, INPUT);
}

void loop() {
    if (Serial.available()) {
      incoming_byte = Serial.read();
      if (incoming_byte == OPCODE_START)
        digitalWrite(MOSFET_PIN, HIGH);
      else if (incoming_byte == OPCODE_POLL_V) {
        min_read = 1965; //~2.4V
        max_read = 3480; //~4.25V
        read_buf = averageAnalogRead(V_SENSE_PIN, 20);
        Serial.write(read_buf & 0xff);
        Serial.write(read_buf >> 8);
      }
      else if (incoming_byte == OPCODE_POLL_I) {
        min_read = 2048; // 2.5V, i.e. 0A.
        max_read = 4096; // 5V
        read_buf = averageAnalogRead(I_SENSE_PIN, 10);
        Serial.write(read_buf & 0xff);
        Serial.write(read_buf >> 8);
      }
      else if (incoming_byte == OPCODE_STOP) {
        digitalWrite(MOSFET_PIN, LOW);
      }
    }
}

int averageAnalogRead(int pin, int n_samples) {
  long  sum = 0;
  int   adcValue;
  analogRead(pin); // Discard first value read
  for (int i=0; i < n_samples; i++) {
    adcValue = analogRead(pin);
    if (adcValue < min_read)
      adcValue = min_read;
    else if (adcValue > max_read)
      adcValue = max_read;
    sum = sum + adcValue;
  }
  return (sum/n_samples);
}