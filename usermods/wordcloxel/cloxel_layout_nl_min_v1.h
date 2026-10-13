#pragma once

#include "cloxel_layout_base.h"

// How many leds are connected?
#define NUM_LEDS    256
#define NUM_ROWS    16
#define NUM_COLS    16

//
// Dutch with all minutes
//
// Latest characters:
//   0123456789012345
// ----------------
// 0|HETMISPKWARTESTο
// 1|BTWEENZESDERTIEN
// 2|FDRIELFZEVENEGEN
// 3|EVIERACHTWAALFOK
// 4|EVIJFVEERTIENUUR
// 5|SOVERVOORXHALFTW
//   0123456789012345
// 6|TTWEENEGENOTIENO
// 7|DDRIELFZEVENVIER
// 8|AVIJFACHTWAALFED
// 9|GZESNEETENSTIJDC
// 0|MADIWODOVRZAZONL
// 1|101121314151617O
// 2|181922023242526X
// 3|27282930JJANFEBE
// 4|MAAPRJUNOVDECJUL
// 5|JB²AUGSEPOKTMEI•
//


// Word definitions, {-1,-1} or WEND terminated (led numbers start at 0)

static const ledpos_t NLMINV1_leadtext  [] = { {0,0}, {1,0}, {2,0}, {4,0}, {5,0}, WEND};

static const ledpos_t NLMINV1_minute_1  [] = { {3,1}, {4,1}, {5,1}, WEND};
static const ledpos_t NLMINV1_minute_2  [] = { {1,1}, {2,1}, {3,1}, {4,1}, WEND};
static const ledpos_t NLMINV1_minute_3  [] = { {1,2}, {2,2}, {3,2}, {4,2}, WEND};
static const ledpos_t NLMINV1_minute_4  [] = { {1,3}, {2,3}, {3,3}, {4,3}, WEND};
static const ledpos_t NLMINV1_minute_5  [] = { {1,4}, {2,4}, {3,4}, {4,4}, WEND};
static const ledpos_t NLMINV1_minute_6  [] = { {6,1}, {7,1}, {8,1}, WEND};
static const ledpos_t NLMINV1_minute_7  [] = { {7,2}, {8,2}, {9,2}, {10,2}, {11,2}, WEND};
static const ledpos_t NLMINV1_minute_8  [] = { {5,3}, {6,3}, {7,3}, {8,3}, WEND};
static const ledpos_t NLMINV1_minute_9  [] = { {11,2}, {12,2}, {13,2}, {14,2}, {15,2}, WEND};
static const ledpos_t NLMINV1_minute_10 [] = { {12,1}, {13,1}, {14,1}, {15,1}, WEND};
static const ledpos_t NLMINV1_minute_11 [] = { {4,2}, {5,2}, {6,2}, WEND};
static const ledpos_t NLMINV1_minute_12 [] = { {8,3}, {9,3}, {10,3}, {11,3}, {12,3}, {13,3}, WEND};
static const ledpos_t NLMINV1_minute_13 [] = { {9,1}, {10,1}, {11,1}, {12,1}, {13,1}, {14,1}, {15,1}, WEND};
static const ledpos_t NLMINV1_minute_14 [] = { {5,4}, {6,4}, {7,4}, {8,4}, {9,4}, {10,4}, {11,4}, {12,4}, WEND};
static const ledpos_t NLMINV1_quarter   [] = { {7,0}, {8,0}, {9,0}, {10,0}, {11,0}, WEND};

static const ledpos_t NLMINV1_pulse     [] = { {15,15}, WEND};

static const ledpos_t NLMINV1_past      [] = { {1,5}, {2,5}, {3,5}, {4,5}, WEND};
static const ledpos_t NLMINV1_to        [] = { {5,5}, {6,5}, {7,5}, {8,5}, WEND};
static const ledpos_t NLMINV1_half1     [] = { {10,5}, {11,5}, {12,5}, {13,5}, WEND};
static const ledpos_t NLMINV1_half2     [] = { {10,5}, {11,5}, {12,5}, {13,5}, WEND};

static const ledpos_t NLMINV1_hour_1    [] = { {3,6}, {4,6}, {5,6}, WEND};
static const ledpos_t NLMINV1_hour_2    [] = { {1,6}, {2,6}, {3,6}, {4,6}, WEND};
static const ledpos_t NLMINV1_hour_3    [] = { {1,7}, {2,7}, {3,7}, {4,7}, WEND};
static const ledpos_t NLMINV1_hour_4    [] = { {12,7}, {13,7}, {14,7}, {15,7}, WEND};
static const ledpos_t NLMINV1_hour_5    [] = { {1,8}, {2,8}, {3,8}, {4,8}, WEND};
static const ledpos_t NLMINV1_hour_6    [] = { {1,9}, {2,9}, {3,9}, WEND};
static const ledpos_t NLMINV1_hour_7    [] = { {7,7}, {8,7}, {9,7}, {10,7}, {11,7}, WEND};
static const ledpos_t NLMINV1_hour_8    [] = { {5,8}, {6,8}, {7,8}, {8,8}, WEND};
static const ledpos_t NLMINV1_hour_9    [] = { {5,6}, {6,6}, {7,6}, {8,6}, {9,6}, WEND};
static const ledpos_t NLMINV1_hour_10   [] = { {11,6}, {12,6}, {13,6}, {14,6}, WEND};
static const ledpos_t NLMINV1_hour_11   [] = { {4,7}, {5,7}, {6,7}, WEND};
static const ledpos_t NLMINV1_hour_12   [] = { {8,8}, {9,8}, {10,8}, {11,8}, {12,8}, {13,8}, WEND};

static const ledpos_t NLMINV1_hour_full [] = { {13,4}, {14,4}, {15,4}, WEND};

static const ledpos_t NLMINV1_monday    [] = { {0,10}, {1,10}, WEND};
static const ledpos_t NLMINV1_tuesday   [] = { {2,10}, {3,10}, WEND};
static const ledpos_t NLMINV1_wednesday [] = { {4,10}, {5,10}, WEND};
static const ledpos_t NLMINV1_thursday  [] = { {6,10}, {7,10}, WEND};
static const ledpos_t NLMINV1_friday    [] = { {8,10}, {9,10}, WEND};
static const ledpos_t NLMINV1_saturday  [] = { {10,10}, {11,10}, WEND};
static const ledpos_t NLMINV1_sunday    [] = { {12,10}, {13,10}, {14,10}, WEND};

static const ledpos_t NLMINV1_day0      [] = { {1,11}, WEND};
static const ledpos_t NLMINV1_day1      [] = { {2,11}, WEND};
static const ledpos_t NLMINV1_day2      [] = { {4,11}, WEND};
static const ledpos_t NLMINV1_day3      [] = { {6,11}, WEND};
static const ledpos_t NLMINV1_day4      [] = { {8,11}, WEND};
static const ledpos_t NLMINV1_day5      [] = { {10,11}, WEND};
static const ledpos_t NLMINV1_day6      [] = { {12,11}, WEND};
static const ledpos_t NLMINV1_day7      [] = { {14,11}, WEND};
static const ledpos_t NLMINV1_day8      [] = { {1,12}, WEND};
static const ledpos_t NLMINV1_day9      [] = { {3,12}, WEND};
static const ledpos_t NLMINV1_day10     [] = { {0,11}, {1,11}, WEND};
static const ledpos_t NLMINV1_day11     [] = { {2,11}, {3,11}, WEND};
static const ledpos_t NLMINV1_day12     [] = { {3,11}, {4,11}, WEND};
static const ledpos_t NLMINV1_day13     [] = { {5,11}, {6,11}, WEND};
static const ledpos_t NLMINV1_day14     [] = { {7,11}, {8,11}, WEND};
static const ledpos_t NLMINV1_day15     [] = { {9,11}, {10,11}, WEND};
static const ledpos_t NLMINV1_day16     [] = { {11,11}, {12,11}, WEND};
static const ledpos_t NLMINV1_day17     [] = { {13,11}, {14,11}, WEND};
static const ledpos_t NLMINV1_day18     [] = { {0,12}, {1,12}, WEND};
static const ledpos_t NLMINV1_day19     [] = { {2,12}, {3,12}, WEND};
static const ledpos_t NLMINV1_day20     [] = { {5,12}, {6,12}, WEND};
static const ledpos_t NLMINV1_day21     [] = { {4,11}, {5,11}, WEND};
static const ledpos_t NLMINV1_day22     [] = { {4,12}, {5,12}, WEND};
static const ledpos_t NLMINV1_day23     [] = { {7,12}, {8,12}, WEND};
static const ledpos_t NLMINV1_day24     [] = { {9,12}, {10,12}, WEND};
static const ledpos_t NLMINV1_day25     [] = { {11,12}, {12,12}, WEND};
static const ledpos_t NLMINV1_day26     [] = { {13,12}, {14,12}, WEND};
static const ledpos_t NLMINV1_day27     [] = { {0,13}, {1,13}, WEND};
static const ledpos_t NLMINV1_day28     [] = { {2,13}, {3,13}, WEND};
static const ledpos_t NLMINV1_day29     [] = { {4,13}, {5,13}, WEND};
static const ledpos_t NLMINV1_day30     [] = { {6,13}, {7,13}, WEND};
static const ledpos_t NLMINV1_day31     [] = { {6,11}, {7,11}, WEND};

static const ledpos_t NLMINV1_januari  [] = { {9,13}, {10,13}, {11,13}, WEND};
static const ledpos_t NLMINV1_februari [] = { {12,13}, {13,13}, {14,13}, WEND};
static const ledpos_t NLMINV1_march    [] = { {0,14}, {1,14}, {2,14}, WEND};
static const ledpos_t NLMINV1_april    [] = { {2,14}, {3,14}, {4,14}, WEND};
static const ledpos_t NLMINV1_may      [] = { {12,15}, {13,15}, {14,15}, WEND};
static const ledpos_t NLMINV1_june     [] = { {5,14}, {6,14}, {7,14}, WEND};
static const ledpos_t NLMINV1_july     [] = { {13,14}, {14,14}, {15,14}, WEND};
static const ledpos_t NLMINV1_august   [] = { {3,15}, {4,15}, {5,15}, WEND};
static const ledpos_t NLMINV1_september[] = { {6,15}, {7,15}, {8,15}, WEND};
static const ledpos_t NLMINV1_october  [] = { {9,15}, {10,15}, {11,15}, WEND};
static const ledpos_t NLMINV1_november [] = { {7,14}, {8,14}, {9,14}, WEND};
static const ledpos_t NLMINV1_december [] = { {10,14}, {11,14}, {12,14}, WEND};

static const ledpos_t NLMINV1_cloxel  [] = { {15,9}, {15,10}, {15,11}, {15,12}, {15,13}, {15,14}, WEND};
static const ledpos_t NLMINV1_word  [] = { {15,5}, {15,6}, {15,7}, {15,8}, WEND};
static const ledpos_t NLMINV1_jb2  [] = { {0,15}, {1,15}, {2,15}, WEND};

static const ledpos_t NLMINV1_bt [] = { {15,0}, WEND};
static const ledpos_t NLMINV1_no  [] = { {4,9}, {5,9}, {6,9}, WEND};
static const ledpos_t NLMINV1_yes  [] = { {9,13}, {10,13}, WEND};

static const ledpos_t NLMINV1_holiday [] = { {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {0,8}, {0,9}, WEND};
static const ledpos_t NLMINV1_dinner [] = { {6,9}, {7,9}, {8,9}, {9,9}, {10,9}, {11,9}, {12,9}, {13,9}, {14,9}, WEND};
static const ledpos_t NLMINV1_time [] = { {11,9}, {12,9}, {13,9}, {14,9}, WEND};

static const ledpos_t NLMINV1_oops  [] = { {3,13}, {4,13}, {5,13}, WEND};

//
// New NL clock layout
//
static const ledclocklayout_t s_layoutNL_MIN_V1 = 
{
    16,
    16,
    256,
    O_NORMAL,
    false,
    
    // Time display method
    TF_NL_EVERYMIN,

    // Time parts
    {
        NLMINV1_pulse, NLMINV1_leadtext, 
        NLMINV1_minute_1, NLMINV1_minute_2, NLMINV1_minute_3, NLMINV1_minute_4, NLMINV1_minute_5, NLMINV1_minute_6, NLMINV1_minute_7, NLMINV1_minute_8, NLMINV1_minute_9, NLMINV1_minute_10,
        NLMINV1_minute_11, NLMINV1_minute_12, NLMINV1_minute_13, NLMINV1_minute_14, NLMINV1_quarter, EMPTY, EMPTY, 
        NLMINV1_past, NLMINV1_to, NLMINV1_past, NLMINV1_to, NLMINV1_past, NLMINV1_to, NLMINV1_past, NLMINV1_to, NLMINV1_past, NLMINV1_to, 
        NLMINV1_half1, NLMINV1_half2, NLMINV1_hour_full
    },

    // 12x Hours
    {
        NLMINV1_hour_12, NLMINV1_hour_1, NLMINV1_hour_2, NLMINV1_hour_3, NLMINV1_hour_4, NLMINV1_hour_5, NLMINV1_hour_6, NLMINV1_hour_7, NLMINV1_hour_8, NLMINV1_hour_9, NLMINV1_hour_10, NLMINV1_hour_11         
    },

    // 7x Day of the week
    {
        NLMINV1_sunday, NLMINV1_monday, NLMINV1_tuesday, NLMINV1_wednesday, NLMINV1_thursday, NLMINV1_friday, NLMINV1_saturday
    },

    // 31x Day number
    {
        NLMINV1_day1, NLMINV1_day2, NLMINV1_day3, NLMINV1_day4, NLMINV1_day5, NLMINV1_day6, NLMINV1_day7, NLMINV1_day8, NLMINV1_day9, NLMINV1_day10, 
        NLMINV1_day11, NLMINV1_day12, NLMINV1_day13, NLMINV1_day14, NLMINV1_day15, NLMINV1_day16, NLMINV1_day17, NLMINV1_day18, NLMINV1_day19, NLMINV1_day20, 
        NLMINV1_day21, NLMINV1_day22, NLMINV1_day23, NLMINV1_day24, NLMINV1_day25, NLMINV1_day26, NLMINV1_day27, NLMINV1_day28, NLMINV1_day29, NLMINV1_day30, 
        NLMINV1_day31
    },

    // 12x Month
    {
        NLMINV1_januari, NLMINV1_februari, NLMINV1_march, NLMINV1_april, NLMINV1_may, NLMINV1_june, NLMINV1_july, NLMINV1_august, NLMINV1_september, NLMINV1_october, NLMINV1_november, NLMINV1_december
    },

    // Extra parts
    {
        NLMINV1_no,
        NLMINV1_yes,
        EMPTY,
        NLMINV1_cloxel,
        NLMINV1_word,
        NLMINV1_jb2,
        NLMINV1_bt,
        EMPTY,
        EMPTY,
        EMPTY,
        NLMINV1_holiday,
        EMPTY,
        EMPTY,
        EMPTY,
        EMPTY,
        EMPTY,
        NLMINV1_dinner,
        NLMINV1_time,
        NLMINV1_oops
    },

    // 10x number
    {
        NLMINV1_day0, NLMINV1_day1, NLMINV1_day2, NLMINV1_day3, NLMINV1_day4, NLMINV1_day5, NLMINV1_day6, NLMINV1_day7, NLMINV1_day8, NLMINV1_day9
    }
};

// // List all words 

// static const ledtexts_t s_wordCodes[] = {
//     {code1, "101"}, {code2, "011"}, {code3, "112"}, {code4, "121"}, {code5, "213"}, {code6, "131"}, {code7, "314"}, {code8, "141"}, {code9, "415"}, {code10, "151"}, {code11, "516"}, {code12, "161"}, {code13, "617"},
//     {code14, "181"}, {code15, "819"}, {code16, "192"}, {code17, "922"}, {code18, "220"}, {code19, "202"}, {code20, "023"}, {code21, "232"}, {code22, "324"}, {code23, "242"}, {code24, "425"}, {code25, "252"}, {code26, "526"},
//     {code27, "272"}, {code28, "728"}, {code29, "282"}, {code30, "829"}, {code31, "293"}, {code32, "930"},
//     NULL };

