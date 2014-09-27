const int numReadings = 10;
#include <Adafruit_NeoPixel.h>

#define PIN 6

int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
int led = 13;
int LDRPin = 0;
int MeterOut = 11;
double MeterCalibration= 11.625;
int PiezoPin = 5;    // No longer using a piezo. Using a MS24P/10 (digital)
int piezo_read = 0;
int piezo_initial = 0;
double gravity = 0.0;
double calibrated_gravity;
double half_t_squared = 0.0;
double height_to_fall = 0.7;
double s_minus_ut = 0.0;
const double initial_velocity = 0.19; //Measured experimentally
int timer;
int ldr_read;
boolean Freefall = false;
unsigned long CurrentTime = millis();
unsigned long StartTime = millis();
unsigned long ElapsedTime = CurrentTime - StartTime;


// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.


int calculate_average(){
  for (int thisReading = 0; thisReading < numReadings; thisReading++){
    delay(100);
    ldr_read = analogRead(LDRPin);
    total = total + ldr_read;
  }
  average = total / numReadings;
  return average;  
}

void setup()
{
  // initialize serial communication with computer:
  Serial.begin(9600);                   
  // initialize all the readings to 0: 
  pinMode(led, OUTPUT);
  pinMode(LDRPin,INPUT);
  pinMode(PiezoPin,INPUT);
  pinMode(MeterOut, OUTPUT);
  digitalWrite(led, HIGH);
  analogWrite(MeterOut, 255);
  delay(100); 
  average = calculate_average();
  piezo_initial = digitalRead(PiezoPin);
  analogWrite(MeterOut, 0);
  //  Serial.println(average); 
  strip.begin();
  strip.show(); // Initialize all pixels to 'off' 
}

void loop() {

  ElapsedTime = CurrentTime - StartTime;  
  // read from the sensor:  
  ldr_read = analogRead(LDRPin); 
  //  Serial.print("####Piezo####");
  //  Serial.print(piezo_value);
  //  Serial.print("####LDR####");
  //    Serial.println(ldr_read);

  if ((average - ldr_read >= 20) && (Freefall == false)){
    Freefall = true;
    StartTime = millis();
    //       Serial.print("\t StartTime = ");
    //       Serial.print(StartTime);
  }

  if (Freefall == true){ 
    CurrentTime = millis();
    ElapsedTime = CurrentTime - StartTime;
    //       Serial.print("\t ElapsedTime = ");
    //       Serial.println(ElapsedTime); 
    //       Serial.print("Freefall:");
    //       Serial.print("####LDR#####");
    //       Serial.print(average);
    //       Serial.print("\t");
    //       Serial.println(ldr_read);
    delay(1);
    piezo_read = digitalRead(PiezoPin);
    delay(1);
        analogWrite(MeterOut, (piezo_read/10.0));
//           Serial.print("####PIEZO\t");
//           Serial.print("\t");
//           Serial.println(piezo_read);
    if (piezo_read != piezo_initial){
      Freefall = false;

      CurrentTime = millis();
      ElapsedTime = CurrentTime - StartTime;
      //        Serial.print("\t StartTime = ");
      //        Serial.print(StartTime);
      //        Serial.print("\t CurrentTime = ");
      //        Serial.print(CurrentTime);
      //        Serial.print("####LANDED####");
//      Serial.println(piezo_read);
      //              ElapsedTime = (ElapsedTime/1000.0);
      //              Serial.print("ElapsedTime = ");
      //              Serial.println(ElapsedTime);  
      s_minus_ut = height_to_fall - (initial_velocity * (ElapsedTime/1000.0));               
      //              Serial.print("s minus ut ");
      //              Serial.println(s_minus_ut); 
      half_t_squared = (sq(ElapsedTime/1000.0))/2.0;
      gravity = s_minus_ut/half_t_squared;  
      //        Serial.print("\t Half T squared = ");
      //        Serial.println(half_t_squared); 
      Serial.print ("Gravity = ");
      Serial.print(gravity);
      Serial.println("ms^-2");
      delay(100);
      calibrated_gravity = gravity * MeterCalibration; 
      analogWrite(MeterOut, calibrated_gravity);
      //        Serial.print ("Calibrated Gravity = ");
      //        Serial.println(calibrated_gravity);
      rainbowCycle(2);
      analogWrite(MeterOut, 0);
      delay(100);
      colorWipe(strip.Color(0, 0, 0), 1);
      piezo_initial = digitalRead(PiezoPin);
      //      average = calculate_average();
    } 
    else if (ElapsedTime > 2000){
      //          Serial.println("Timeout");
      CurrentTime = millis();
      ElapsedTime = CurrentTime - StartTime;
      //         Serial.println(ElapsedTime);
      Freefall = false;
      //         average = calculate_average();
      delay(100);
      piezo_initial = digitalRead(PiezoPin);
    } 
    else { 
    }

  } // End of Freefall == true
}//End of void loop


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}




