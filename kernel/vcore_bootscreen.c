/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
*/
#include "../include/kernel.h"
#include "../include/vcore_logo.h"

extern void fb_but_pixel(uint32_t x, uint32_t y, uint32_t color);
extern void fb_clear(uint32_t color);
extern void font_draw_string_centered(uint32_t y, const char* str, uint32_t color, uint32_t bg, int scale);
extern uint32_t fb_get_width(void);
extern uint32_t fb_get_height(void);
extern uint32_t font_get_height(void);
extern int fb_is_ready(void);



#define COLOR_BG      0x0A0A14
#define COLOR_WHITE   0xFFFFFF
#define COLOR_GRAY    0x888888
#define COLOR_SPINNER 0xFFFFFF



static const int sin_table[360] = {
      0, 17, 34, 52, 69, 87, 104, 121, 139, 156, 173, 190,
      207,  224,  241,  258,  275,  292,  309,  325,  342,  358,  374,  390,
      406,  422,  438,  453,  469,  484,  499,  515,  529,  544,  559,  573,
      587,  601,  615,  629,  642,  656,  669,  681,  694,  707,  719,  731,
      743,  754,  766,  777,  788,  798,  809,  819,  829,  838,  848,  857,
      866,  874,  882,  891,  898,  906,  913,  920,  927,  933,  939,  945,
      951,  956,  961,  965,  970,  974,  978,  981,  984,  987,  990,  992,
      994,  996,  997,  998,  999,  999, 1000,  999,  999,  998,  997,  996,
      994,  992,  990,  987,  984,  981,  978,  974,  970,  965,  961,  956,
      951,  945,  939,  933,  927,  920,  913,  906,  898,  891,  882,  874,
      866,  857,  848,  838,  829,  819,  809,  798,  788,  777,  766,  754,
      743,  731,  719,  707,  694,  681,  669,  656,  642,  629,  615,  601,
      587,  573,  559,  544,  529,  515,  499,  484,  469,  453,  438,  422,
      406,  390,  374,  358,  342,  325,  309,  292,  275,  258,  241,  224,
      207,  190,  173,  156,  139,  121,  104,   87,   69,   52,   34,   17,
        0,  -17,  -34,  -52,  -69,  -87, -104, -121, -139, -156, -173, -190,
     -207, -224, -241, -258, -275, -292, -309, -325, -342, -358, -374, -390,
     -406, -422, -438, -453, -469, -484, -500, -515, -529, -544, -559, -573,
     -587, -601, -615, -629, -642, -656, -669, -681, -694, -707, -719,
     -743, -754, -766, -777, -788, -798, -809, -819, -829, -838, -848, -857,
     -866, -874, -882, -891, -898, -906, -913, -920, -927, -933, -939, -945,
     -951, -956, -961, -965, -970, -974, -978, -981, -984, -987, -990, -992,
     -994, -996, -997, -998, -999, -999,-1000, -999, -999, -998, -997, -996,
     -994, -992, -990, -987, -984, -981, -978, -974, -970, -965, -961, -956,
     -951, -945, -939, -933, -927, -920, -913, -906, -898, -891, -882, -874,
     -866, -857, -848, -838, -829, -819, -809, -798, -788, -777, -766, -754,
     -743, -731, -719, -707, -694, -681, -669, -656, -642, -629, -615, -601,
     -587, -573, -559, -544, -529, -515, -500, -484, -469, -453, -438, -422,
     -406, -390, -374, -358, -342, -325, -309, -292, -275, -258, -241, -224,
     -207, -190, -173, -156, -139, -121, -104,  -87,  -69, -52, -34, -17,
};

static const int cos_table[360] = {
1000,  999,  999,  998,  997,  996,  994,  992,  990,  987,  984,  981,
      978,  974,  970,  965,  961,  956,  951,  945,  939,  933,  927,  920,
      913,  906,  898,  891,  882,  874,  866,  857,  848,  838,  829,  819,
      809,  798,  788,  777,  766,  754,  743,  731,  719,  707,  694,  681,
      669,  656,  642,  629,  615,  601,  587,  573,  559,  544,  529,  515,
      500,  484,  469,  453,  438,  422,  406,  390,  374,  358,  342,  325,
      309,  292,  275,  258,  241,  224,  207,  190,  173,  156,  139,  121,
      104,   87,   69,   52,   34,   17,    0,  -17,  -34,  -52,  -69,  -87,
     -104, -121, -139, -156, -173, -190, -207, -224, -241, -258, -275, -292,
     -309, -325, -342, -358, -374, -390, -406, -422, -438, -453, -469, -484,
     -499, -515, -529, -544, -559, -573, -587, -601, -615, -629, -642, -656,
     -669, -681, -694, -707, -719, -731, -743, -754, -766, -777, -788, -798,
     -809, -819, -829, -838, -848, -857, -866, -874, -882, -891, -898, -906,
     -913, -920, -927, -933, -939, -945, -951, -956, -961, -965, -970, -974,
     -978, -981, -984, -987, -990, -992, -994, -996, -997, -998, -999, -999,
    -1000, -999, -999, -998, -997, -996, -994, -992, -990, -987, -984, -981,
     -978, -974, -970, -965, -961, -956, -951, -945, -939, -933, -927, -920,
     -913, -906, -898, -891, -882, -874, -866, -857, -848, -838, -829, -819,
     -809, -798, -788, -777, -766, -754, -743, -731, -719, -707, -694, -681,
     -669, -656, -642, -629, -615, -601, -587, -573, -559, -544, -529, -515,
     -500, -484, -469, -453, -438, -422, -406, -390, -374, -358, -342, -325,
     -309, -292, -275, -258, -241, -224, -207, -190, -173, -156, -139, -121,
     -104,  -87,  -69,  -52,  -34,  -17,    0,   17,   34,   52,   69,   87,
      104,  121,  139,  156,  173,  190,  207,  224,  241,  258,  275,  292,
      309,  325,  342,  358,  374,  390,  406,  422,  438,  453,  469,  484,
      500,  515,  529,  544,  559,  573,  587,  601,  615,  629,  642,  656,
      669,  681,  694,  707,  719,  731,  743,  754,  766,  777,  788,  798,
      809,  819,  829,  838,  848,  857,  866,  874,  882,  891,  898,  906,
      913,  920,  927,  933,  939,  945,  951,  956, 961,  965,  970,  974,
      978,  981,  984,  987,  990,  992,  994,  996, 997, 998, 999, 999,
};


#define SPINNER_SEGMENTS 12
#define SPINNER_R_OUTER  24
#define SPINNER_R_INNER  14
#define SPINNER_GAP      4

static int spinner_offset = 0;
static int spinner_cx     = 0;
static int spinner_cy     = 0;


/* draw_logo 128x128 */
static void draw_logo(void) {
  uint32_t w = fb_get_width();
  uint32_t h = fb_get_height();


  uint32_t logo_x = (w - LOGO_WIDTH) / 2;
  uint32_t logo_y = (h / 2) - LOGO_HEIGHT - 60;


  for (uint32_t row = 0; row < LOGO_HEIGHT; row++) {
      for (uint32_t col = 0; col < LOGO_WIDTH; col++) {
          uint32_t color = valiantcore_logo[row * LOGO_WIDTH + col];
          fb_but_pixel(logo_x + col, logo_y + row, color);
      }
   }
}


/* draw-segment */
static void draw_segment(int cx, int cy, int s, int e, int r_out, int r_in, uint32_t color) {
   for (int deg = s; deg <= e; deg++) {
       int d = deg % 360;
       if (d < 0) d += 360;
       for (int r = r_in; r <= r_out; r++) {
           int x = cx + (r * cos_table[d]) / 1000;
           int y = cy + (r * sin_table[d]) / 1000;
           fb_but_pixel((uint32_t)x, (uint32_t)y, color);
         }
     }
}

/* draw_spinner */
static void draw_spinner(int cx, int cy, int offset) {
    int seg_deg = 360 / SPINNER_SEGMENTS;

    for (int i = 0; i < SPINNER_SEGMENTS; i++) {
        int s = (i * seg_deg + SPINNER_GAP/2 + offset) % 360;
        if (s < 0) s += 360;
        int e = s + seg_deg + SPINNER_GAP;
        if (e >= 360) e = 359;

        int active = (SPINNER_SEGMENTS - 1 - i) % SPINNER_SEGMENTS;
        uint32_t color;

        if      (active == 0) color = 0xFFFFFF;
        else if (active == 1) color = 0xCCCCCC;
        else if (active <= 3) color = 0x888888;
        else                  color = 0x333333;

        draw_segment(cx, cy, s, e, SPINNER_R_OUTER, SPINNER_R_INNER, color);
    }

    for (int dy = -SPINNER_R_INNER; dy <= SPINNER_R_INNER; dy++) {
        for (int dx = -SPINNER_R_INNER; dx <= SPINNER_R_INNER; dx++) {
            if (dx*dx + dy*dy < SPINNER_R_INNER * SPINNER_R_INNER)
                fb_but_pixel((uint32_t)(cx+dx),
                            (uint32_t)(cy+dy), COLOR_BG);
        }
    }
}
                 
   
/* boot_screen_show */
void/**/boot_screen_show(void) {
    if/**/(!fb_is_ready()) return;


    uint32_t w  = fb_get_width();
    uint32_t h  = fb_get_height();
    uint32_t fh = font_get_height();


    fb_clear(COLOR_BG);


    draw_logo();

    spinner_cx     = (int)(w / 2);
    spinner_cy     = (int)(h / 2) + 20;
    spinner_offset = 0;

    draw_spinner(spinner_cx, spinner_cy, spinner_offset);


    font_draw_string_centered((uint32_t)(spinner_cy + SPINNER_R_OUTER + 20),
                               "By Finn Dev",
                               COLOR_GRAY, COLOR_BG, 1);
}
/* boot_screen_spin */
void boot_screen_spin(void) {
    if (!fb_is_ready()) return;

    int r = SPINNER_R_OUTER + 2;
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx*dx + dy*dy <= r*r)
                fb_but_pixel((uint32_t)(spinner_cx + dx),
                             (uint32_t)(spinner_cy + dy),
                             COLOR_BG);
        }
    }

    spinner_offset = (spinner_offset + 30) % 360;
    draw_spinner(spinner_cx, spinner_cy, spinner_offset);
}
