#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(4);

#include <BMP280_DEV.h>

unsigned long CurrentTime;
unsigned long PreviousDerivativeTime;
float PreviousHeight;
const int DerivativeAverageSamplingCount = 2;
float DerivativeArray[DerivativeAverageSamplingCount];
int newDerivativeIndex = 0;

const float VerticalSpeedZero = -0.5;
const float VerticalSpeedTolerance = 0.5;
const float VerticalSpeedPositiveThreshold = VerticalSpeedZero + VerticalSpeedTolerance;
const float VerticalSpeedNegativeThreshold = VerticalSpeedZero - VerticalSpeedTolerance;

float temperature, pressure, altitude;
BMP280_DEV bmp280;

void setup() 
{
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  bmp280.begin(BMP280_I2C_ALT_ADDR); 
  //bmp280.setPresOversampling(OVERSAMPLING_X16);
  //bmp280.setTempOversampling(OVERSAMPLING_X1);
  bmp280.setIIRFilter(IIR_FILTER_4);
  bmp280.setTimeStandby(TIME_STANDBY_500MS);
  bmp280.startNormalConversion();

  while(!bmp280.getMeasurements(temperature, pressure, altitude))
  {
    PreviousHeight = altitude;
  }
  PreviousHeight = altitude;
}

void loop() 
{
  if (bmp280.getMeasurements(temperature, pressure, altitude))
  {
    
    CurrentTime = millis();
    float timeInterval = (CurrentTime - PreviousDerivativeTime) * 0.001;
    float newDerivative = (altitude - PreviousHeight) / timeInterval;
    PreviousHeight = altitude;
    PreviousDerivativeTime = CurrentTime;

    DerivativeArray[newDerivativeIndex] = newDerivative;

    if(newDerivativeIndex < DerivativeAverageSamplingCount)
    {
      newDerivativeIndex++;
    }else
    {
      newDerivativeIndex = 0;
    }

    float AverageDerivative = 0;
    for (int i = 0; i < DerivativeAverageSamplingCount; i++)
    {
        AverageDerivative += DerivativeArray[i]/((float)DerivativeAverageSamplingCount);
    }

    display.clearDisplay();

    //Show text: vertical speed + altitude
    display.setCursor(display.width()/2 - 14, 0);
    display.println((String) StringSign(AverageDerivative));
    display.setCursor(display.width()/2 - 7, 0);
    display.println((String) abs(AverageDerivative));
    display.setCursor(0, 25);
    display.println("Alt: " + (String) altitude);

    //Set display Vertical speed coef
    float coef = 0.2;
    
    //Show the Vertical speed trigger threshold
    int x = display.width()/2 + ((display.width() - 32)/2) * coef * VerticalSpeedPositiveThreshold;
    display.drawLine(x, 15, x, 17, WHITE);

    //Invert screen when threshold is passed
    if(AverageDerivative >= VerticalSpeedPositiveThreshold)
    {
      display.invertDisplay(true);
    }else
    {
      display.invertDisplay(false);
    }
    
    //Draw the vertical speed line
    float derivativeLine = AverageDerivative * coef;
    display.drawLine(display.width()/2, 16, display.width()/2 + ((display.width() - 32)/2) * derivativeLine, 16, WHITE);
    display.display();
    
    //Serial.print(derivative * 50);
    //Serial.print(",");
    //Serial.print(altitude);
    //Serial.println();
  }
}

String StringSign(float value)
{
  if(value < 0)
  {
    return "-";
  }else
  {
    return "";
  }
}
