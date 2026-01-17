#include "wled.h"
#include "wordcloxel.h"
#include "time_word_convertor.h"
#include "cloxel_layout_en_v1.h"
#include "cloxel_layout_nl_v1.h"

// strings to reduce flash memory usage (used more than twice)
const char WordCloxel::_txtName[]  PROGMEM = "Wordcloxel";
const char WordCloxel::_txtNameLower[]  PROGMEM = "wordcloxel";
const char WordCloxel::_txtMsg[]  PROGMEM = "msg";
const char WordCloxel::_txtTime[]  PROGMEM = "time";
constexpr int MINIMUM_INITIALIZE_TIME = 5000;
constexpr int CLOXEL_STARTUP_CYCLE_TICKS = 200;
constexpr int CLOXEL_STARTUP_CYCLES = 4;
constexpr int CLOXEL_STARTUP_TOTAL_TICKS = CLOXEL_STARTUP_CYCLE_TICKS * CLOXEL_STARTUP_CYCLES + 100;
constexpr uint32_t CLOXEL_STARTUP_COLOR = RGBW32(0xFF, 0x7E, 0, 0);
constexpr float_t NUMBER_OF_SECOND_PULSES_PER_MINUTE = 20.0f;
constexpr uint32_t NUMBER_OF_MILLIS_PER_PULSE = 3000;

constexpr char MY_RIPPLE_DATA[] PROGMEM = "Ripple@!,Wave #,Blur,,,,Overlay;,!;!;1;c1=0";

//constexpr float_t SECOND_PULSE_DURATION = 60000.0 / NUMBER_OF_SECOND_PULSES_PER_MINUTE;

namespace
{
    String colorToHexString(CRGB c) 
    {
        char buffer[9];
        sprintf(buffer, "%02X%02X%02X", c.r, c.g, c.b);
        return buffer;
    }
   
    bool hexStringToColor(String const &hexString, uint32_t &outputColor, uint32_t defaultColor)
    {
        char *ep;
        uint32_t color = strtoul(hexString.c_str(), &ep, 16);
        if (*ep == 0) 
        {
            outputColor = color;
            return true;
        }
        else 
        {
            outputColor = defaultColor;
            return false;
        }
    }
}

/*
 * Word cloxel usermod to display the correct time & date in words on a cloxel matrix
 */
void WordCloxel::setup()
{
    // Do nothing
    m_pCloxelLayout = &s_layoutEN_V1;
    m_fInitialized = true;
    m_displayMode = EDisplayModes::INITIALIZING;
    m_startOfInitializedTime = millis();

    // Initialize matrix layout
    WS2812FX::Panel pan;
    pan.width = 16;
    pan.height = 16;
    pan.rightStart = true;
    pan.serpentine = true;
    if (strip.panel.size() == 0)
        strip.panel.push_back(pan);
    strip.isMatrix = true;
    strip.setUpMatrix();
    strip.resetSegments();

    strip.setBrightness(128);

    // Select initial effect
    Segment& seg0 = strip.getSegment(0);
    seg0.palette = 50; // <-- TODO: Get palette working!
    seg0.speed = 128;
    seg0.intensity = 128;
    seg0.custom1 = 0;
    seg0.mode = FX_MODE_RIPPLE;

    seg0.fadeToBlackBy(20);

    seg0.beginDraw(0xFFFFU);             

    //strip.setBrightness(128);
}


/* 
* Called when WiFi is (re)connected
*/
void WordCloxel::connected()
{
}


/*
* loop() is called continuously. In this method we check if we are inside the configured period or not.
* If inside the brightness will be set to the configured value. If not, the default brightness is used.
*/
void WordCloxel::loop() 
{
    if (m_configEnabled) 
    {
        if (millis() - m_lastUpdateTime > 200) 
        { 
            m_vecWordsTime.clear();
            m_vecWordsDate.clear();
            m_vecWordsWeekday.clear();
            m_vecWordsSecond.clear();
            
            switch (m_displayMode)
            {
                case EDisplayModes::INITIALIZING:                    
                    if (m_displayCounter > CLOXEL_STARTUP_TOTAL_TICKS)
                    {
                        m_displayCounter = 0;
                        if (WLED_CONNECTED && year(localTime) > 2025)
                        {
                            m_displayMode = EDisplayModes::NORMAL;

                            // Select a default effect
                            Segment& seg0 = strip.getSegment(0);                            
                            seg0.palette = 50; // Palette is NOT working! Aurora (55 = Aurora_gp)
                            seg0.speed = 20;
                            seg0.intensity = 128;
                            seg0.mode = FX_MODE_2DPOLARLIGHTS;
                        }
                        else
                        {
                            m_displayMode = EDisplayModes::NOWIFI;
                        }
                    }
                    else
                    {                        
                        m_vecWordsTime.push_back(m_pCloxelLayout->extra.myriadclock);
                        m_vecWordsTime.push_back(m_pCloxelLayout->extra.word);
                    }
                    break;

                case EDisplayModes::NOWIFI:
                    m_vecWordsTime.push_back(m_pCloxelLayout->extra.no);
                    m_vecWordsTime.push_back(m_pCloxelLayout->extra.wifi);
                    m_displayMode = EDisplayModes::NOWIFI;
                    if (millis() > 15000)
                    {
                        esp_restart();
                    }
                    break;

                case EDisplayModes::NORMAL:
                    // Determine the words to display
                    ClockTimeWordConvertor::convertHoursAndMinutes(m_pCloxelLayout, m_vecWordsTime);
                    ClockTimeWordConvertor::convertDate(m_pCloxelLayout, m_vecWordsDate);
                    ClockTimeWordConvertor::convertWeekDay(m_pCloxelLayout, m_vecWordsWeekday);
                    ClockTimeWordConvertor::convertSeconds(m_pCloxelLayout, m_vecWordsSecond);
                    break;

                default:
                    break;
            }
        
            // Remember last update
            m_lastUpdateTime = millis();
        }
    }
}

//
// Add a single word to the display/leds
// customParam can be any value, normal operation when ColorHandler is not overriden: customParam is the color
void WordCloxel::AddWordsToLeds(std::vector<const ledpos_t*> rVecWords, CRGB color) 
{
    for(const ledpos_t* pCurrentWord : rVecWords)
    {
        uint8_t charIndex = 0;
        ledpos_t ledPos = pCurrentWord[charIndex];
        while (ledPos.x >= 0 && ledPos.y >= 0)
        {
            // Led numbers are inverted left to right every other row:     
            strip.setPixelColorXY(ledPos.x, ledPos.y, color);
            
            // Next char
            charIndex++;
            ledPos = pCurrentWord[charIndex];
        }
    }
}

/*
* handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
* Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
* Commonly used for custom clocks (Cronixie, 7 segment)
*/
void WordCloxel::handleOverlayDraw()
{
    // Only when this module is enabled
    if (m_configEnabled)
    {
        // At startup, always show cloxel text
        if (m_displayMode == EDisplayModes::INITIALIZING)
        {
            //strip.fill(BLACK);
            m_displayCounter++;
            if (m_displayCounter < CLOXEL_STARTUP_CYCLE_TICKS * CLOXEL_STARTUP_CYCLES)
            {
                float value = 127 * (1.0f - cos_approx(m_displayCounter * M_TWOPI / (float_t)CLOXEL_STARTUP_CYCLE_TICKS));
                AddWordsToLeds(m_vecWordsTime, color_fade(CLOXEL_STARTUP_COLOR, (uint8_t)value));
            }
        }
        else if (m_displayMode == EDisplayModes::NOWIFI)
        {
            //strip.fill(BLACK);
            m_displayCounter++;
            float value = 127 * (cos_approx(m_displayCounter * M_TWOPI / (float_t)CLOXEL_STARTUP_CYCLE_TICKS) + 1.0f);
            CRGB color = RGBW32((int) value, 0, 0, 0);
            AddWordsToLeds(m_vecWordsTime, color);
        }
        else
        {
            Segment& seg0 = strip.getSegment(0);
            //int factor = m_configBackgroundFade * 10 / 100; // TODO THIS IS NOT GOOD!
            for(int i = 0; i < seg0.width() * seg0.height(); i++) 
            {
                uint32_t c1 = strip.getPixelColor(i);
                uint8_t r = byte(c1>>16), g = byte(c1>>8), b = byte(c1), w = byte(c1>>24); 
                //strip.setPixelColor(i, RGBW32(r/factor, g/factor, b/factor, 0)); // <= This works better but colors will be screwed
                strip.setPixelColor(i, color_fade(c1, m_configBackgroundFade)); // blank out the segment
            }   
            // Add all words to the clock
            AddWordsToLeds(m_vecWordsTime, m_configTimeColor);
            AddWordsToLeds(m_vecWordsWeekday, m_configWeekdayColor);
            AddWordsToLeds(m_vecWordsDate, m_configDateColor);

            unsigned long milliOnly = millis() % NUMBER_OF_MILLIS_PER_PULSE; 
            float value = 127 * (1.0f + cos_approx(milliOnly * M_TWOPI / (float_t)NUMBER_OF_MILLIS_PER_PULSE));
            AddWordsToLeds(m_vecWordsSecond, color_fade(m_configTimeColor, (uint8_t)value));
        }
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
* readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
* Values in the state object may be modified by connected clients
*/
void WordCloxel::readFromJsonState(JsonObject& root)
{
    if (!m_fInitialized) return;  // prevent crash on boot applyPreset()

    JsonObject usermod = root[FPSTR(_txtNameLower)];
    if (!usermod.isNull()) 
    {
        if (usermod[FPSTR(_txtMsg)] && usermod[FPSTR(_txtMsg)].is<String>()) 
        {
            m_displayMsg = usermod[FPSTR(_txtMsg)].as<String>();
        }
        if (usermod[FPSTR(_txtTime)] && usermod[FPSTR(_txtTime)].is<int>()) 
        {
            m_displayTime = usermod[FPSTR(_txtTime)].as<int>();
            if (m_displayTime > 0) m_displayTime++; // Make sure we show it at least X seconds
            DEBUG_PRINTF("Incoming message '%s' for %d s\n", m_displayMsg.c_str(), m_displayTime);
        }
    }
}

/*
* Add our own config items
*/
void WordCloxel::addToConfig(JsonObject& root)
{
    JsonObject top = root.createNestedObject(F(_txtName));

    top[F("Active")] = m_configEnabled;
    top[F("Cloxel Layout")] = m_configLayout;
    top[F("Time color (RRGGBB)")] = colorToHexString(m_configTimeColor);
    top[F("Weekday color (RRGGBB)")] = colorToHexString(m_configWeekdayColor);
    top[F("Date color (RRGGBB)")] = colorToHexString(m_configDateColor);
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
    //oappend(F("dd=addDropdown('")); oappend(_txtName); oappend(F("','Cloxel layout');"));
    //oappend(F("addOption(dd,'English V1',0);"));
    //oappend(F("addOption(dd,'Dutch V2',1);"));

    oappend(F("dd=addDropdown('")); oappend(String(FPSTR(_txtName)).c_str()); oappend(F("','Cloxel layout');"));
    oappend(F("addOption(dd,'Nothing',0);"));
    oappend(F("addOption(dd,'Everything',42);"));

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

    configComplete &= getJsonValue(top[F("Cloxel layout")], m_configLayout);
    
    String tempColor;
    configComplete &= getJsonValue(top[F("Time color (RRGGBB)")], tempColor, F("FFFFFF")) && hexStringToColor(tempColor, m_configTimeColor, 0xFFFFFF);
    configComplete &= getJsonValue(top[F("Weekday color (RRGGBB)")], tempColor, F("FFFFFF")) && hexStringToColor(tempColor, m_configWeekdayColor, 0xFFFFFF);
    configComplete &= getJsonValue(top[F("Date color (RRGGBB)")], tempColor, F("FFFFFF")) && hexStringToColor(tempColor, m_configDateColor, 0xFFFFFF);

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
//  - DONE: Let AddWordToLeds use an vector instead of a struct
//  - Show cloxel at startup for X seconds
//  - React to unconnected / connected => show no-wifi
//     - Or use RTC time
//  - use colorFromHexString
//  - Make sure the timings work on a clean  ESP32
//  - Setup correct LED matric layout on a clean ESP32

// Debugging using:
//DEBUG_PRINTF_P(PSTR(" value %df\n"), color);
