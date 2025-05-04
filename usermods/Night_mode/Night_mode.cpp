#include "wled.h"

/*
 * Usermod to dim the LEDs during a specific period (for instance at night)
 */

class NightMode : public Usermod 
{
  private:
    unsigned long lastTime = 0;
    static const char _txtName[];
    static const char _txtNameLower[];
    static const char _txtBrightness[];

    // Config variables
    bool configEnabled = false;
    uint8_t configStartHour = 23;
    uint8_t configStartMinute = 0;
    uint8_t configEndHour = 7;
    uint8_t configEndMinute = 0;
    uint configBrightness = 2; // Default is really low
    
  public:
    void setup()
    {
      // Do nothing
    }

    /*
     * loop() is called continuously. In this method we check if we are inside the configured period or not.
     * If inside the brightness will be set to the configured value. If not, the default brightness is used.
     */
    void loop() {
      if (millis() - lastTime > 200) { // Check a few times per second...
        
        bool inPeriod = false;
        if (configEnabled) {
          int minutes = minute(localTime);
          int hours = hour(localTime);

          int currentTimeMin = hours * 60 + minutes;
          int startTimeMin = configStartHour * 60 + configStartMinute;
          int endTimeMin = configEndHour * 60 + configEndMinute;

          if (startTimeMin < endTimeMin) {
            // Start time before end time
            inPeriod = (currentTimeMin >= startTimeMin && currentTimeMin < endTimeMin);
          } else {
            // Start time after end time (day wrap)
            inPeriod = (currentTimeMin >= startTimeMin || currentTimeMin < endTimeMin);
          }
        }

        if (inPeriod) {
          strip.setBrightness(scaledBri(configBrightness));
        }
        else {
          strip.setBrightness(bri);
        }
        
        // Remember last update
        lastTime = millis();
      }
    }

    /*
     * Add our own config items
     */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject(F(_txtName));

      top[F("Active")] = configEnabled;
      top[F("Start hour")] = configStartHour;
      top[F("Start minute")] = configStartMinute;
      top[F("End hour")] = configEndHour;
      top[F("End minute")] = configEndMinute;
      top[F("Brightness")] = configBrightness;
    }

    /*
     * Add some extra information to the config items
     */
    void appendConfigData()
    {
      oappend(F("addInfo('")); oappend(_txtName); oappend(F(":Start hour', 1, '(0-23)');"));
      oappend(F("addInfo('")); oappend(_txtName); oappend(F(":Start minute', 1, '(0-59)');"));
      oappend(F("addInfo('")); oappend(_txtName); oappend(F(":End hour', 1, '(0-23)');"));
      oappend(F("addInfo('")); oappend(_txtName); oappend(F(":End minute', 1, '(0-59)');"));
      oappend(F("addInfo('")); oappend(_txtName); oappend(F(":Brightness', 1, '(%)');"));
    }

    /*
     * Read the values of the config items
     */
    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root[F(_txtName)];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top[F("Active")], configEnabled);
      configComplete &= getJsonValue(top[F("Start hour")], configStartHour);
      configComplete &= getJsonValue(top[F("Start minute")], configStartMinute);
      configComplete &= getJsonValue(top[F("End hour")], configEndHour);
      configComplete &= getJsonValue(top[F("End minute")], configEndMinute);
      configComplete &= getJsonValue(top[F("Brightness")], configBrightness);

      return true;//configComplete;
    }

    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return 255;
    }
};

static NightMode usermod_night_mode;
REGISTER_USERMOD(usermod_night_mode);

// strings to reduce flash memory usage (used more than twice)
const char NightMode::_txtName[]  PROGMEM = "Night mode";
const char NightMode::_txtNameLower[]  PROGMEM = "night_mode";
