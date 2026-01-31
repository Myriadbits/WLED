#pragma once
#include "wled.h"
#include "cloxel_layout_base.h"
#include "Time_word_convertor.h"
#include "BLEConfig/BLEConfig.h"

typedef enum EDisplayMode
{
    DM_NONE = 0,
    DM_INITIALIZING, 
    DM_NOWIFI,
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
#define CONFIG_WIFI                     1
#define CONFIG_LOCATION                 2
#define CONFIG_LAYOUT                   3
#define CONFIG_TIMEZONE                 5
#define CONFIG_DAYLIGHTSAVING           6

#define CONFIG_COLOR_TIME               10
#define CONFIG_COLOR_WEEKDAY            11
#define CONFIG_COLOR_DATE               12
#define CONFIG_COLOR_BACKGROUND         13
#define CONFIG_BRIGHTNESS_DAY           14
#define CONFIG_BRIGHTNESS_NIGHT         15
#define CONFIG_BRIGHTNESS_BACKGROUND    16

#define CONFIG_OPTIONS_TIME             20
#define CONFIG_OPTIONS_WEEKDAY          21
#define CONFIG_OPTIONS_DATE             22
#define CONFIG_OPTIONS_CLOXEL           23

#define CONFIG_NAME                     30

#define CONFIG_COMMAND                  42

/*
 * Word cloxel usermod to display the correct time & date in words on a cloxel matrix
 */
class WordCloxel : public Usermod, public IBLEConfigCallbacks
{
  private:
    unsigned long m_lastUpdateTime {0};
    unsigned long m_startOfInitializedTime {0};
    int m_displayCounter {0};
    bool m_fInitialized {false};
    EDisplayMode m_displayMode {EDisplayMode::DM_INITIALIZING};
    EMessageMode m_messageMode {EMessageMode::EM_NONE};
    unsigned long m_messageEndTime {0};
    int m_messageTime {0};

    static const char _txtName[];
    static const char _txtNameLower[];
    static const char _txtMsg[];
    static const char _txtTime[];

    static const char _txtBrightness[];

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
    const ledclocklayout_t *m_pCloxelLayout {nullptr};

    int m_heapCounter {0};

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
    virtual void onDisplayPassKey(uint32_t passkey);
    virtual void onBluetoothConnection(bool success);
    virtual void onConfigItemChanged(BLEConfigItemBase *pconfigItem);

private:
    void AddWordsToLeds(std::vector<const ledpos_t*> rVecWords, CRGB defaultColor);
};

