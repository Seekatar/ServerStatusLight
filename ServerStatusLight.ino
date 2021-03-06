#include <stdarg.h>

bool displayOk = false;
char logMsgBuffer[300];
#ifdef ARDUINO_SAMD_FEATHER_M0
const int STATUS_LED = 13;
elif defined(ESP_PLATFORM)
const int STATUS_LED = 5;
#else
const int STATUS_LED = 14;
#endif

#include "Esp32AnalogWrite.h"

#ifdef USE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 32)
  #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// On 32u4 or M0 Feathers, buttons A, B & C connect to 9, 6, 5 respectively
const int buttonA = 9;
const int buttonB = 6;
const int buttonC = 5;

void initDisplay()
{
  Serial.println("Starting display");

  // display init
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true); // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();

  // initialize the button pin as a input:
  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);
  pinMode(buttonC, INPUT_PULLUP);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Display ok");
  display.display();

  displayOk = true;
}

// 4, 21 char lines on display
char logArray[4][22];
int topLine = 0;
const int LINE_LEN = 21;


void logLine( const char *msg )
{
  //  try
  {
    if (displayOk)
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextColor(WHITE);
      display.setTextSize(1);

      strncpy( logArray[topLine], msg, LINE_LEN );

      if ( ++topLine > 3 )
        topLine = 0;

      int j = 0;
      for ( int i = topLine; i < 4; i++ )
        display.println(logArray[i]);
      for ( int i = 0; i < topLine; i++)
        display.println(logArray[i]);

      display.display();
    }
  }
  //  catch ( ... )
  //  {
  //    Serial.println("Exception using display.  Turning it off");
  //    displayOk = false;
  //  }
}
#else
void initDisplay() {}
void logLine( const char *msg ) {}
#endif

void logMsg(const char *msg, ...)
{
  va_list args;
  va_start( args, msg );
  vsnprintf( logMsgBuffer, 300, msg, args );

  Serial.println(logMsgBuffer);

  #if USE_OLED
  char *s = strtok( logMsgBuffer, "\r\n" );
  while ( s != NULL )
  {
    char *t = s;
    while ( strlen( t ) > LINE_LEN )
    {
      char line[22];
      strncpy( line, t, LINE_LEN );
      line[21] = '\0';
      logLine( line );
      t += LINE_LEN;
    }
    if ( strlen(t) > 0 )
      logLine(t);
    s = strtok( NULL, "\r\n");
  }
  #endif

  va_end(args);
}


#include "SystemStatus.h"
SystemStatus processor = SystemStatus(logMsg);

#define USE_CUBE
#ifdef USE_CUBE
#include "StatusCube.h"
StatusCube light = StatusCube(processor,logMsg, A0);
#else
#include "StatusWheel.h"
StatusWheel light = StatusWheel(processor,logMsg, A0);
#endif

void setup()
{
  pinMode( STATUS_LED, OUTPUT );
  analogWriteSetup(STATUS_LED);
  analogWrite( STATUS_LED, 150 );

  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  unsigned long startTime = millis();
  while (!Serial)
  {
    delay(5);

    // wait for 5 seconds for serial port to connect. Needed for native USB port only
    if ( millis() - startTime > 5000 )
    {
      if ( !Serial )
      {
        logMsg( "No Serial connection" );
        delay(3000);
        break;
      }
    }
  }
  Serial.println("Serial started");
  initDisplay();

  light.initialize();

  processor.initialize();
}

const int minLEDSetting = 30;
const int maxLEDSetting = 200;
int lastLEDSetting = minLEDSetting;
int inc = 2;
unsigned int lastLEDTime = millis();

void fadeLed()
{
    if ( millis() - lastLEDTime > 10 )
    {
      lastLEDTime = millis();
      lastLEDSetting += inc;
      if ( lastLEDSetting > maxLEDSetting )
      {
        lastLEDSetting = maxLEDSetting;
        inc = -inc;
      }
      else if ( lastLEDSetting < minLEDSetting )
      {
        lastLEDSetting = minLEDSetting;
        inc = -inc;
      }
    }
    //logMsg( "LastLedsetting %d", lastLEDSetting );
    analogWrite( STATUS_LED, lastLEDSetting );
}

void loop()
{
    light.process( processor.process() );

    fadeLed();
}
