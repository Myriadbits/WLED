#pragma once
#include "wled.h"
#include "cloxel_layout_base.h"
#include "Time_word_convertor.h"
#include "BLEConfig/BLEConfig.h"

typedef enum class EDisplayMode : uint8_t
{
    None = 0,
    Initializing, 
    NotTime,
    Normal
} EDisplayMode;

typedef enum class EMessageMode : uint8_t
{
    None = 0,
    GoodMorning,
    Breakfast, 
    Lunch,
    Dinner,
    WordCloxel
} EMessageMode;

typedef enum class EEffectMode : uint8_t
{
    None = 0,
    Foreground,
    Background,
    Double,
} EEffectMode;

typedef struct SConfigItems
{
    uint8_t layout;
    uint8_t timezone;
    uint8_t effectMode;
    uint8_t backgroundEffect;
    uint8_t backgroundPalette;
    uint8_t backgroundBrightness;
    uint8_t foregroundEffect;
    uint8_t foregroundPalette;
    uint8_t foregroundBrightness;
    uint8_t introPalette;
    uint8_t lastCommand;
    uint8_t dummy1; // To align to 32 bit
    uint32_t time;
    uint8_t ipAddress1;
    uint8_t ipAddress2;
    uint8_t ipAddress3;
    uint8_t ipAddress4;
} SConfigItems;


// TODO All configuration items
#define SID_WIFI                     1
#define SID_COMMAND                 42
#define SID_MULTISETTING            50

#define MULTiSETTINGS_VERSION        1

// #define CONFIG_LOCATION                 2
// #define CONFIG_LAYOUT                   3
// #define CONFIG_TIMEZONE                 5
// #define CONFIG_DAYLIGHTSAVING           6

// #define CONFIG_COLOR_TIME               10
// #define CONFIG_COLOR_WEEKDAY            11
// #define CONFIG_COLOR_DATE               12
// #define CONFIG_COLOR_BACKGROUND         13
// #define CONFIG_BRIGHTNESS_DAY           14
// #define CONFIG_BRIGHTNESS_NIGHT         15
// #define CONFIG_BRIGHTNESS_BACKGROUND    16

// #define CONFIG_OPTIONS_TIME             20
// #define CONFIG_OPTIONS_WEEKDAY          21
// #define CONFIG_OPTIONS_DATE             22
// #define CONFIG_OPTIONS_CLOXEL           23

// #define CONFIG_NAME                     30

#define WORDCLOCK_MANUFACTURER          "www.cloxel.nl"
#define WORDCLOCK_MODEL                 "Wordcloxel"
#define WORDCLOCK_VERSION               "2.1.0"
#define WORDCLOCK_DEFAULTNAME           "Wordcloxel"
#define WORDCLOCK_DEFAULTLOCATION       "Home"


/*
 * Word cloxel usermod to display the correct time & date in words on a cloxel matrix
 */
class WordCloxel : public Usermod, public IBLEConfigCallbacks
{
  private:
    unsigned long m_lastUpdateTime {0};
    int m_displayCounter {0};
    uint8_t m_introY {5};
    uint8_t m_introX {0};

    bool m_fInitialized {false};
    EDisplayMode m_displayMode {EDisplayMode::Initializing};
    EMessageMode m_messageMode {EMessageMode::None};
    unsigned long m_messageEndTime {0};

    static const char _txtName[] PROGMEM;
    static const char _txtNameLower[] PROGMEM;
    static const char _txtMsg[] PROGMEM;
    static const char _txtTime[] PROGMEM;
    char m_txtBackground[16] {0};
    char m_txtForeground[16] {0};

    // BLE Config items
    BLEConfig  m_bleconfig {WORDCLOCK_MODEL, WORDCLOCK_MANUFACTURER, WORDCLOCK_VERSION, 256}; // 256 = Clock TODO VERSION
    BLEConfigItemWiFi m_bleWiFi {SID_WIFI};
    BLEConfigItemMultiSetting m_bleMultiSetting {SID_MULTISETTING, MULTiSETTINGS_VERSION };   

    // Config variables
    bool m_configEnabled {true};

    std::vector<const ledpos_t*> m_vecWordsTime;
    std::vector<const ledpos_t*> m_vecWordsDate;
    std::vector<const ledpos_t*> m_vecWordsWeekday;
    std::vector<const ledpos_t*> m_vecWordsSecond;
    std::vector<const ledpos_t*> m_vecWordsExtra;
    const ledclocklayout_t *m_pCloxelLayout { nullptr };

    bool m_isBTConnected { false };
    int m_heapCounter { 0 };

    EEffectMode m_lastEffectMode {EEffectMode::None};
    SConfigItems m_configuration {
        .layout = 0,
        .timezone = WLED_TIMEZONE,
        .effectMode = (uint8_t) EEffectMode::Double,
        .backgroundEffect = FX_MODE_TWINKLEFOX,
        .backgroundPalette = 50, // Aurora
        .backgroundBrightness = 128,
        .foregroundEffect = FX_MODE_BREATH,
        .foregroundPalette = 1, // Random
        .foregroundBrightness = 128,
        .introPalette = 37, // Cyane (also nice 26 or 34)

        .lastCommand = 0,
        .time = 0,
        .ipAddress1 = 0,
        .ipAddress2 = 0,
        .ipAddress3 = 0,
        .ipAddress4 = 0,
     };

  public:
    void setup() override;
    void setupEffects();
    void connected() override;  
    void loop() override;
    void handleOverlayDraw() override;

    void addToJsonInfo(JsonObject& root) override;
    void readFromJsonState(JsonObject& root) override;

    void addToConfig(JsonObject& root) override;
    bool readFromConfig(JsonObject& root) override;
    void appendConfigData() override;

    uint16_t getId() override { return 255; }

     // MIOT Callbacks
    virtual void onBluetoothConnection(bool success);
    virtual void onConfigItemChanged(BLEConfigItemBase *pconfigItem);

private:
    void setLayout();
    void showCloxelIntro();
    void modifyBackground();
    void addWordToLeds(uint8_t segment, const ledpos_t* pWord, CRGB color, int idx, bool useForegroundEffect = true);
    void addWordsToLeds(uint8_t segment, std::vector<const ledpos_t*> rVecWords, CRGB defaultColor, int idx, bool useForegroundEffect = true);
    void refreshConfiguration();
};

