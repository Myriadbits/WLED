#pragma once
#include "wled.h"
#include "cloxel_layout_base.h"
#include "Time_word_convertor.h"
#include "BLEConfig/BLEConfig.h"

typedef enum EDisplayMode
{
    DM_NONE = 0,
    DM_INITIALIZING, 
    DM_NOTIME,
    DM_NORMAL
} EDisplayMode;

typedef enum EMessageMode
{
    EM_NONE = 0,
    EM_GOODMORNING,
    EM_BREAKFAST, 
    EM_LUNCH,
    EM_DINNER,
    EM_WORDCLOXEL
} EMessageMode;

// TODO All configuration items
#define SID_WIFI                    1
#define SID_MULTISETTING            2

// Multi config settings
#define MC_LAYOUT                   0
#define MC_IMEZONE                  1

#define MC_FG_EFFECT                2
#define MC_FG_PALETTE               3
#define MC_BK_EFFECT                4
#define MC_BK_PALETTE               5

#define MC_BRIGHTNESS               6
#define MC_BK_BRIGHTNESS            7

#define MC_COMMAND                  8

#define MC_TIME_1                   9 
#define MC_TIME_2                   10
#define MC_TIME_3                   11
#define MC_TIME_4                   12

#define MC_IPADDRESS_1              13 
#define MC_IPADDRESS_2              14
#define MC_IPADDRESS_3              15
#define MC_IPADDRESS_4              16


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
    unsigned long m_startOfInitializedTime {0};
    int m_displayCounter {0};
    uint8_t m_introY {5};
    uint8_t m_introX {0};

    bool m_fInitialized {false};
    EDisplayMode m_displayMode {EDisplayMode::DM_INITIALIZING};
    EMessageMode m_messageMode {EMessageMode::EM_NONE};
    unsigned long m_messageEndTime {0};
    int m_messageTime {0};

    static const char _txtName[] PROGMEM;
    static const char _txtNameLower[] PROGMEM;
    static const char _txtMsg[] PROGMEM;
    static const char _txtTime[] PROGMEM;

    static const char _txtBrightness[];

    // BLE Config items
    BLEConfig  m_bleconfig {WORDCLOCK_MODEL, WORDCLOCK_MANUFACTURER, WORDCLOCK_VERSION, 256}; // 256 = Clock TODO VERSION
    BLEConfigItemWiFi m_bleWiFi {SID_WIFI};
    BLEConfigItemMultiSetting m_bleMultiSetting {SID_MULTISETTING };

    //BLEConfigItemUInt32 m_bleSetTime {CONFIG_TIME, CT_UINT32, "Set the time (unixtime)"};
    //BLEConfigItemUInt8 m_bleTimezone {CONFIG_TIMEZONE, CT_UINT8, "Time zone", 24};
    //BLEConfigItemUInt8 m_bleEffect {CONFIG_EFFECT, CT_UINT8, "Effect", MODE_COUNT};
    //BLEConfigItemUInt8 m_blePalette {CONFIG_PALETTE, CT_UINT8, "Palette", 58};
    

    // Config variables
    bool m_configEnabled {true};
    uint8_t m_configLayout {0};
    CRGB m_configTimeColor {0x00FF00};
    CRGB m_configWeekdayColor {0xFFA500};
    CRGB m_configDateColor {0xFF5500};
    uint m_configBackgroundFade {40};

    std::vector<const ledpos_t*> m_vecWordsTime;
    std::vector<const ledpos_t*> m_vecWordsDate;
    std::vector<const ledpos_t*> m_vecWordsWeekday;
    std::vector<const ledpos_t*> m_vecWordsSecond;
    std::vector<const ledpos_t*> m_vecWordsExtra;
    const ledclocklayout_t *m_pCloxelLayout { nullptr };

    bool m_isBTConnected { false };
    int m_heapCounter { 0 };

  public:
    void setup() override;
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
    void addWordToLeds(uint8_t segment, const ledpos_t* pWord, CRGB color, int idx, bool useForegroundEffect = true);
    void addWordsToLeds(uint8_t segment, std::vector<const ledpos_t*> rVecWords, CRGB defaultColor, int idx, bool useForegroundEffect = true);
};

