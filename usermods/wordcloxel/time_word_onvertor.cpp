///////////////////////////////////////////////////////////////////////////////
// ClockTimeWordConvertor class
// Class to convert the time to words
///////////////////////////////////////////////////////////////////////////////

#include "wled.h"
#include "time_word_convertor.h"

#define arraysize(x)          (int)(sizeof((x))/sizeof(*(x)))

using namespace std;

//
// @brief Convert the given time into words for the given clock layout
//
void ClockTimeWordConvertor::convertHoursAndMinutes(const ledclocklayout_t* pLayout, std::vector<const ledpos_t*> &rVecOutput)
{
    if (pLayout == nullptr) return;
    
    // check the time
    int nHours = hour(localTime);
    int nMinutes = minute(localTime);        

    // Quarter past 1 => 14 minutes to half two (in Dutch this is correct, English I don't know)
    int min5 = nMinutes / 5;
    int min1 = nMinutes % 5;        
    int hours = nHours;

    const ledtime_t* pTime = &(pLayout->time);

    bool addHours = true;
    switch (pLayout->timeFormat)
    {
    case ETimeFormat::TF_NL_EVERYMIN:
        {
            rVecOutput.push_back(pTime->leadtext);

            // Quarter past 1 => 14 minutes to half two (in Dutch this is correct, English I don't know)
            int quarterNum = nMinutes / 15;
            min1 = nMinutes % 15;     
            if (quarterNum > 0 && !(quarterNum == 1 && min1 == 0)) hours++; // Increase the hour, but do NOT increase when we are at exactly a quarter past
            hours %= 12; // Limit hours to 12
            
            const ledpos_t* pMinuteWords[15] {NULL,pTime->minute_1, pTime->minute_2, pTime->minute_3, pTime->minute_4, 
                    pTime->minute_5, pTime->minute_6, pTime->minute_7, pTime->minute_8, pTime->minute_9, pTime->minute_10, 
                    pTime->minute_11, pTime->minute_12, pTime->minute_13, pTime->minute_14};

            // Determine to/past
            if (min1 == 0)
            {
                // Specials quarters:
                switch (quarterNum)
                {
                    case 0:
                        // Full hour, show the minute 1-12 as hour
                        rVecOutput.push_back(pMinuteWords[hours % 12]);
                        rVecOutput.push_back(pTime->hour_full);
                        addHours = false;
                        break;
                    case 1:                
                        rVecOutput.push_back(pTime->quarter);
                        rVecOutput.push_back(pTime->past_15); 
                        break;
                    case 2:                
                        rVecOutput.push_back(pTime->half_to);
                        break;
                    case 3:                
                        rVecOutput.push_back(pTime->quarter);
                        rVecOutput.push_back(pTime->to_15); 
                        break;
                }
            }
            else
            {
                if (quarterNum == 0 || quarterNum == 2)
                {
                    rVecOutput.push_back(pTime->past_5);
                }
                else if (quarterNum == 1 || quarterNum == 3)
                {
                    min1 = 15 - min1;
                    rVecOutput.push_back(pTime->to_5);
                }
                if (nMinutes > 15 && nMinutes < 45)
                {
                    rVecOutput.push_back(pTime->half_to);
                }
                
                rVecOutput.push_back(pMinuteWords[min1 % 15]);
            }               
        }
        break;
    
    case ETimeFormat::TF_NL_5MIN:
        {
            if (min5 > 3) hours++; // Increase the hour, but do NOT increase when we are at exactly a quarter past
            hours %= 12; // Limit hours to 12

            switch(min5)
            {
                case  0: rVecOutput.push_back(pTime->hour_full); break;
                case  1: rVecOutput.push_back(pTime->minute_5); rVecOutput.push_back(pTime->past_5); break;
                case  2: rVecOutput.push_back(pTime->minute_10); rVecOutput.push_back(pTime->past_10); break;
                case  3: rVecOutput.push_back(pTime->quarter);   rVecOutput.push_back(pTime->past_15); break;
                case  4: rVecOutput.push_back(pTime->minute_10); rVecOutput.push_back(pTime->to_10); rVecOutput.push_back(pTime->half_to); break;
                case  5: rVecOutput.push_back(pTime->minute_5); rVecOutput.push_back(pTime->to_5); rVecOutput.push_back(pTime->half_to); break;
                case  6: rVecOutput.push_back(pTime->half_to); break;
                case  7: rVecOutput.push_back(pTime->minute_5); rVecOutput.push_back(pTime->past_5); rVecOutput.push_back(pTime->half_past); break;
                case  8: rVecOutput.push_back(pTime->minute_10); rVecOutput.push_back(pTime->past_10); rVecOutput.push_back(pTime->half_past); break;
                case  9: rVecOutput.push_back(pTime->quarter);   rVecOutput.push_back(pTime->to_15); break;
                case 10: rVecOutput.push_back(pTime->minute_10); rVecOutput.push_back(pTime->to_10); break;
                case 11: rVecOutput.push_back(pTime->minute_5); rVecOutput.push_back(pTime->to_5); break;
            }
            
            const ledpos_t* pMinute5Words[5] {NULL, pTime->minute_1, pTime->minute_2, pTime->minute_3,pTime->minute_4};
            if (min1 > 0)
            {
                rVecOutput.push_back(pMinute5Words[min1]);
            }
        }
        break;

    case ETimeFormat::TF_EN_5MIN:
    default:
        {
            if (min5 > 6) hours++; // Increase the hour, when after the half
            hours %= 12; // Limit hours to 12

            switch(min5)
            {
                case  0: rVecOutput.push_back(pTime->hour_full); break;
                case  1: rVecOutput.push_back(pTime->minute_5); rVecOutput.push_back(pTime->past_5); break;
                case  2: rVecOutput.push_back(pTime->minute_10); rVecOutput.push_back(pTime->past_10); break;
                case  3: rVecOutput.push_back(pTime->quarter);   rVecOutput.push_back(pTime->past_15); break;
                case  4: rVecOutput.push_back(pTime->minute_20); rVecOutput.push_back(pTime->past_20); break;
                case  5: rVecOutput.push_back(pTime->minute_25); rVecOutput.push_back(pTime->past_25); break;
                case  6: rVecOutput.push_back(pTime->half_past); break;
                case  7: rVecOutput.push_back(pTime->minute_25); rVecOutput.push_back(pTime->to_25); break;
                case  8: rVecOutput.push_back(pTime->minute_20); rVecOutput.push_back(pTime->to_20); break;
                case  9: rVecOutput.push_back(pTime->quarter);   rVecOutput.push_back(pTime->to_15); break;
                case 10: rVecOutput.push_back(pTime->minute_10); rVecOutput.push_back(pTime->to_10); break;
                case 11: rVecOutput.push_back(pTime->minute_5); rVecOutput.push_back(pTime->to_5); break;
            }
            
            const ledpos_t* pMinute5Words[5] {NULL, pTime->minute_1, pTime->minute_2, pTime->minute_3,pTime->minute_4};
            if (min1 > 0)
            {
                rVecOutput.push_back(pMinute5Words[min1]);
            }
        }
        break;
    }

    // And finally, add the hour
    if (addHours)
    {
        rVecOutput.push_back(pLayout->hours[hours % 12]);
    }
}

//
// @brief  Convert the seconds into words for the given clock layout
//
void ClockTimeWordConvertor::convertSeconds(const ledclocklayout_t* pLayout, std::vector<const ledpos_t*> &rVecOutput)
{
    if (pLayout == nullptr) return;
    
    int nSeconds = second(localTime);

    rVecOutput.push_back(pLayout->time.second);
}

//
// @brief  Convert the date into words for the given clock layout
//
void ClockTimeWordConvertor::convertDate(const ledclocklayout_t* pLayout, std::vector<const ledpos_t*> &rVecOutput)
{
    if (pLayout == nullptr) return;
    
    int monthday = day(localTime);
    int monthnum = month(localTime) - 1; // Januari = 1, we need it to be 0

    rVecOutput.push_back(pLayout->days[(monthday - 1) % 31]); // Days start at 1
    rVecOutput.push_back(pLayout->months[monthnum % 12]);
}


//
// @brief  Convert the date into words for the given clock layout
//
void ClockTimeWordConvertor::convertWeekDay(const ledclocklayout_t* pLayout, std::vector<const ledpos_t*> &rVecOutput)
{
    if (pLayout == nullptr) return;
    
    // check the time
    int nWeekDay = weekday(localTime) - 1; // Weekday returns (1 - 7), Sunday = 1
    rVecOutput.push_back(pLayout->weekdays[nWeekDay % 7]);
}