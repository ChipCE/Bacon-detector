/*
    Read the sensors every 100ms and increase the status counter of sensor is condition matched
    If any of the sensor counter is above the threshold, turn on the BUZZER
    When the buzzer is on, user have SHUTDOWN_PENDING(time) to Disable the shutdown by pressing the button
    if the button is pressed, it will ignore all warning for USER_ACTION_IGNORE_TIME
    If the button is not pressed , the device will send shutdown signal after SHUTDOWN_PENDING is over
*/

#include <DHT.h>
#include <Adafruit_NeoPixel.h>

// @PIN_CONFIG
#define BUTTON_PIN 16
#define MOTION_PIN A0
#define SMOKE_PIN 14
#define FLAME_PIN 12
#define BUZZER_PIN 0
#define TEMP_PIN 13
#define NEOPIXEL_PIN 2
#define PIXEL_COUNT 1
Adafruit_NeoPixel pixels(PIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

DHT dht(TEMP_PIN, DHT22);

// @TIMER_CONFIG
#define SAMPLING_INTERVAL 1000

#define SMOKE_WARNING_THRESHOLD 3
#define FLAME_WARNING_THRESHOLD 3
#define TEMP_WARNING_THRESHOLD 60
#define TEMP_WARNING_VALUE 40
#define SHUTDOWN_PENDING 60
#define USER_ACTION_IGNORE_TIME 5*60 //5 mins

// Neopixel color config {R,G,B,(W)}
int NEOPIXEL_INIT_COLOR[] = {0, 255, 0, 255};
int NEOPIXEL_WARNING_COLOR[] = {255, 0, 0 ,255};
int NEOPIXEL_DIM_COLOR[] = {0, 0, 255 , 128};
int NEOPIXEL_ACTIVE_COLOR[] = {255, 255, 255, 255};
int NEOPIXEL_USER_IGNORE_COLOR[] = {255, 255, 0, 255};
#define NEOPIXEL_DIM_TIMEOUT 3*600

// var
bool _global_warning = false;
bool _flame = false;
int _flame_warning = 0;
bool _smoke = false;
int _smoke_warning = 0;
int _temp = 0;
int _hum = 0;
int _temp_warning = 0;
bool _motion;

int _ignore_warning = 0;
int _neopixel_timeout = 0;
unsigned long _lastRead = 0;

void readSensor();
void initSensors();
void buzzerWarningHandle();
void networkWarningHandle();
void neopixelHandle();
void setPixelColor(int *color);
void readSensor();
void networkReportHandle();
void serialReportHandle();
void drawLcdTemplate();
void drawLcdData();


void setup()
{
    Serial.begin(115200);
    initSensors();
}

void loop()
{
    if( (millis() - _lastRead >= SAMPLING_INTERVAL) || (_lastRead > millis()) )
    {
        readSensor();
        buzzerWarningHandle();
        neopixelHandle();
        serialReportHandle();
    }
}


void initSensors()
{
    pinMode(BUTTON_PIN,INPUT_PULLUP);
    pinMode(MOTION_PIN,INPUT);
    pinMode(SMOKE_PIN,INPUT);
    pinMode(FLAME_PIN,INPUT);
    pinMode(BUZZER_PIN,OUTPUT);
    digitalWrite(BUZZER_PIN,HIGH);
    dht.begin();
    pixels.begin();
    pixels.clear();


    //waitForSensorInit();
}

void buzzerWarningHandle()
{
    if ( _global_warning && (_ignore_warning == 0))
    {
        //turn on 
        digitalWrite(BUZZER_PIN,LOW);
    }

    if ( !_global_warning || ( _ignore_warning > 0 ))
    {
        //turn off
        digitalWrite(BUZZER_PIN,HIGH);
    }
}

void setPixelColor(int *color)
{
    for(int i=0; i<PIXEL_COUNT; i++)
    {
        pixels.setPixelColor(i, pixels.Color(int(color[0]), int(color[1]), int(color[2]), int(color[3])));
        pixels.show();
    }
}

void neopixelHandle()
{
    if(_global_warning)
    {
        if(_ignore_warning > 0)
            setPixelColor(NEOPIXEL_INIT_COLOR);
        else
            setPixelColor(NEOPIXEL_INIT_COLOR);
    }
    else
    {
        if (_neopixel_timeout >0)
            setPixelColor(NEOPIXEL_INIT_COLOR);
        else
            setPixelColor(NEOPIXEL_INIT_COLOR);
    }
}


void readSensor()
{
    _lastRead = millis();

    if (_ignore_warning > 0)
        _ignore_warning--;

    _smoke = !digitalRead(SMOKE_PIN);
    if (_smoke)
        if( _smoke_warning < SMOKE_WARNING_THRESHOLD)
            _smoke_warning++;
    else
        if( _smoke_warning > 0)
            _smoke_warning--;

    _flame = !digitalRead(FLAME_PIN);
    if (_flame)
        if( _flame_warning < FLAME_WARNING_THRESHOLD)
            _flame_warning++;
    else
        if( _flame_warning > 0)
            _flame_warning--;

    _motion = digitalRead(MOTION_PIN);

    if (_motion)
    {
        _neopixel_timeout = NEOPIXEL_DIM_TIMEOUT;
    }
    else
    {
        if( _neopixel_timeout > 0)
            _neopixel_timeout--;
    }

    _temp = int(dht.readTemperature());
    _hum = int(dht.readHumidity());

    if ( _temp >= TEMP_WARNING_VALUE )
        if( _temp_warning < TEMP_WARNING_THRESHOLD)
                _temp_warning++;
    else
        if( _temp_warning > 0)
                _temp_warning--;

    if (( _smoke_warning >= SMOKE_WARNING_THRESHOLD ) || ( _flame_warning >= FLAME_WARNING_THRESHOLD) || ( _temp_warning >= TEMP_WARNING_THRESHOLD))
        _global_warning = true;

    if ( ( _smoke_warning == 0 ) && ( _flame_warning == 0 ) && ( _temp_warning == 0 ))
        _global_warning = false;

    if ( (digitalRead(BUTTON_PIN) == LOW) && _global_warning)
        _ignore_warning = USER_ACTION_IGNORE_TIME;
}


void networkReportHandle()
{

}

void serialReportHandle()
{
    Serial.println("SENSORS DATA REPORT");
    Serial.print("SMOKE : "); Serial.println(_smoke);
    Serial.print("FLAME : "); Serial.println(_flame);
    Serial.print("MOTION : "); Serial.println(_motion);
    Serial.print("TEMP : "); Serial.println(_temp);
    Serial.print("HUM : "); Serial.println(_hum);

    Serial.println("");

    Serial.println("THRESHOLD VALUE");
    Serial.print("_global_warning"); Serial.println(_global_warning);
    Serial.print("_flame_warning"); Serial.println(_flame_warning);
    Serial.print("_smoke_warning"); Serial.println(_smoke_warning);
    Serial.print("_temp_warning"); Serial.println(_temp_warning);
    Serial.print("_ignore_warning"); Serial.println(_ignore_warning);
    Serial.print("_neopixel_timeout"); Serial.println(_neopixel_timeout);
}


void drawLcdTemplate()
{}

void drawLcdData()
{}
