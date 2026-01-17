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
}

/*
 * Word cloxel usermod to display the correct time & date in words on a cloxel matrix
 */
void WordCloxel::setup()
{
    // Do nothing
    m_pCloxelLayout = &s_layoutEN_V1;
    m_fInitialized = true;
    m_displayMode = EDisplayMode::DM_INITIALIZING;
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
    //strip.resetSegments(); // Do not use: palette is screwed up!

    strip.setBrightness(128);

    // Select initial effect
    Segment& seg0 = strip.getSegment(0);
    seg0.palette = 50; 
    seg0.speed = 40;
    seg0.intensity = 128;
    seg0.custom1 = 0;
    seg0.mode = FX_MODE_RIPPLE;

    // seg0.fadeToBlackBy(20);

    //seg0.beginDraw(0xFFFFU);
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
        unsigned long currentTime = millis();
        if (currentTime - m_lastUpdateTime > 100) 
        { 
            // Remember last update
            m_lastUpdateTime = currentTime;

            // Clear previous words
            m_vecWordsTime.clear();
            m_vecWordsDate.clear();
            m_vecWordsWeekday.clear();
            m_vecWordsSecond.clear();
            m_vecWordsExtra.clear();
            
            switch (m_displayMode)
            {
                case EDisplayMode::DM_INITIALIZING:         
                    DEBUG_PRINTF("%ld, Counter %d\n", currentTime, m_displayCounter);
                    if (m_displayCounter > CLOXEL_STARTUP_TOTAL_TICKS)
                    {
                        m_displayCounter = 0;
                        if (WLED_CONNECTED && year(localTime) > 2025)
                        {
                            m_displayMode = EDisplayMode::DM_NORMAL;

                            // Select a default effect
                            Segment& seg0 = strip.getSegment(0);                            
                            seg0.palette = 50; // Palette is NOT working! Aurora (55 = Aurora_gp)
                            seg0.speed = 20;
                            seg0.intensity = 128;
                            seg0.mode = FX_MODE_2DPOLARLIGHTS;
                        }
                        else
                        {
                            m_displayMode = EDisplayMode::DM_NOWIFI;
                        }
                    }
                    else
                    {                        
                        m_vecWordsTime.push_back(m_pCloxelLayout->extra.myriadclock);
                        m_vecWordsTime.push_back(m_pCloxelLayout->extra.word);
                    }
                    break;

                case EDisplayMode::DM_NOWIFI:
                    m_vecWordsTime.push_back(m_pCloxelLayout->extra.no);
                    m_vecWordsTime.push_back(m_pCloxelLayout->extra.wifi);
                    m_displayMode = EDisplayMode::DM_NOWIFI;
                    if (currentTime > 15000)
                    {
                        esp_restart();
                    }
                    break;

                case EDisplayMode::DM_NORMAL:
                    // Determine the words to display
                    ClockTimeWordConvertor::convertHoursAndMinutes(m_pCloxelLayout, m_vecWordsTime);
                    ClockTimeWordConvertor::convertDate(m_pCloxelLayout, m_vecWordsDate);
                    ClockTimeWordConvertor::convertWeekDay(m_pCloxelLayout, m_vecWordsWeekday);
                    ClockTimeWordConvertor::convertSeconds(m_pCloxelLayout, m_vecWordsSecond);

                    if (currentTime < m_messageEndTime)
                    {
                        switch (m_messageMode)
                        {
                            case EMessageMode::EM_GOODMORNING:
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.goodmorning);
                                break;
                            case EMessageMode::EM_BREAKFAST:
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.breakfast);
                                break;
                            case EMessageMode::EM_LUNCH:
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.lunch);
                                break;
                            case EMessageMode::EM_DINNER:
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.dinner);
                                break;
                            case EMessageMode::EM_WORDCLOXEL:
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.myriadbits);
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.word);
                                break;
                            default:
                                break;
                        }
                    }
                    else
                    {
                        m_messageMode = EMessageMode::EM_NONE;
                        m_messageEndTime = 0;
                    }

                    break;

                default:
                    break;
            }            
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
        if (m_displayMode == EDisplayMode::DM_INITIALIZING)
        {
            strip.fill(BLACK);
            m_displayCounter++;
            if (m_displayCounter < CLOXEL_STARTUP_CYCLE_TICKS * CLOXEL_STARTUP_CYCLES)
            {
                float value = 127 * (1.0f - cos_approx(m_displayCounter * M_TWOPI / (float_t)CLOXEL_STARTUP_CYCLE_TICKS));
                AddWordsToLeds(m_vecWordsTime, color_fade(CLOXEL_STARTUP_COLOR, (uint8_t)value));
            }
        }
        else if (m_displayMode == EDisplayMode::DM_NOWIFI)
        {
            strip.fill(BLACK);
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
            AddWordsToLeds(m_vecWordsSecond, color_fade((uint32_t) m_configTimeColor, (uint8_t)value));

            AddWordsToLeds(m_vecWordsExtra, CRGB::Red);
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
        if (usermod[FPSTR(_txtTime)] && usermod[FPSTR(_txtTime)].is<int>()) 
        {
            int messageTimeInSeconds = usermod[FPSTR(_txtTime)].as<int>();
            m_messageEndTime = millis() + messageTimeInSeconds * 1000;  
        }
        if (usermod[FPSTR(_txtMsg)] && usermod[FPSTR(_txtMsg)].is<String>()) 
        {
            String displayMsg = usermod[FPSTR(_txtMsg)].as<String>();
            displayMsg.toLowerCase();
            // TODO Make map?
            if (displayMsg == "goodmorning") 
            {
                m_messageMode = EMessageMode::EM_GOODMORNING;
            }
            else if (displayMsg == "breakfast") 
            {
                m_messageMode = EMessageMode::EM_BREAKFAST;
            }
            else if (displayMsg == "lunch") 
            {
                m_messageMode = EMessageMode::EM_LUNCH;
            }
            else if (displayMsg == "dinner") 
            {
                m_messageMode = EMessageMode::EM_DINNER;
            }
            else if (displayMsg == "wordcloxel") 
            {
                m_messageMode = EMessageMode::EM_WORDCLOXEL;
            }
            DEBUG_PRINTF("Incoming message '%s' for %d s\n", displayMsg.c_str(), m_messageTime);
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
    top[F("Layout")] = m_configLayout;
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
    oappend(F("dd=addDropdown('")); oappend(_txtName); oappend(F("','Layout');"));
    oappend(F("addOption(dd,'English V1',0);"));
    oappend(F("addOption(dd,'Dutch V2',1);"));

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
    configComplete &= getJsonValue(top[F("Layout")], m_configLayout);

    String tempColor;
    configComplete &= getJsonValue(top[F("Time color (RRGGBB)")], tempColor, F("FFFFFF")) && colorFromHexString(m_configTimeColor.raw, tempColor.c_str());
    configComplete &= getJsonValue(top[F("Weekday color (RRGGBB)")], tempColor, F("FFFFFF")) && colorFromHexString(m_configWeekdayColor.raw, tempColor.c_str());
    configComplete &= getJsonValue(top[F("Date color (RRGGBB)")], tempColor, F("FFFFFF")) && colorFromHexString(m_configDateColor.raw, tempColor.c_str());

    configComplete &= getJsonValue(top[F("Background fade")], m_configBackgroundFade);

    return configComplete;
}

/*
* Definition of the main usermode class wordcloxel
*/
static WordCloxel usermod_wordcloxel;
REGISTER_USERMOD(usermod_wordcloxel);


// TODO:
//  - Show cloxel at startup for X seconds
//  - React to unconnected / connected => show no-wifi
//     - Or use RTC time
//  - Make sure the timings work on a clean  ESP32
//  - Setup correct LED matrix layout on a clean ESP32

// Debugging using:
//DEBUG_PRINTF_P(PSTR(" value %df\n"), color);
