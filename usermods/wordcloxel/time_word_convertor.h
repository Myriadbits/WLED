#pragma once

#include <vector>
#include <string>
#include "cloxel_layout_base.h"

#ifndef NULL
#define NULL    (void *)0
#endif

typedef struct
{
    const ledpos_t* pToPastWord;
    const ledpos_t* pMinutesMainWord;
    const ledpos_t* pMinutesRestWord;
    const ledpos_t* pHalfWord;
    const ledpos_t* pHourWord;

    const ledpos_t* pDayWord;
    const ledpos_t* pDayOfMonthWord;
    const ledpos_t* pMonthWord;
    const ledpos_t* pSecondLeds;
} ClockWords_t;

///////////////////////////////////////////////////////////////////////////////
// ClockTimeWordConvertor class
// Helper class to convert time into words for different clockstypes
///////////////////////////////////////////////////////////////////////////////
class ClockTimeWordConvertor
{
    public:
        static void convert(const ledclocklayout_t* pLayout, ClockWords_t* pOutput);
};