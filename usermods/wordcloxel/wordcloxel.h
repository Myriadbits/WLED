#pragma once
#include "wled.h"
#include "cloxel_layout_base.h"
#include "Time_word_convertor.h"

typedef enum 
{
    NONE = 0,
    INITIALIZING, 
    NOWIFI,
    NORMAL
} EDisplayModes;

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
    EDisplayModes m_displayMode {EDisplayModes::INITIALIZING};
    static const char _txtName[];
    static const char _txtNameLower[];
    static const char _txtMsg[];
    static const char _txtTime[];

    static const char _txtBrightness[];

    // Config variables
    bool m_configEnabled {false};
    uint8_t m_configLayout = 0;
    uint32_t m_configTimeColor = 0x00FF00;
    uint32_t m_configWeekdayColor = 0xFFA500;
    uint32_t m_configDateColor = 0xE59400;
    uint m_configBackgroundFade {70};

    std::vector<const ledpos_t*> m_vecWordsTime;
    std::vector<const ledpos_t*> m_vecWordsDate;
    std::vector<const ledpos_t*> m_vecWordsWeekday;
    std::vector<const ledpos_t*> m_vecWordsSecond;
    const ledclocklayout_t *m_pCloxelLayout = nullptr;

    String m_displayMsg;
    int m_displayTime {0};
    
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

