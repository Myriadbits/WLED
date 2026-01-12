#pragma once

#include <vector>
#include <string>
#include "cloxel_layout_base.h"

///////////////////////////////////////////////////////////////////////////////
// ClockTimeWordConvertor class
// Helper class to convert time into words for different clockstypes
///////////////////////////////////////////////////////////////////////////////
class ClockTimeWordConvertor
{
    public:
        static void convertHoursAndMinutes(const ledclocklayout_t* pLayout, std::vector<const ledpos_t*> &rVecOutput);
        static void convertSeconds(const ledclocklayout_t* pLayout, std::vector<const ledpos_t*> &rVecOutput);
        static void convertDate(const ledclocklayout_t* pLayout, std::vector<const ledpos_t*> &rVecOutput);
        static void convertWeekDay(const ledclocklayout_t* pLayout, std::vector<const ledpos_t*> &rVecOutput);
};