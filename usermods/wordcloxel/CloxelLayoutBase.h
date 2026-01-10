
#pragma once
#include <cstdint>

#ifndef NULL
#define NULL    (void *)0
#endif

#define WEND  {-1,-1}

typedef struct {
    int8_t   x; // X-Pos or Column
    int8_t   y; // Y-Pos or Row
} ledpos_t;

typedef struct {
    const ledpos_t  *leds; // Indices of leds to light up
    const char      *text; // Description
} ledtexts_t;

enum ETimeFormat
{
    TF_NL_5MIN,      // Dutch, 5, 10, quarter, to, past + 4 separate minute dots. i.e. "vijf voor half tien" with 2 dots indicating actual "3 voor half 10"
    TF_NL_EVERYMIN,  // Dutch, Every minute a single word, one, two, three, etc. i.e. "dertien voor half tien"
    TF_EN_5MIN       // English, 5, 10, quarter, 20, 25 to/past "twenty-five past ten"
};

enum EOrientation
{
    O_NORMAL,        // Normal orientation
    O_ROTATED,       // Rotated display (90 degrees)
};

// Definitions for times text
typedef struct {    
    const ledpos_t *second;
    const ledpos_t *leadtext; 
    const ledpos_t *minute_1;  
    const ledpos_t *minute_2;
    const ledpos_t *minute_3;
    const ledpos_t *minute_4;
    const ledpos_t *minute_5;
    const ledpos_t *minute_6;
    const ledpos_t *minute_7;
    const ledpos_t *minute_8;
    const ledpos_t *minute_9;
    const ledpos_t *minute_10;
    const ledpos_t *minute_11;
    const ledpos_t *minute_12;
    const ledpos_t *minute_13;
    const ledpos_t *minute_14;
    const ledpos_t *quarter;
    const ledpos_t *minute_20;
    const ledpos_t *minute_25;
    const ledpos_t *past_5;     // Past for `five past` = default for past
    const ledpos_t *to_5;       // To for `five to` = default for to
    const ledpos_t *past_10;    // Past for `ten past`
    const ledpos_t *to_10;      // To for `ten to`
    const ledpos_t *past_15;    // Past for `quarter past`
    const ledpos_t *to_15;      // To for `quarter to`
    const ledpos_t *past_20;    // Past for `twenty past`
    const ledpos_t *to_20;      // To for `twenty to`
    const ledpos_t *past_25;    // Past for `twenty-five past`
    const ledpos_t *to_25;      // To for `twenty-five to`

    const ledpos_t *half_to;    // Half to use for 'voor half xxx' = default 'half'
    const ledpos_t *half_past;  // Half to use for 'over half xxx'
    const ledpos_t *hour_full;    
} ledtime_t;

// Definitions for extra words
typedef struct {
    const ledpos_t *no;
    const ledpos_t *yes;
    const ledpos_t *wifi;
    const ledpos_t *myriadclock;
    const ledpos_t *word; // From WORDcloxel (to have the word in another color)
    const ledpos_t *myriadbits;
    const ledpos_t *bluetooth;
    const ledpos_t *passcode;
    const ledpos_t *goodmorning;
    const ledpos_t *birthday;
    const ledpos_t *holiday;
    const ledpos_t *party;
    const ledpos_t *wakeup;
    const ledpos_t *heart;
    const ledpos_t *breakfast;
    const ledpos_t *lunch;
    const ledpos_t *dinner;    
} ledextra_t;

// empty led position
static const ledpos_t EMPTY[] = { WEND};

// Definitions for extra words
typedef struct 
{
    const int           columns;
    const int           rows;
    const int           numLeds;
    const EOrientation  orientation;
    const bool          pixelDisplay;
    const ETimeFormat   timeFormat;
    const ledtime_t     time;
    const ledpos_t*     hours[12];
    const ledpos_t*     weekdays[7];
    const ledpos_t*     days[31];
    const ledpos_t*     months[12];
    const ledextra_t    extra;
    const ledpos_t*     numbers[10];
} ledclocklayout_t;

