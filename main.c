/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "board.h"
#include "hal.h"
#include "ch.h"
#include "string.h"

static void columnCallback(GPTDriver* driver);


ioline_t ledColumns[NUM_COLUMN] = {
  LINE_LED_COL_1, 
  LINE_LED_COL_2, 
  LINE_LED_COL_3, 
  LINE_LED_COL_4, 
  LINE_LED_COL_5, 
  LINE_LED_COL_6, 
  LINE_LED_COL_7, 
  LINE_LED_COL_8, 
  LINE_LED_COL_9, 
  LINE_LED_COL_10,
  LINE_LED_COL_11,
  LINE_LED_COL_12,
  LINE_LED_COL_13,
  LINE_LED_COL_14
};

ioline_t ledRows[NUM_ROW * 3] = {
  LINE_LED_ROW_1_R,
  LINE_LED_ROW_1_G,
  LINE_LED_ROW_1_B,
  LINE_LED_ROW_2_R,
  LINE_LED_ROW_2_G,
  LINE_LED_ROW_2_B,
  LINE_LED_ROW_3_R,
  LINE_LED_ROW_3_G,
  LINE_LED_ROW_3_B,
  LINE_LED_ROW_4_R,
  LINE_LED_ROW_4_G,
  LINE_LED_ROW_4_B,
  LINE_LED_ROW_5_R,
  LINE_LED_ROW_5_G,
  LINE_LED_ROW_5_B,
};

#define REFRESH_FREQUENCY           200

uint16_t ledColors[NUM_COLUMN * NUM_ROW] = {
  0xF00,0, 0x0F0,0, 0x00F, 0,0,0,0,0,0,0,0,0,
  0xF00,0, 0x0F0,0, 0x00F, 0,0,0,0,0,0,0,0,0,
  0xF00,0, 0x0F0,0, 0x00F, 0,0,0,0,0,0,0,0,0,
  0xF00,0, 0x0F0,0, 0x00F, 0,0,0,0,0,0,0,0,0,
  0xF00,0, 0x0F0,0, 0x00F, 0,0,0,0,0,0,0,0,0,
};
static uint32_t currentColumn = 0;
static uint32_t columnPWMCount = 0;

// BFTM0 Configuration, this runs at 15 * REFRESH_FREQUENCY Hz
static const GPTConfig bftm0Config = {
  .frequency = NUM_COLUMN * REFRESH_FREQUENCY * 2 * 16,
  .callback = columnCallback
};

static const SerialConfig usart1Config = {
  .speed = 115200
};

/*
 * Thread 1.
 */
THD_WORKING_AREA(waThread1, 128);
THD_FUNCTION(Thread1, arg) {

  (void)arg;

  while (true)
  {
  }
}

void columnCallback(GPTDriver* _driver)
{
  (void)_driver;
  if (columnPWMCount < 16)
  {
    for (size_t row = 0; row < NUM_ROW; row++)
    {
    const uint16_t row_color = ledColors[currentColumn + (NUM_COLUMN * row)];
    // R
    if (((row_color >> 8) & 0xF) > columnPWMCount)
      palSetLine(ledRows[row * 3]);
    else if (((row_color >> 8) & 0xF) <= columnPWMCount)
      palClearLine(ledRows[row * 3]);
    // G
    if (((row_color >> 4) & 0xF) > columnPWMCount)
      palSetLine(ledRows[row * 3 + 1]);
    else if (((row_color >> 4) & 0xF) <= columnPWMCount)
      palClearLine(ledRows[row * 3 + 1]);
    // B
    if (((row_color >> 0) & 0xF) > columnPWMCount)
      palSetLine(ledRows[row * 3 + 2]);
    else if (((row_color >> 0) & 0xF) <= columnPWMCount)
      palClearLine(ledRows[row * 3 + 2]);
    }
  }
  if (columnPWMCount == 15) {
    palClearLine(ledColumns[currentColumn]);
  }
  if (columnPWMCount > 16)
  {
    palClearLine(ledColumns[currentColumn]);
    if (++currentColumn >= NUM_COLUMN)
      currentColumn = 0;
    palSetLine(ledColumns[currentColumn]);
    columnPWMCount = 0;
  } else {
    columnPWMCount++;
  }
}

/*
 * Application entry point.
 */
int main(void) {
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  // Powerup LED
  palSetLine(LINE_LED_PWR);

  // Setup UART1
  sdStart(&SD1, &usart1Config);

  // Setup Column Multiplex Timer
  gptStart(&GPTD_BFTM0, &bftm0Config);
  gptStartContinuous(&GPTD_BFTM0, 1);

  // chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  /* This is now the idle thread loop, you may perform here a low priority
     task but you must never try to sleep or wait in this loop. Note that
     this tasks runs at the lowest priority level so any instruction added
     here will be executed after all other tasks have been started.*/
  while (true) {
  }
}
