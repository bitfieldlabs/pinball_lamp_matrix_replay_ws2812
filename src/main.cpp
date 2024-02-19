#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include "replay_totan.h"


#define PIN        4
#define NUMPIXELS 64

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define NUM_COL 8

static uint32_t sTtagReplay = 0;

// Replay lamp matrix
static byte sReplayLamps[NUM_COL] = {0};

// Last replay update ttag
static uint32_t sReplayLastUpdTtag = 0;

// Replay event position
static uint32_t sReplayPos = 0;

// Current replay event
static AG_LAMP_SWITCH_2B_t sReplayEvent = {0};


void setup()
{
    pixels.begin();
    Serial.begin(115200);
}

//------------------------------------------------------------------------------
void getNextEvent()
{
    // read the next byte
    byte evByte = *(byte*)(kLampReplay + sReplayPos);

    // check if this is a two-byte event marker
    if (evByte != AG_REPLAY_2B_MARKER)
    {
        // single byte event -> convert to two byte
        AG_LAMP_SWITCH_1B_t ev1B;
        *((byte*)&ev1B) = evByte;
        sReplayEvent.col = ev1B.col;
        sReplayEvent.row = ev1B.row;
        sReplayEvent.dttag = ev1B.dttag;
        sReplayPos++;
    }
    else
    {
        // two byte event
        // read the next two bytes
        uint16_t evBytes = pgm_read_word_near(kLampReplay + sReplayPos + 1);
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

void loop()
{/*
    pixels.clear();

    for(int i=0; i<NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(0, 10, 0));

        pixels.show();

        delay(100);
    }
    */

    uint32_t m1 = millis();
    byte c = (NUM_COL-1);
    byte row = 0;
    byte lampMask = 0x01;

    for(int i=0; i<NUMPIXELS; i++)
    {
        if (sReplayLamps[c] & lampMask)
        {
            byte r = kLampColors[c][row].r >> 4;
            byte g = kLampColors[c][row].g >> 4;
            byte b = kLampColors[c][row].b >> 4;
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
            c--;
            lampMask = 0x01;
            row = 0;
        }
        if (c>NUM_COL) c=(NUM_COL-1);
    }
    pixels.show();

    sTtagReplay++;
    replay();
    
    // create a 16ms interval
    uint32_t m2 = millis();
    uint32_t dm = (m2-m1);
    if (dm < 16)
    {
        delay(16-dm);
    }
}
