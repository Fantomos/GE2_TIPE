// Bluetooth
#include <SoftwareSerial.h>
SoftwareSerial BTSerial(7, 8); // RX | TX

// Gyro
#include <Wire.h>
//https://www.firediy.fr/article/mesurer-des-angles-avec-un-arduino-drone-ch-7
#define MPU_ADDRESS 0x68  // I2C address of the MPU-6050
#define X 0
#define Y 1
#define Z 2
float gyro_offset[3] = {0,0,0};
float real_value[3] = {0,0,0};
int acc_raw[3] = {0,0,0};
int gyro_raw[3] = {0,0,0};
int temperature = 0;

// Force
#include "HX711.h"  // Library needed to communicate with HX711 https://github.com/bogde/HX711
#define DOUT  4  // Arduino pin 6 connect to HX711 DOUT
#define CLK  3  //  Arduino pin 5 connect to HX711 CLK
HX711 scale; 

const unsigned long interval2 = 500; // ms
unsigned long lastRefreshTime = 0;


void setupMpu6050Registers() {
    // Configure power management
    Wire.beginTransmission(MPU_ADDRESS); // Start communication with MPU
    Wire.write(0x6B);                    // Request the PWR_MGMT_1 register
    Wire.write(0x00);                    // Apply the desired configuration to the register
    Wire.endTransmission();              // End the transmission
  
    // Configure the gyro's sensitivity
    Wire.beginTransmission(MPU_ADDRESS); // Start communication with MPU
    Wire.write(0x1B);     
    Wire.write(0x10);                    // Apply the desired configuration to the register : ±1000°/s
    Wire.endTransmission();              // End the transmission
  

    // Configure low pass filter
    Wire.beginTransmission(MPU_ADDRESS); // Start communication with MPU
    Wire.write(0x1A);                    // Request the CONFIG register
    Wire.write(0x03);                    // Set Digital Low Pass Filter about ~43Hz
    Wire.endTransmission();              // End the transmission
}

void calibrateMpu6050()
{
    int max_samples = 2000;

    for (int i = 0; i < max_samples; i++) {
        readSensor();

        gyro_offset[X] += gyro_raw[X];
        gyro_offset[Y] += gyro_raw[Y];
        gyro_offset[Z] += gyro_raw[Z];

        // Just wait a bit before next loop
        delay(3);
    }

    // Calculate average offsets
    gyro_offset[X] /= max_samples;
    gyro_offset[Y] /= max_samples;
    gyro_offset[Z] /= max_samples;
}

void readSensor() {
    Wire.beginTransmission(MPU_ADDRESS);// Start communicating with the MPU-6050
    Wire.write(0x3B);                   // Send the requested starting register
    Wire.endTransmission();             // End the transmission
    Wire.requestFrom(MPU_ADDRESS,14);   // Request 14 bytes from the MPU-6050

    // Wait until all the bytes are received
    while(Wire.available() < 14);
    acc_raw[X]  = Wire.read() << 8 | Wire.read(); // Add the low and high byte to the acc_raw[X] variable
    acc_raw[Y]  = Wire.read() << 8 | Wire.read(); // Add the low and high byte to the acc_raw[Y] variable
    acc_raw[Z]  = Wire.read() << 8 | Wire.read(); // Add the low and high byte to the acc_raw[Z] variable
    temperature = Wire.read() << 8 | Wire.read(); // Add the low and high byte to the temperature variable
    gyro_raw[X] = Wire.read() << 8 | Wire.read(); // Add the low and high byte to the gyro_raw[X] variable
    gyro_raw[Y] = Wire.read() << 8 | Wire.read(); // Add the low and high byte to the gyro_raw[Y] variable
    gyro_raw[Z] = Wire.read() << 8 | Wire.read(); // Add the low and high byte to the gyro_raw[Z] variable
}


void setup() {
  Serial.begin(9600);
 
  
  //Init Gyro
  setupMpu6050Registers();
  delay(1000);
  calibrateMpu6050();

  //Init bluetooth
  BTSerial.begin(38400);  //Bluetooth     Adresse MAC HC-06 : 98D3:31:FB1CB7

  //Init force
  scale.begin(DOUT, CLK);
  scale.set_scale();  // Start scale
  scale.tare();       // Reset scale to zero
}

void loop() {
   readSensor();
   if(millis() - lastRefreshTime >= interval2)
  {
    lastRefreshTime = millis();
    real_value[X] = (gyro_raw[X] - gyro_offset[X]) / (32.8/6);
    real_value[Y] = (gyro_raw[Y] - gyro_offset[Y]) / (32.8/6);
    real_value[Z] = (gyro_raw[Z] - gyro_offset[Z]) / (32.8/6);
    Serial.println(real_value[X]);
    Serial.println(real_value[Y]);
    Serial.println(real_value[Z]);
    Serial.println("\n");
    String t = "C" + String(abs(real_value[X]));
    String r = "P" + String(random(10,1000));
    BTSerial.print(r);
    BTSerial.print(t);
  }
 
 

}
