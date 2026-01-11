#include "wled.h"
#include "wordcloxel.h"
#include "time_word_convertor.h"
#include "cloxel_layout_en_v1.h"
#include "cloxel_layout_nl_v1.h"

// strings to reduce flash memory usage (used more than twice)
const char WordCloxel::_txtName[]  PROGMEM = "Word cloxel";
const char WordCloxel::_txtNameLower[]  PROGMEM = "wordcloxel";

/*
 * Word cloxel usermod to display the correct time & date in words on a cloxel matrix
 */
void WordCloxel::setup()
{
    // Do nothing
    m_pCloxelLayout = &s_layoutEN_V1;
}

/*
* loop() is called continuously. In this method we check if we are inside the configured period or not.
* If inside the brightness will be set to the configured value. If not, the default brightness is used.
*/
void WordCloxel::loop() 
{
    if (m_configEnabled) 
    {
        if (millis() - lastTime > 200) 
        { 
            ClockTimeWordConvertor::convert(m_pCloxelLayout, &m_sClockWords);
        
            // Remember last update
            lastTime = millis();
        }
    }
}

//
// Add a single word to the display/leds
// customParam can be any value, normal operation when ColorHandler is not overriden: customParam is the color
void WordCloxel::AddWordToLeds(const ledpos_t *pCurrentWord, CRGB defaultColor) 
{
    if (pCurrentWord == NULL) return;

    uint8_t charIndex = 0;
    ledpos_t ledPos = pCurrentWord[charIndex];
    while (ledPos.x >= 0 && ledPos.y >= 0)
    {
        // Led numbers are inverted left to right every other row:     
        strip.setPixelColorXY(ledPos.x, ledPos.y, defaultColor);
        
        // Next char
        charIndex++;
        ledPos = pCurrentWord[charIndex];
    }
}

/*
    * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
    * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
    * Commonly used for custom clocks (Cronixie, 7 segment)
    */
void WordCloxel::handleOverlayDraw()
{
    // check if usermod is active
    if (m_configEnabled)
    {
        Segment& seg1 = strip.getSegment(0);
        int factor = m_configBackgroundFade * 10 / 100; // TODO THIS IS NOT GOOD!
        for(int i = 0; i < 256; i++) 
        {
            uint32_t c1 = strip.getPixelColor(i);
            uint8_t r = byte(c1>>16), g = byte(c1>>8), b = byte(c1), w = byte(c1>>24); 
            strip.setPixelColor(i, RGBW32(r/factor, g/factor, b/factor, 0)); // <= This works better but colors will be screwed
            //strip.setPixelColor(i, color_fade(c1, m_configBackgroundFade)); // blank out the segment
        }   

        // loop over all leds
        CRGB color = RGBW32(255, 255, 255, 255);
        AddWordToLeds(m_sClockWords.pToPastWord, color);
        AddWordToLeds(m_sClockWords.pMinutesMainWord, color);
        AddWordToLeds(m_sClockWords.pMinutesRestWord, color);
        AddWordToLeds(m_sClockWords.pHalfWord, color);
        AddWordToLeds(m_sClockWords.pHourWord, color);

        AddWordToLeds(m_sClockWords.pDayWord, color);

        AddWordToLeds(m_sClockWords.pDayOfMonthWord, color);
        AddWordToLeds(m_sClockWords.pMonthWord, color);

        AddWordToLeds(m_sClockWords.pSecondLeds, color);
    }
}

/*
* addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
* Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
* Below it is shown how this could be used for e.g. a light sensor
*/
void WordCloxel::addToJsonInfo(JsonObject& root)
{
    // if "u" object does not exist yet wee need to create it
    JsonObject user = root["u"];
    if (user.isNull()) user = root.createNestedObject("u");

    //this code adds "u":{"ExampleUsermod":[20," lux"]} to the info object
    //int reading = 20;
    //JsonArray lightArr = user.createNestedArray(FPSTR(_name))); //name
    //lightArr.add(reading); //value
    //lightArr.add(F(" lux")); //unit

    // if you are implementing a sensor usermod, you may publish sensor data
    //JsonObject sensor = root[F("sensor")];
    //if (sensor.isNull()) sensor = root.createNestedObject(F("sensor"));
    //temp = sensor.createNestedArray(F("light"));
    //temp.add(reading);
    //temp.add(F("lux"));
}

/*
* Add our own config items
*/
void WordCloxel::addToConfig(JsonObject& root)
{
    JsonObject top = root.createNestedObject(F(_txtName));

    top[F("Active")] = m_configEnabled;
    top[F("Background fade")] = m_configBackgroundFade;
    // top[F("Start hour")] = configStartHour;
    // top[F("Start minute")] = configStartMinute;
    // top[F("End hour")] = configEndHour;
    // top[F("End minute")] = configEndMinute;
    // top[F("Brightness")] = configBrightness;
}

/*
* Add some extra information to the config items
*/
void WordCloxel::appendConfigData()
{
    // oappend(F("addInfo('")); oappend(_txtName); oappend(F(":Start hour', 1, '(0-23)');"));
    // oappend(F("addInfo('")); oappend(_txtName); oappend(F(":Start minute', 1, '(0-59)');"));
    // oappend(F("addInfo('")); oappend(_txtName); oappend(F(":End hour', 1, '(0-23)');"));
    // oappend(F("addInfo('")); oappend(_txtName); oappend(F(":End minute', 1, '(0-59)');"));
    // oappend(F("addInfo('")); oappend(_txtName); oappend(F(":Brightness', 1, '(%)');"));
}

/*
* Read the values of the config items
*/
bool WordCloxel::readFromConfig(JsonObject& root)
{
    JsonObject top = root[F(_txtName)];

    bool configComplete = !top.isNull();

    configComplete &= getJsonValue(top[F("Active")], m_configEnabled);
    configComplete &= getJsonValue(top[F("Background fade")], m_configBackgroundFade);
    // configComplete &= getJsonValue(top[F("Start hour")], configStartHour);
    // configComplete &= getJsonValue(top[F("Start minute")], configStartMinute);
    // configComplete &= getJsonValue(top[F("End hour")], configEndHour);
    // configComplete &= getJsonValue(top[F("End minute")], configEndMinute);
    // configComplete &= getJsonValue(top[F("Brightness")], configBrightness);

    return configComplete;
}

/*
* Definition of the main usermode class wordcloxel
*/
static WordCloxel usermod_wordcloxel;
REGISTER_USERMOD(usermod_wordcloxel);


// TODO:
//  - X,Y coordinates can be pre-calculated at compile time
//  - Let AddWordToLeds use an vector instead of a struct
//  - Show cloxel at startup for X seconds
//  - React to unconnected / connected => show no-wifi
