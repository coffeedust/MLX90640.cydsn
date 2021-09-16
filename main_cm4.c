/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "MLX90640.h"
#include "MLX90640_API.h"

int main(void)
{
    float frame[32*24]; // buffer for full frame of temperatures
    
    __enable_irq(); /* Enable global interrupts. */
    
    mUART_Start();
    CyDelay(100);

    mUART_PutString("Adafruit MLX90640 Simple Test\n");
    if (! begin()) {
        mUART_PutString("MLX90640 not found!\n");
        while (1) CyDelay(10);
    }
    mUART_PutString("Found Adafruit MLX90640\n");
    
    char str[255];
    sprintf(str, "Serial number: %02x%02x%02x\n", serialNumber[0], serialNumber[1], serialNumber[2]);
    mUART_PutString(str);
  
  //mlx.setMode(MLX90640_INTERLEAVED);
    setMode(MLX90640_CHESS);
    mUART_PutString("Current mode: ");
    if (getMode() == MLX90640_CHESS) {
        mUART_PutString("Chess\n");
    } else {
        mUART_PutString("Interleave\n");    
    }

    setResolution(MLX90640_ADC_18BIT);
    mUART_PutString("Current resolution: ");
    mlx90640_resolution_t res = getResolution();
    switch (res) {
        case MLX90640_ADC_16BIT: mUART_PutString("16 bit\n"); break;
        case MLX90640_ADC_17BIT: mUART_PutString("17 bit\n"); break;
        case MLX90640_ADC_18BIT: mUART_PutString("18 bit\n"); break;
        case MLX90640_ADC_19BIT: mUART_PutString("19 bit\n"); break;
    }

    setRefreshRate(MLX90640_2_HZ);
    mUART_PutString("Current frame rate: ");
    mlx90640_refreshrate_t rate = getRefreshRate();
    switch (rate) {
        case MLX90640_0_5_HZ: mUART_PutString("0.5 Hz\n"); break;
        case MLX90640_1_HZ: mUART_PutString("1 Hz\n"); break; 
        case MLX90640_2_HZ: mUART_PutString("2 Hz\n"); break;
        case MLX90640_4_HZ: mUART_PutString("4 Hz\n"); break;
        case MLX90640_8_HZ: mUART_PutString("8 Hz\n"); break;
        case MLX90640_16_HZ: mUART_PutString("16 Hz\n"); break;
        case MLX90640_32_HZ: mUART_PutString("32 Hz\n"); break;
        case MLX90640_64_HZ: mUART_PutString("64 Hz\n"); break;
    }
    for(;;)
    {
        CyDelay(500);
        if (getFrame(frame) != 0) {
            mUART_PutString("Failed\n");
            while(1);
        }
        mUART_PutString("\n\n");
  for (uint8_t h=0; h<24; h++) {
    for (uint8_t w=0; w<32; w++) {
      float t = frame[h*32 + w];
#define PRINT_ASCIIART
#ifdef PRINT_TEMPERATURES
      sprintf(str, "%01d", t);
      mUART_PutString(str);
      mUART_PutString(", ");
#endif
#ifdef PRINT_ASCIIART
      char c = '&';
      if (t < 20) c = ' ';
      else if (t < 23) c = '.';
      else if (t < 25) c = '-';
      else if (t < 27) c = '*';
      else if (t < 29) c = '+';
      else if (t < 31) c = 'x';
      else if (t < 33) c = '%';
      else if (t < 35) c = '#';
      else if (t < 37) c = 'X';
      mUART_Put(c);
#endif
    }
    mUART_PutString("\n");
  }
    }
}

/* [] END OF FILE */
