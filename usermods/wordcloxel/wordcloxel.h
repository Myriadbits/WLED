#pragma once
#include "wled.h"
#include "cloxel_layout_base.h"
#include "Time_word_convertor.h"

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

/*
 * Word cloxel usermod to display the correct time & date in words on a cloxel matrix
 */
class WordCloxel : public Usermod 
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

private:
    void AddWordsToLeds(std::vector<const ledpos_t*> rVecWords, CRGB defaultColor);
};

