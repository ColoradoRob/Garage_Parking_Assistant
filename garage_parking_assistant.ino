#include <Adafruit_NeoPixel.h> // Lights
#include <NewPing.h> // Ultrasound distance sensor
#include <MillisTimer.h>
#include <Preferences.h> // persistent storage for the StopDistance

// NeoPixel bars
#define LED_PIN        D7
#define LED_COUNT      34
// Ultrasound Sensor
#define TRIG_PIN                D9
#define ECHO_PIN                D10
#define MAX_DISTANCE            400     // HC-SR04 max distance is 400CM
#define DISTANCE_TOLERANCE      5       // tolerance to determine if the car is stopped (CENTIMETERS)

#define LED_TIMEOUT             10000   // in milliseconds, time before lights go out (sleep) once car stopped
#define BUTTON_PIN              D6       // push-button for setting stopdistance

const int startdistance =       400;    // distance from sensor to begin scan as car pulls in (CENTIMETERS)
int stopdistance =              0;      // parking position from sensor (CENTIMETERS); either specify here, or leave as zero and set dynamically with push-button
const int durationarraysz =     20;     // number of measurements to take per cycle

int distance, previous_distance, i;
unsigned int uS; // raw ultrasound sensor output

Preferences prefs;
uint16_t stopdistance_prefs = 10;
MillisTimer timer = MillisTimer(LED_TIMEOUT);
bool LED_sleep;

int delayval = 500;
int currentColor = 0;
int bars_to_light = LED_COUNT;
int relative_distance = 0;
int range = startdistance - stopdistance;
int warn = (float)LED_COUNT * 0.3;

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE); 
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint32_t green = strip.Color(0, 255, 0);
uint32_t yellow = strip.Color(255, 255, 0);
uint32_t red = strip.Color(255, 0, 0);
uint32_t color = green;

void sleepmode_start(MillisTimer &mt)
{
  strip.clear();
  strip.show();
  timer.reset();
  LED_sleep = true;
}

void setup() {
  strip.begin(); 
  strip.setBrightness(100); 
  strip.clear();
  strip.show();  
  LED_sleep = false;
  Serial.begin(9600);
  // NVRam storage for the preferred stopdistance
  prefs.begin("settings", false);
  if (!stopdistance)
  {
    stopdistance = prefs.getUInt("stopDist", 10);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  timer.setInterval(LED_TIMEOUT);
  timer.expiredHandler(sleepmode_start);
  timer.setRepeats(0);
}

void loop() {
  timer.run();

  uS = sonar.ping_median(durationarraysz, MAX_DISTANCE);
  distance = sonar.convert_cm(uS);

  // Check if we've stopped moving
  if (abs(distance - previous_distance) < DISTANCE_TOLERANCE)
  {      
    if (!timer.isRunning() && !LED_sleep)
    {
      timer.start();
    }
  }
  else // movement beyond threshold, stop timer and turn off sleep
  {
    previous_distance = distance;
    timer.reset();
    LED_sleep = false;
  }
  // If buttonpress, save the current distance as our preferred stopdistance
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    stopdistance = distance;
    range = startdistance - stopdistance;
    // Replacement for: eeprom_write_word(&stopdistance_ee, stopdistance);
    prefs.putUInt("stopDist", stopdistance);
  }

  relative_distance = distance - stopdistance;
  bars_to_light = (int)(((float)relative_distance / range) * LED_COUNT);
  if (LED_sleep)
  {
    strip.clear();
    strip.show();
  }
  else
  { 
    color = green;
    if(bars_to_light < 4){ // TODO - make this %age of total lights
      color = red;
    } else if(bars_to_light <= warn){
      color = yellow;
    }

    if(bars_to_light > 0){
      strip.clear();
      for(i=0; i < bars_to_light; i++){
        strip.setPixelColor(i, color);
      }
      strip.show();
    } else {
      if(distance == 0){ // no signal
        strip.fill(green);
        strip.show();
      } 
      else 
      {
        strip.fill(red);
        strip.show();
      }
    }
  }
  delay(delayval);
}
