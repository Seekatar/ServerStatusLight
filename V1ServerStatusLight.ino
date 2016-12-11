/*

 */

/**************************************************************************/
// start OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// On 32u4 or M0 Feathers, buttons A, B & C connect to 9, 6, 5 respectively
const int modeButtonPin = 9;   // cycle temp/pressure/humidy
const int systemButtonPin = 6; // cycle English/metric
const int dimPin = 5;          // toggle dim
// end OLED
/******************************************************************/

bool displayOk = false;

void initDisplay()
{
  // display init
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false); // initialize with the I2C addr 0x3C (for the 128x32)
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();

  // initialize the button pin as a input:
  pinMode(modeButtonPin, INPUT_PULLUP);
  pinMode(systemButtonPin, INPUT_PULLUP);
  pinMode(dimPin, INPUT_PULLUP);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Initialized");
  display.println("  display...");
  display.display();

  displayOk = true;
}

char logMsgBuffer[300];

// 4, 21 char lines
char logArray[4][22];
int topLine = 0;

void logMsg(const char *msg)
{
  Serial.println(msg);

  //  try
  {
    if (displayOk)
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextColor(WHITE);
      display.setTextSize(1);

      strncpy( logArray[topLine], msg, 21 );
      
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

#include "SystemStatus.h"
SystemStatus processor = SystemStatus(logMsg);

#include "StatusWheel.h"
StatusWheel wheel = StatusWheel(processor,logMsg);

void setup()
{
  initDisplay();

  processor.initialize();

  wheel.initialize();
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
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

}

void loop()
{
    processor.process();

    wheel.process();
    
    // wait a bit before retrying
    delay(10000);
}


