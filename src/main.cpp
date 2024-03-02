
/***********************************************************************
 _    ____ _  _ ___  _  _ ____ ___ ____ _ _ _ ____ ____ ___  _    ____ _ _
 |___ |--| |\/| |--' |\/| |--|  |  |--< | _X_ |--< |=== |--' |___ |--|  Y 

 *
 ***********************************************************************/

/*
***********************************************************************
 *  This file is part of the afterglow pinball lamp matrix WS2812
 *  replay project (PLM2812):
 *  https://github.com/bitfieldlabs/pinball_lamp_matrix_replay_ws2812
 *
 *  PLM2812 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  PLM2812 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with afterglow.
 *  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include "replay_t2.h"


//------------------------------------------------------------------------------
// Setup

// Replay time base (1 ttag = 20ms)
#define REPLAY_TIME_INT 20

// Pico GPIO used for WS2812 data output
#define PIN        4

// Number of LEDs in the strand
#define NUMPIXELS 64

// Number of lamp matrix columns
#define NUM_COL 8

// LED brightness scale (1=full, 2=half, 3=quarter etc.)
#define LED_BRIGHTNESS_SCALE 2

// Use LED colors (1=color, 0=off/all white)
#define LED_COLORS 1


//------------------------------------------------------------------------------
// Static variables

static Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

static uint32_t sTtagReplay = 0;

// Replay lamp matrix
static byte sReplayLamps[NUM_COL] = {0};

// Last replay update ttag
static uint32_t sReplayLastUpdTtag = 0;

// Replay event position
static uint32_t sReplayPos = 0;

// Current replay event
static AG_LAMP_SWITCH_2B_t sReplayEvent = {0};


//------------------------------------------------------------------------------
void setup()
{
    pixels.begin();
    Serial.begin(115200);
}

//------------------------------------------------------------------------------
void getNextEvent()
{
    // read the next byte
    uint8_t evByte = *((uint8_t*)kLampReplay + sReplayPos);

    // check if this is a two-byte event marker
    if (evByte != AG_REPLAY_2B_MARKER)
    {
        // single byte event -> convert to two byte
        AG_LAMP_SWITCH_1B_t ev1B;
        *((uint8_t*)&ev1B) = evByte;
        sReplayEvent.col = ev1B.col;
        sReplayEvent.row = ev1B.row;
        sReplayEvent.dttag = ev1B.dttag;
        sReplayPos++;
    }
    else
    {
        // two byte event
        // read the next two bytes
        uint16_t evBytes;
        // memcpy to avoid unaligned access
        // swap bytes for endianess
        memcpy(&evBytes, (uint8_t*)kLampReplay + sReplayPos + 2, 1);
        memcpy((uint8_t*)evBytes+1, (uint8_t*)kLampReplay + sReplayPos + 1, 1);

        uint16_t *pReplay2B = (uint16_t*)&sReplayEvent;
        *pReplay2B = evBytes;
        sReplayPos += 3;
    }
}

//------------------------------------------------------------------------------
void replay()
{
    // update the lamp matrix
    if (sReplayLastUpdTtag == 0)
    {
        sReplayLastUpdTtag = sTtagReplay;
    }
    uint32_t dTtag = (sTtagReplay - sReplayLastUpdTtag);
    if (dTtag >= sReplayEvent.dttag)
    {
        // handle all events of this ttag
        do
        {
            getNextEvent();
            sReplayLamps[sReplayEvent.col] ^= (1 << sReplayEvent.row);

            // check whether we have read the last event
            if (sReplayPos >= sizeof(kLampReplay))
            {
                // start over again
                sReplayPos = 0;
                memset(sReplayLamps, 0, sizeof(sReplayLamps));
            }
        } while (sReplayEvent.dttag == 0);
        sReplayLastUpdTtag = sTtagReplay;
    }
}

//------------------------------------------------------------------------------
void loop()
{
    /*
    for (int i=0; i<64; i++)
    {
        pixels.setPixelColor(i, pixels.Color(55, 55, 0));
        pixels.show();
        delay(500);
    }
    */
    uint32_t m1 = millis();
    byte col = 0;
    byte row = 0;
    byte lampMask = 0x01;

    // update all LEDs
    for(int i=0; i<NUMPIXELS; i++)
    {
        if (sReplayLamps[col] & lampMask)
        {
            byte r, g, b;
#if (LED_COLORS)
            r = kLampColors[col][row].r >> LED_BRIGHTNESS_SCALE;
            g = kLampColors[col][row].g >> LED_BRIGHTNESS_SCALE;
            b = kLampColors[col][row].b >> LED_BRIGHTNESS_SCALE;
#else
                r = g = b = (255 >> LED_BRIGHTNESS_SCALE);
#endif
            pixels.setPixelColor(i, pixels.Color(r, g, b));
        }
        else
        {
            pixels.setPixelColor(i, pixels.Color(0, 0, 0));
        }
        lampMask <<= 1;
        row++;
        if (lampMask == 0)
        {
            col++;
            lampMask = 0x01;
            row = 0;
        }
    }
    pixels.show();

    sTtagReplay++;
    replay();
    
    // create a REPLAY_TIME_INT ms update interval
    uint32_t m2 = millis();
    uint32_t dm = (m2-m1);
    if (dm < REPLAY_TIME_INT)
    {
        delay(REPLAY_TIME_INT-dm);
    }

}
