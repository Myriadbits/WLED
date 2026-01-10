#pragma once
#include "wled.h"
#include "CloxelLayoutBase.h"
#include "ClockTimeWordConvertor.h"

/*
 * Word cloxel usermod to display the correct time & date in words on a cloxel matrix
 */
class WordCloxel : public Usermod 
{
  private:
    unsigned long lastTime = 0;
    static const char _txtName[];
    static const char _txtNameLower[];
    static const char _txtBrightness[];

    // Config variables
    bool m_configEnabled = false;
    // uint8_t m_configStartHour = 23;
    // uint8_t m_configStartMinute = 0;
    // uint8_t m_cconfigEndHour = 7;
    // uint8_t configEndMinute = 0;
    // uint configBrightness = 2; // Default is really low

    ClockWords_t            m_sClockWords;
    const ledclocklayout_t *m_pCloxelLayout = nullptr;
    
  public:
    void setup() override;

    /*
     * loop() is called continuously. In this method we check if we are inside the configured period or not.
     * If inside the brightness will be set to the configured value. If not, the default brightness is used.
     */
    void loop() override;
    void handleOverlayDraw() override;

    void addToJsonInfo(JsonObject& root) override;
    void addToConfig(JsonObject& root) override;
    bool readFromConfig(JsonObject& root) override;
    void appendConfigData() override;

    uint16_t getId() override { return 255; }

private:
    void AddWordToLeds(const ledpos_t *pCurrentWord, CRGB defaultColor);
};

