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
    uint8_t foregroundEffect;
    uint8_t foregroundPalette;
    uint8_t backgroundEffect;
    uint8_t backgroundPalette;
    uint8_t brightness;
    uint8_t backgroundBrightness;
    uint8_t lastCommand;
    uint32_t time;
    uint8_t ipAddress1;
    uint8_t ipAddress2;
    uint8_t ipAddress3;
    uint8_t ipAddress4;
    uint8_t introPalette;
} SConfigItems;

// TODO All configuration items
#define SID_WIFI                    1
#define SID_MULTISETTING            2

// Multi config settings
#define MC_LAYOUT                   0
#define MC_IMEZONE                  1

#define MC_FG_MODE                  2
#define MC_FG_EFFECT                3
#define MC_FG_PALETTE               4
#define MC_BK_EFFECT                5
#define MC_BK_PALETTE               6

#define MC_BRIGHTNESS               7
#define MC_BK_BRIGHTNESS            8

#define MC_COMMAND                  9

#define MC_TIME_1                   10 
#define MC_TIME_2                   11
#define MC_TIME_3                   12
#define MC_TIME_4                   13

#define MC_IPADDRESS_1              14 
#define MC_IPADDRESS_2              15
#define MC_IPADDRESS_3              16
#define MC_IPADDRESS_4              17

#define MC_INTRO_PALETTE            50



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
    BLEConfigItemMultiSetting m_bleMultiSetting {SID_MULTISETTING };   

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
        .timezone = 1,
        .effectMode = (uint8_t) EEffectMode::Double,
        .foregroundEffect = FX_MODE_TWINKLEFOX,
        .foregroundPalette = 50, // Aurora
        .backgroundEffect = FX_MODE_BREATH,
        .backgroundPalette = 1, // Random
        .brightness = 128,
        .backgroundBrightness = 128,
        .lastCommand = 0,
        .time = 0,
        .ipAddress1 = 0,
        .ipAddress2 = 0,
        .ipAddress3 = 0,
        .ipAddress4 = 0,

        .introPalette = 37 // Cyane (also nice 26 or 34)
     };

  public:
    void setup() override;
    void setupEffects(EEffectMode newEffect);
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
};

