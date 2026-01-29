//Real really real code
#include <LiquidCrystal.h>
#include <Wire.h>
#include <MPU6050.h>
#include <MatrizLed.h>
















// Accelerometer
MPU6050 mpu(0x68);
MatrizLed matrix;
















// LCD pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
















// --- Sensitivity Settings ---
const float LOWER_LIMIT = 4.0;
const float UPPER_LIMIT = 5.0;
float smoothedZ = 9.8;  
//float absVelo = 0.0;    
float alpha = 0.3;
















// --- Logic Variables ---
int repCount = 0;
bool isDown = false;
















// Matrix Pins
int Din = 5;
int CS = 4;
int Clk = 3;
















// 7-Segment/LED pins (Defined to avoid compile error)
int pinA = A1; int pinB = 2; int pinC = 13; // Moved pinC to 13 to avoid matrix conflict
int pinD = A0; int pinE = A2; int pinF = 6;
int pinG = A3; int D1 = 0;
















// --- Effort Tracking ---
float rep1Accel = 0.0;
float currentRepPeak = 0.0;
float effortPercent = 100.0;








//Time Vars
unsigned long startTime = 0;
unsigned int seconds = 0;
unsigned int minutes = 0;
















void setup() {
  lcd.begin(16, 2);
  lcd.print("Hackathon 2026");
















  // Matrix Initialization
  matrix.begin(Din, Clk, CS, 1);
  matrix.borrar();
  matrix.escribirFrase("0"); // Initial count
















  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);
















  mpu.initialize();
  mpu.setSleepEnabled(false);
  delay(100);
















  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
















  // Set pin modes
  pinMode(pinA, OUTPUT); pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT); pinMode(pinD, OUTPUT);
  pinMode(pinE, OUTPUT); pinMode(pinF, OUTPUT);
  pinMode(pinG, OUTPUT); pinMode(D1, OUTPUT);
















  delay(2000);
  lcd.clear();
}
















void loop() {
  // --- Time Calculations ---
  if (repCount >= 1) {
    // Capture the start time only once, the moment the first rep happens
    if (startTime == 0) {
      startTime = millis();
    }
    // Calculate elapsed time relative to that start time
    unsigned long totSeconds = (millis() - startTime) / 1000;
    seconds = totSeconds % 60;
    minutes = totSeconds / 60;
  }
  else {
    // Ensure everything stays at zero if no reps have started
    startTime = 0;
    seconds = 0;
    minutes = 0;
  }
  //Reset
  if(repCount == 17){
    repCount = 0;
  }
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);




//absVelo = sqrt(sq(ax)+sq(ay)+sq(az));




  // Smooth the data
  smoothedZ = alpha * ((-ax / 16384) * 9.8) + (1.0 - alpha) * smoothedZ;
  Serial.println(smoothedZ);




  // Rep Detection Logic
  if (smoothedZ < LOWER_LIMIT && !isDown) {
    isDown = true;
  }




  if (smoothedZ > UPPER_LIMIT && isDown) {
    repCount++;
    isDown = false;




    // --- UPDATE MATRIX DISPLAY ---
    matrix.borrar();
    String countStr = String(repCount/2);
    if (repCount <= 16){
      matrix.escribirFrase(countStr.c_str());
    }




    if (repCount == 1) {
      rep1Accel = currentRepPeak;
      effortPercent = 100.0;
    } else {
      effortPercent = 100-(currentRepPeak/rep1Accel)*(currentRepPeak/rep1Accel)*100;
    }
    currentRepPeak = 0.0;
  }




  // Peak tracking
  if (isDown && smoothedZ > currentRepPeak && repCount % 2 == 0) {
    currentRepPeak = smoothedZ;
  }




  Serial.print("Z (g): ");
  Serial.print(smoothedZ, 2); Serial.println(", ");




  // LCD display
  lcd.setCursor(0, 0);
  lcd.print("Reps: ");
  lcd.print(repCount/2);
  lcd.print("   ");
  lcd.print(minutes);
  lcd.print(":");
  if(seconds < 10) lcd.print("0");
  lcd.print(seconds);
  lcd.print("    ");




  lcd.setCursor(0, 1);
  lcd.print("Effort: ");
  lcd.print(abs(effortPercent), 0);
  lcd.print("%   ");




  delay(50); // Improved responsiveness
}