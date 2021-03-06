//SDS011 Nova PM sensor

#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <stdio.h>
#include <LIDARLite.h>
#include <SDS011.h> //for Nova PM Sensor
   
LIDARLite myLidarLite;
SDS011 my_sds;

//init for Lidar sensors. Assigning memory addresses.
int sensorPins[] = {2, 4}; // Array of pins connected to the sensor Power Enable lines
unsigned char addresses[] = {0x66, 0x68};

//for Sonar init
const int anPin1 = 0;
const int anPin2 = 1;
const int anPin3 = 2;
const int triggerPin1 = 13;

//for Gases sensor init
// ADS address needs to be set appropriately based on the gas sensor number (among 0x48, 0x49, 0x4A, 0x4B)
Adafruit_ADS1015 ads;    

// PM
float p10, p25;
int error;

// Reset Pin
int resetPin = 12;

void setup() {
  
  digitalWrite(resetPin, HIGH);
  delay(200);
  pinMode(resetPin, OUTPUT);     
  Wire.begin();       //join i2c bus
  my_sds.begin(10, 11);   //start Nova PM sensor with digital pins 10 and 11
  //Serial.begin(9600, SERIAL_7E1); //start serial for output
  Serial.begin(9600, SERIAL_8N1);
  ads.begin();  
//  ads.setGain(GAIN_ONE);
  myLidarLite.begin();
  myLidarLite.changeAddressMultiPwrEn(2, sensorPins, addresses);
  pinMode(triggerPin1, OUTPUT); 
}



//SONAR SENSORS
void start_sensor() {
  digitalWrite(triggerPin1, HIGH);
  delay(1);
  digitalWrite(triggerPin1, LOW);
}

//GASES SENSORS
float *get_gas_values() {
  static float values[4];
  int Vx[4];
  float Ix[4];
  
  //initialising base voltage and current values for each gas
  Vx[0] = 818;
  Vx[1] = 823;
  Vx[2] = 812;
  Vx[3] = 810;
  Ix[0] = 0.00000000475;
  Ix[1] = 0.000000025;
  Ix[2] = 0.000000032;
  Ix[3] = 0.0000000040;

  //adjusting sensor values
  for (int i = 0; i < 4; i++) {
    //values[i] = (ads.readADC_SingleEnded(i) - Vx[i]) / (500000 * Ix[i]);
    values[i] = ads.readADC_SingleEnded(i);
    delay(10);
  }
  return values;
}

void print_gas() {
  String gas[4];
  gas[0] = "CO";
  gas[1] = "SO";
  gas[2] = "O3";
  gas[3] = "NO";
  float *v = get_gas_values();
  
  for (int i = 0; i < 4; i++) {
    Serial.print(gas[i] + " ");
    Serial.print(v[i]);
    Serial.print(" ");
  }
}

//LIDAR SENSORS
float* get_lidar_distance() {
  static float lidars[2];
  lidars[0] = myLidarLite.distance(true, true, 0x66);
  lidars[1]  = myLidarLite.distance(true, true, 0x68);
  return lidars;
}

void print_lidar() {
  float *lidars = get_lidar_distance();
  Serial.print("Lidar1 ");
  Serial.print(lidars[0]);
  Serial.print(" ");
  Serial.print("Lidar2 ");
  Serial.print(lidars[1]);
  Serial.print(" ");
}


float *get_sonar_values() {
  /*
    Scale factor is (Vcc/512) per inch. A 5V supply yields ~9.8mV/in
    Arduino analog pin goes from 0 to 1024, so the value has to be divided by 2 to get the actual inches
  */
  static float distance[3];
  distance[0] = analogRead(anPin1) / 2;
  distance[1] = analogRead(anPin2) / 2;
  distance[2] = analogRead(anPin3) / 2;
  
  return distance;
}

void print_sonar() {
  float *dists = get_sonar_values();
  Serial.print("SONAR1 ");
  Serial.print(dists[0] * 2.54);
  Serial.print(" SONAR2 ");
  Serial.print(dists[1] * 2.54);
  Serial.print(" SONAR3 ");
  Serial.print(dists[2] * 2.54);
  Serial.print(" ");
}


void print_pm() { 
  p10 = 0;
  p25= 0;
  error = my_sds.read(&p25, &p10);
  if (!error) {
    Serial.print("P25 " + String(p25) + " ");
    Serial.print("P10 " + String(p10) + " ");
  }
}

int i = 1;

//MAIN LOOP
void loop() {
    int response = Serial.read();

    if (response == 102){ // "f" means fail
      Serial.println("Serial port reset...");
      digitalWrite(resetPin, LOW);
      delay(20);
      digitalWrite(resetPin, HIGH);
      delay(200);
    } else if (response == 103) { // "g" means good
      print_lidar();
      start_sensor();
      print_sonar();
  
      if (i%5 == 0){
        print_gas();
        print_pm();
        i=1;
      }
      i++;
      Serial.println();
    } else {
      delay(200);
    }
    delay(20);
}
