#include "wled.h"
#include "wordcloxel.h"
#include "time_word_convertor.h"
#include "cloxel_layout_en_v1.h"
#include "cloxel_layout_nl_v1.h"

// #define WORDCLOCK_MANUFACTURER    "www.cloxel.nl"
// #define WORDCLOCK_MODEL           "Wordcloxel"
// #define WORDCLOCK_VERSION         "2.1.0"
// #define WORDCLOCK_DEFAULTNAME     "Wordcloxel"
// #define WORDCLOCK_DEFAULTLOCATION "Home"

// strings to reduce flash memory usage (used more than twice)
const char WordCloxel::_txtName[]  PROGMEM = "Wordcloxel";
const char WordCloxel::_txtNameLower[]  PROGMEM = "wordcloxel";
const char WordCloxel::_txtMsg[]  PROGMEM = "msg";
const char WordCloxel::_txtTime[]  PROGMEM = "time";
constexpr int MINIMUM_INITIALIZE_TIME = 5000;
constexpr int CLOXEL_STARTUP_WAIT_TICKS = 500;
constexpr int CLOXEL_STARTUP_CYCLE_TICKS = 200;
constexpr int CLOXEL_STARTUP_CYCLES = 5;
constexpr int CLOXEL_STARTUP_TOTAL_TICKS = CLOXEL_STARTUP_WAIT_TICKS + (CLOXEL_STARTUP_CYCLE_TICKS * CLOXEL_STARTUP_CYCLES) + 100;
constexpr uint32_t CLOXEL_STARTUP_COLOR = RGBW32(0xFF, 0x7E, 0, 0);
constexpr float_t NUMBER_OF_SECOND_PULSES_PER_MINUTE = 20.0f;
constexpr uint32_t NUMBER_OF_MILLIS_PER_PULSE = 3000;
constexpr uint32_t RESTART_TIME_NOWIFI = 300000; // ms

static CRGB g_colorLoop[] {CRGB::Olive, CRGB::Navy, CRGB::LightBlue, CRGB::Aqua, CRGB::Teal, CRGB::Green, CRGB::Silver, CRGB::Yellow, 
                           CRGB::Orange, CRGB::Red, CRGB::Maroon, CRGB::Fuchsia, CRGB::Purple, CRGB::Magenta, CRGB::White, CRGB::Lime} PROGMEM;

constexpr char MY_RIPPLE_DATA[] PROGMEM = "Ripple@!,Wave #,Blur,,,,Overlay;,!;!;1;c1=0";

//constexpr float_t SECOND_PULSE_DURATION = 60000.0 / NUMBER_OF_SECOND_PULSES_PER_MINUTE;

// The BLE Config library
// Without BLE: 98620
// 94452

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
    m_displayMode = EDisplayMode::Initializing;

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

    strip.setBrightness(128); // Was 128

    // This is essential! Or else the whole program will crash due to the set_sleep issue!
    noWifiSleep = false;

    // Setup RTC
    i2c_sda = 21;
    i2c_scl = 22;

    // 25560 => 61360
    // Load/initialize all BLE Config settings
    m_bleconfig.addConfigItem(&m_bleWiFi);
    m_bleconfig.addConfigItem(&m_bleMultiSetting); // 591 bytes on heap per piece!

    // Start the BLE Config stuff
    // This will also load all previously stored settings
    m_bleconfig.start(this); // TODO MAYBE THIS SHOULD BE DONE IN THE SETUP?

    sleep(0.5);

    m_configuration.effectMode = (uint8_t) EEffectMode::Double;

    // Setup all the effects
    setupEffects();

    refreshConfiguration();
}


void WordCloxel::setupEffects()
{
    if (strip.getSegmentsNum() != 1)
        strip.resetSegments();

    // Add exta segment for foreground
    strip.appendSegment(0, 16, 0, 16); 

    Segment& seg0 = strip.getSegment(0);
    seg0.mode = m_configuration.backgroundEffect;
    seg0.palette = m_configuration.backgroundPalette;
    if (m_configuration.effectMode == (uint8_t) EEffectMode::Background)
        seg0.setOpacity(m_configuration.backgroundBrightness);
    else if (m_configuration.effectMode == (uint8_t) EEffectMode::None)
        seg0.setOpacity(0);
    else
        seg0.setOpacity(10);

    Segment& seg1 = strip.getSegment(1);
    seg1.mode = m_configuration.foregroundEffect;
    seg1.palette = m_configuration.foregroundPalette; 
    seg1.setOpacity(1);

    seg0.stop = 15;
    seg0.stop = 16;
    seg0.refreshLightCapabilities();
        
    // Yes, the following call makes sure the palette is used for segment 1
    seg1.refreshLightCapabilities();    
}

/* 
* Called when WiFi is (re)connected
*/
void WordCloxel::connected()
{
}

/*
* Set the layout to use based on the given layout id
*/
void WordCloxel::setLayout()
{
    switch (m_configuration.layout)
    {
        case 0: m_pCloxelLayout = &s_layoutEN_V1; break;
        case 1: m_pCloxelLayout = &s_layoutNL_V1; break;
        default: m_pCloxelLayout = &s_layoutEN_V1; break;
    }
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

            if (m_heapCounter++ >= 100)
            {
                m_heapCounter = 0;
                BLECONFIG_LOG("Heap now: %ld", getFreeHeapSize());
            }

            // Clear previous words
            m_vecWordsTime.clear();
            m_vecWordsDate.clear();
            m_vecWordsWeekday.clear();
            m_vecWordsSecond.clear();
            m_vecWordsExtra.clear();

            // Update the configuration with some static info
            m_configuration.ipAddress1 = Network.localIP()[0];
            m_configuration.ipAddress2 = Network.localIP()[1];
            m_configuration.ipAddress3 = Network.localIP()[2];
            m_configuration.ipAddress4 = Network.localIP()[3];
            
            switch (m_displayMode)
            {
                case EDisplayMode::Initializing:         
                    if (m_displayCounter > CLOXEL_STARTUP_TOTAL_TICKS)
                    {    
                        if (year(localTime) > 2025)
                        {
                            m_displayCounter = 0;
                            m_displayMode = EDisplayMode::Normal;
                        }
                        else
                        {
                            m_displayMode = EDisplayMode::NotTime;
                        }
                    }
                    break;

                case EDisplayMode::NotTime:
                    m_vecWordsTime.push_back(m_pCloxelLayout->extra.no);
                    m_vecWordsTime.push_back(m_pCloxelLayout->extra.time);
                    if (year(localTime) > 2025)
                    {
                        m_displayMode = EDisplayMode::Normal;
                    }   
                    else if (currentTime > RESTART_TIME_NOWIFI)
                    {
                        esp_restart();
                    }
                    break;

                case EDisplayMode::Normal:
                    strip.setBrightness(m_configuration.foregroundBrightness, true);

                    // Determine the words to display
                    ClockTimeWordConvertor::convertHoursAndMinutes(m_pCloxelLayout, m_vecWordsTime);
                    ClockTimeWordConvertor::convertDate(m_pCloxelLayout, m_vecWordsDate);
                    ClockTimeWordConvertor::convertWeekDay(m_pCloxelLayout, m_vecWordsWeekday);
                    ClockTimeWordConvertor::convertSeconds(m_pCloxelLayout, m_vecWordsSecond);

                    if (currentTime < m_messageEndTime)
                    {
                        switch (m_messageMode)
                        {
                            case EMessageMode::GoodMorning:
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.goodmorning);
                                break;
                            case EMessageMode::Breakfast:
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.breakfast);
                                break;
                            case EMessageMode::Lunch:
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.lunch);
                                break;
                            case EMessageMode::Dinner:
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.dinner);
                                break;
                            case EMessageMode::WordCloxel:
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.myriadbits);
                                m_vecWordsExtra.push_back(m_pCloxelLayout->extra.word);
                                break;
                            default:
                                break;
                        }
                    }
                    else
                    {
                        m_messageMode = EMessageMode::None;
                        m_messageEndTime = 0;
                    }

                    if (m_isBTConnected)
                        m_vecWordsExtra.push_back(m_pCloxelLayout->extra.bluetooth);

                    break;

                default:
                    break;
            }            
        }

        // When no effect is present, we should update the display (or handleOverlayDraw is not called often enough)
        strip.show();
    }
}

//
// Add a single word to the display/leds
// customParam can be any value, normal operation when ColorHandler is not overriden: customParam is the color
void WordCloxel::addWordToLeds(uint8_t segment, const ledpos_t* pWord, CRGB color, int idx, bool useForegroundEffect) 
{
    uint8_t charIndex = 0;
    ledpos_t ledPos = pWord[charIndex];
    Segment& seg = strip.getSegment(segment);
    uint32_t col = (uint32_t) color;
    while (ledPos.x >= 0 && ledPos.y >= 0)
    {
        if (useForegroundEffect)
        {
            col = seg.getPixelColorXY(ledPos.x, ledPos.y);  
        }
        strip.setPixelColorXY(ledPos.x, ledPos.y, col);
        
        // Next char
        charIndex++;
        ledPos = pWord[charIndex];
    }
}

//
// Add a single word to the display/leds
// customParam can be any value, normal operation when ColorHandler is not overriden: customParam is the color
void WordCloxel::addWordsToLeds(uint8_t segment, std::vector<const ledpos_t*> rVecWords, CRGB color, int idx, bool useForegroundEffect) 
{
    for(const ledpos_t* pCurrentWord : rVecWords)
    {
        addWordToLeds(segment, pCurrentWord, color, idx, useForegroundEffect);
    }
}

/// @brief Show the cloxel intro animation, which comes in from the left
void WordCloxel::showCloxelIntro()
{
    strip.fill(BLACK);

    float value = 255;
    m_displayCounter++;
    if (m_displayCounter < CLOXEL_STARTUP_WAIT_TICKS)
        return; // Wait a while to initialize stuff
  
    int pal = m_configuration.introPalette;
    CRGBPalette16 palette = CRGBPalette16();
    byte tcp[72];
    memcpy_P(tcp, (byte*)pgm_read_dword(&(gGradientPalettes[pal - (DYNAMIC_PALETTE_COUNT + FASTLED_PALETTE_COUNT)])), sizeof(tcp));
    CRGBPalette16 targetPalette;
    targetPalette.loadDynamicGradientPalette(tcp);
    
    if (m_introY < 15)
    {
        m_introX++;
        if (m_introX > 15)
        {
            m_introX = 0;
            m_introY++;
        }             
        strip.setPixelColorXY(m_introX, m_introY, (uint32_t)targetPalette[(m_introY - 5) % 16]);
    }

    if (m_displayCounter > CLOXEL_STARTUP_TOTAL_TICKS - 300)
    {
        m_introX++;
        if (m_introX > 8)
        {
            m_introX = 0;
            m_introY++;
        }
        int startY = m_introY - 10;
        for(uint8_t y = startY; y < 16; y++)
        {
            strip.setPixelColorXY(15, y, (uint32_t)targetPalette[(y - startY) % 16]);
        }
    }
    else
    {
        for(uint8_t y = 5; y < m_introY; y++)
        {
            strip.setPixelColorXY(15, y, (uint32_t)targetPalette[(y - 5) % 16]);
        }
    }
}

void WordCloxel::modifyBackground()
{
    Segment& seg0 = strip.getSegment(0);
    for(int i = 0; i < seg0.width() * seg0.height(); i++) 
    {
        uint32_t c1 = strip.getPixelColor(i);
        strip.setPixelColor(i, color_fade(c1, m_configuration.backgroundBrightness)); // blank out the segment
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
        m_displayCounter++;

        // At startup, always show cloxel text
        if (m_displayMode == EDisplayMode::Initializing)
        {
            showCloxelIntro();
        }
        else if (m_displayMode == EDisplayMode::NotTime)
        {
            strip.fill(BLACK);
            float value = 127 * (cos_approx(m_displayCounter * M_TWOPI / (float_t)CLOXEL_STARTUP_CYCLE_TICKS) + 1.0f);
            CRGB color = RGBW32((int) value, 0, 0, 0);
            addWordsToLeds(1, m_vecWordsTime, color, 250);
        }
        else
        {
            // Segment 0, plays background effect
            // Segment 1, plays text effect
            int segment = 1;
            EEffectMode currentEffectMode = (EEffectMode)m_configuration.effectMode;
            bool useEffectFromSeg = (currentEffectMode == EEffectMode::Double || currentEffectMode == EEffectMode::Foreground);
            
            CRGB colTime = ColorFromPaletteWLED(SEGPALETTE, 1);
            CRGB colWeekday = ColorFromPaletteWLED(SEGPALETTE, 40);
            CRGB colDate = ColorFromPaletteWLED(SEGPALETTE, 100);

            if (currentEffectMode == EEffectMode::Background)
            {
                modifyBackground();
            }
            else if (currentEffectMode == EEffectMode::Foreground || currentEffectMode == EEffectMode::None)
            {
                strip.fill(BLACK);
            }
            else
            {
                Segment& seg0 = strip.getSegment(0);
                for(int i = 0; i < seg0.width() * seg0.height(); i++) 
                {
                    uint32_t c1 = seg0.getPixelColor(i);
                    strip.setPixelColor(i, color_fade(c1, m_configuration.backgroundBrightness)); // blank out the segment
                }
            }

            // Add all words to the clock
            addWordsToLeds(segment, m_vecWordsTime, colTime, 1, useEffectFromSeg);
            addWordsToLeds(segment, m_vecWordsWeekday, colWeekday, 50, useEffectFromSeg);
            addWordsToLeds(segment, m_vecWordsDate, colDate, 100, useEffectFromSeg);

            // Second pulse
            unsigned long milliOnly = millis() % NUMBER_OF_MILLIS_PER_PULSE; 
            float value = 127 * (1.0f + cos_approx(milliOnly * M_TWOPI / (float_t)NUMBER_OF_MILLIS_PER_PULSE));
            addWordsToLeds(segment, m_vecWordsSecond, color_fade((uint32_t) colTime, (uint8_t)value), 150, useEffectFromSeg);

            // BT & WiFi
            float value2 = 127 * (cos_approx(m_displayCounter * M_TWOPI / (float_t)CLOXEL_STARTUP_CYCLE_TICKS) + 1.0f);
            value2 /= 10; // Make the BT & No-WiFi very faint
            if (m_isBTConnected)
            {                
                addWordToLeds(0, m_pCloxelLayout->extra.bluetooth, color_fade((uint32_t) CRGB(0, 130, 252), (uint8_t)value2), 0, false); // Actual BT color
            }
            if (!WiFi.isConnected())
            {
                // Is this really such an issue that we need to show it on the clock? Maybe just show a small Wifi signal icon?
                CRGB color = color_fade((uint32_t) CRGB(255, 0, 0), (uint8_t)value2);
                addWordToLeds(0, m_pCloxelLayout->extra.no, color, 0, false);
                addWordToLeds(0, m_pCloxelLayout->extra.wifi, color, 0, false);
            }
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
                m_messageMode = EMessageMode::GoodMorning;
            }
            else if (displayMsg == "breakfast") 
            {
                m_messageMode = EMessageMode::Breakfast;
            }
            else if (displayMsg == "lunch") 
            {
                m_messageMode = EMessageMode::Lunch;
            }
            else if (displayMsg == "dinner") 
            {
                m_messageMode = EMessageMode::Dinner;
            }
            else if (displayMsg == "wordcloxel") 
            {
                m_messageMode = EMessageMode::WordCloxel;
            }
            DEBUG_PRINTF("Incoming message '%s'\n", displayMsg.c_str());
        }
    }
}

/*
* Add our own config items
*/
void WordCloxel::addToConfig(JsonObject& root)
{
    BLECONFIG_LOG("**** addToConfig");

    JsonObject top = root.createNestedObject(F(_txtName));

    top[F("Active")] = m_configEnabled;

    top[F("Layout")] = m_configuration.layout;
    top[F("Timezone")] = m_configuration.timezone;
    top[F("EffectMode")] = m_configuration.effectMode;
    top[F("ForegroundEffect")] = m_configuration.foregroundEffect;
    top[F("ForegroundPalette")] = m_configuration.foregroundPalette;
    top[F("ForegroundBrightness")] = m_configuration.foregroundBrightness;
    top[F("BackgroundEffect")] = m_configuration.backgroundEffect;
    top[F("BackgroundPalette")] = m_configuration.backgroundPalette;
    top[F("BackgroundBrightness")] = m_configuration.backgroundBrightness;
    top[F("IntroPalette")] = m_configuration.introPalette;
}

/*
* Add some extra information to the config items
*/
void WordCloxel::appendConfigData()
{
    oappend(F("dd=addDropdown('")); oappend(_txtName); oappend(F("','Layout');"));
    oappend(F("addOption(dd,'English V1',0);"));
    oappend(F("addOption(dd,'Dutch V2',1);"));

    oappend(F("dd=addDropdown('")); oappend(_txtName); oappend(F("','EffectMode');"));
    oappend(F("addOption(dd,'None',0);"));
    oappend(F("addOption(dd,'Foreground',1);"));
    oappend(F("addOption(dd,'Background',2);"));
    oappend(F("addOption(dd,'Both',3);"));

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
    BLECONFIG_LOG("**** readFromConfig");

    JsonObject top = root[F(_txtName)];

    bool configComplete = !top.isNull();

    m_configuration.version = MULTiSETTINGS_VERSION;
    configComplete &= getJsonValue(top[F("Active")], m_configEnabled);
    configComplete &= getJsonValue(top[F("Layout")], m_configuration.layout);
    configComplete &= getJsonValue(top[F("Timezone")], m_configuration.timezone);
    configComplete &= getJsonValue(top[F("EffectMode")], m_configuration.effectMode);
    configComplete &= getJsonValue(top[F("ForegroundEffect")], m_configuration.foregroundEffect);
    configComplete &= getJsonValue(top[F("ForegroundPalette")], m_configuration.foregroundPalette);
    configComplete &= getJsonValue(top[F("ForegroundBrightness")], m_configuration.foregroundBrightness);
    configComplete &= getJsonValue(top[F("BackgroundEffect")], m_configuration.backgroundEffect);
    configComplete &= getJsonValue(top[F("BackgroundPalette")], m_configuration.backgroundPalette);
    configComplete &= getJsonValue(top[F("BackgroundBrightness")], m_configuration.backgroundBrightness);
    configComplete &= getJsonValue(top[F("IntroPalette")], m_configuration.introPalette);

    refreshConfiguration();

    return configComplete;
}

/// @brief Refresh the screen/effects 
void WordCloxel::refreshConfiguration()
{    
    currentTimezone = m_configuration.timezone;

    // Refresh the screen with the new settings
    strip.suspend();
    setLayout();
    setupEffects();
    strip.resume();
}

//
// A BT connection has been established or failed
// 
void WordCloxel::onBluetoothConnection(bool connected)
{   
    m_isBTConnected = connected;

    // Copy data to BLE settings
    m_bleMultiSetting.setData((uint8_t*) &m_configuration, sizeof(m_configuration));
}

//
// A config item has changed through BLE, forward to the settings
// 
void WordCloxel::onConfigItemChanged(BLEConfigItemBase *pconfigItem)
{
    if (pconfigItem != NULL)
    {
        switch (pconfigItem->getId())
        {
            case SID_WIFI:
                {
                    // WiFi config has changed, we can react to it here if needed
                    BLECONFIG_LOG("Incoming SID WiFi");
                    BLEConfigItemWiFi* pconfig = (BLEConfigItemWiFi*) pconfigItem;
                    if (pconfig != NULL)
                    {
                        std::string ssid = pconfig->getSSID();
                        std::string passphrase = pconfig->getPassphrase();
                        BLECONFIG_LOG("Switching to network: '%s' with passphrase '%s'", ssid.c_str(), passphrase.c_str());

                        // Calling WLED stuff directly

                        // This is essential! Or else the whole program will crash due to the set_sleep issue!
                        noWifiSleep = false;

                        // Copy settings to the WiFi config
                        // See also 
                        memset(multiWiFi[0].clientSSID, 0, 32);
                        memcpy(multiWiFi[0].clientSSID, ssid.c_str(), ssid.length());

                        memset(multiWiFi[0].clientPass, 0, 64);
                        memcpy(multiWiFi[0].clientPass, passphrase.c_str(), passphrase.length());

                        forceReconnect = true;
                        serializeConfigToFS();

                        BLECONFIG_LOG("Wifi configured: %d", WLED_WIFI_CONFIGURED);

                        BLECONFIG_LOG("Wifi connected: %d", Network.isConnected());
                    }
                }
                break;

            case SID_MULTISETTING:
                {                    
                    BLECONFIG_LOG("Incoming SID MultiSetting");
                    BLEConfigItemMultiSetting* pconfig = (BLEConfigItemMultiSetting*) pconfigItem;
                    if (pconfig != NULL)
                    {
                        if (pconfig->getData()[0] == MULTiSETTINGS_VERSION)
                        {
                            // Do a plain copy to get the new settings
                            memcpy(&m_configuration, pconfig->getData(), sizeof(m_configuration));
                            refreshConfiguration();

                            serializeConfigToFS();
                        }
                    }                    
                }
                break;
        }
    }
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
