/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "ws2812_array.h"
#include "aht20.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// mode
Mode_Select mode_select = FUNCTION_OFF;

int key1_press_trigger_state = 0;
int key1_release_trigger_state = 0;
int key2_trigger_state = 0;
int mode_trigger_state = 0;

// bluetooth
int numbers[5];
uint8_t receive_data[20];
char send_data[100];
extern DMA_HandleTypeDef hdma_usart2_rx;

// oled
char percent_000[] = "                     ";
char percent_025[] = "=======              ";
char percent_050[] = "==============       ";
char percent_075[] = "=====================";

// touch
int touch_state = 0;

// bluetooth buffer
char buffer_1[100];
char buffer_2[100];

// pulse
int pulse_last_toggle_time_1 = 0;
int pulse_last_toggle_time_2 = 0;
int pulse_period_1 = 500;
int pulse_period_2 = 500;
int pulse_period_1_array_index = 0;
int pulse_period_2_array_index = 0;
const int pulse_period_array[3] = {500, 1500, 3000};
int pulse_led_state_1 = 0;
int pulse_led_state_2 = 0;

// breathing
float breathing_brightness = 0.0f;
float breathing_brightness_increment = 0.01f;
uint32_t breathing_period = 500;
int breathing_period_array_index = 0;
const int breathing_period_array[3] = {500, 1500, 3000};
uint32_t breathing_last_breathing_time = 0;

// brightness
int adjustable_brightness = 0;

// rgb_color
LED_Color rgb_led_color = BLACK;

// adc
uint32_t adc_value;
float adc_threshold;
int adc_threshold_index = 0;
float adc_actual_value;
char buffer_3[100];
char buffer_4[100];
const float adc_threshold_array[3] = {0.00, 1.50, 3.00};

// volume
uint32_t volume_value;
float volume_actual_value;
float volume_threshold = 0;
const float volume_threshold_array[3] = {0.00, 1.50, 3.00};
int volume_threshold_index = 0;
float adc_volume_lambda = 1;

char buffer_6[100];
char buffer_7[100];

// temperature
float temperature = 0.0;
float temperature_threshold;
int temperature_threshold_index = 0;
const float temperature_threshold_array[3] = {20.00, 30.00, 60.00};
char buffer_5[100];
char buffer_8[100];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void set_led_color(int number, uint8_t green, uint8_t red, uint8_t blue)
{
  for (int i = 0; i < 8; i++)
  {
    led_strip_data[24 * number + i] = (green & (1 << (7 - i))) ? 60 : 30;
    led_strip_data[24 * number + 8 + i] = (red & (1 << (7 - i))) ? 60 : 30;
    led_strip_data[24 * number + 16 + i] = (blue & (1 << (7 - i))) ? 60 : 30;
  }
}

void set_led_brightness(int number, float brightness)
{

  uint8_t scaled_green = (uint8_t)(brightness * 255);
  uint8_t scaled_red = (uint8_t)(brightness * 255);
  uint8_t scaled_blue = (uint8_t)(brightness * 255);
  set_led_color(number, scaled_green, scaled_red, scaled_blue);
}

void set_led_bright_code(int number, int brightness_code)
{
  float brightness = brightness_code / 3.00;
  set_led_brightness(number, brightness);
}

void split_and_convert(uint8_t *receive_data, int *numbers, uint16_t size)
{
  char str_copy[100];
  strncpy(str_copy, (char *)receive_data, (int)size);
  str_copy[size] = '\0';
  char *token = strtok(str_copy, " ");
  int count = 0;
  while (token != NULL)
  {
    numbers[count] = atoi(token);
    count++;
    token = strtok(NULL, " ");
  }
  while (count < 5)
  {
    numbers[count] = 0;
    count++;
  }
}

void show_progress(int line, int value)
{
  switch (value)
  {
  case 0:
    OLED_PrintString(0, 8 * (line + 1), percent_000, &font8x6, OLED_COLOR_NORMAL);
    break;
  case 1:
    OLED_PrintString(0, 8 * (line + 1), percent_025, &font8x6, OLED_COLOR_NORMAL);
    break;
  case 2:
    OLED_PrintString(0, 8 * (line + 1), percent_050, &font8x6, OLED_COLOR_NORMAL);
    break;
  case 3:
    OLED_PrintString(0, 8 * (line + 1), percent_075, &font8x6, OLED_COLOR_NORMAL);
    break;
  }
}

void mode_init()
{
  OLED_NewFrame();
  set_led_color(1, 0, 0, 0);

  key1_press_trigger_state = 0;
  key1_release_trigger_state = 0;
  key2_trigger_state = 0;
  mode_trigger_state = 0;

  switch (mode_select)
  {
  case FUNCTION_OFF:
    OLED_PrintString(0, 0, "Function Off         ", &font8x6, OLED_COLOR_REVERSED);
    OLED_PrintString(0, 8, "Bluetooth Control    ", &font8x6, OLED_COLOR_NORMAL);

    break;
  case RELEASE_ACTIVATE:
    OLED_PrintString(0, 0, "Release To Activate  ", &font8x6, OLED_COLOR_REVERSED);
    set_led_color(1, 0, 0, 0);
    OLED_PrintString(0, 8, "LED OFF              ", &font8x6, OLED_COLOR_NORMAL);
    break;
  case PRESS_ACTIVATE:
    OLED_PrintString(0, 0, "Press To Activate    ", &font8x6, OLED_COLOR_REVERSED);
    set_led_color(1, 255, 255, 255);
    OLED_PrintString(0, 8, "LED ON               ", &font8x6, OLED_COLOR_NORMAL);
    break;
  case TOUCH_ACTIVATE:
    OLED_PrintString(0, 0, "Touch To Activate    ", &font8x6, OLED_COLOR_REVERSED);
    OLED_PrintString(0, 8, "LED OFF              ", &font8x6, OLED_COLOR_NORMAL);
    touch_state = 0;
    break;
  case PULSE_MODE:
    OLED_PrintString(0, 0, "Pulse Mode           ", &font8x6, OLED_COLOR_REVERSED);
    OLED_PrintString(0, 8, "Period:              ", &font8x6, OLED_COLOR_NORMAL);
    show_progress(1, 1);
    show_progress(2, 1);
    pulse_last_toggle_time_1 = 0;
    pulse_last_toggle_time_2 = 0;

    pulse_led_state_1 = 0;
    pulse_led_state_2 = 0;
    pulse_period_1_array_index = 0;
    pulse_period_2_array_index = 0;
    pulse_period_1 = pulse_period_array[pulse_period_1_array_index];
    pulse_period_2 = pulse_period_array[pulse_period_2_array_index];

    break;
  case BREATHING_MODE:
    OLED_PrintString(0, 0, "Breathing mode       ", &font8x6, OLED_COLOR_REVERSED);
    OLED_PrintString(0, 8, "Period:              ", &font8x6, OLED_COLOR_NORMAL);
    show_progress(1, 1);
    breathing_brightness = 0.0f;
    breathing_brightness_increment = 0.01f;

    breathing_last_breathing_time = 0;
    breathing_period_array_index = 0;
    breathing_period = breathing_period_array[breathing_period_array_index];

    break;
  case ADJUSTABLE_BRIGHTNESS:
    OLED_PrintString(0, 0, "Adjust bright        ", &font8x6, OLED_COLOR_REVERSED);
    OLED_PrintString(0, 8, "Brightness           ", &font8x6, OLED_COLOR_NORMAL);
    show_progress(1, 0);
    adjustable_brightness = 0;

    break;
  case RGB_LED:
    OLED_PrintString(0, 0, "RGB LED              ", &font8x6, OLED_COLOR_REVERSED);
    OLED_PrintString(0, 8, "Color                ", &font8x6, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 16, "LED OFF              ", &font8x6, OLED_COLOR_NORMAL);
    rgb_led_color = BLACK;
    break;
  case POTENTIOMETER:
    OLED_PrintString(0, 0, "POTENTIOMETER        ", &font8x6, OLED_COLOR_REVERSED);

    adc_threshold_index = 0;
    adc_threshold = adc_threshold_array[adc_threshold_index];
    sprintf(buffer_4, "THRESHOLD:%.2f       ", adc_threshold);
    OLED_PrintString(0, 16, buffer_4, &font8x6, OLED_COLOR_NORMAL);
    break;
  case TEMPERATURE:
    OLED_PrintString(0, 0, "TEMPERATURE          ", &font8x6, OLED_COLOR_REVERSED);
    temperature_threshold_index = 0;
    temperature_threshold = temperature_threshold_array[temperature_threshold_index];
    sprintf(buffer_8, "THRESHOLD:%.2f       ", temperature_threshold);
    OLED_PrintString(0, 16, buffer_8, &font8x6, OLED_COLOR_NORMAL);
    break;
  case VOLUME_LED_THRESHOLD:
    OLED_PrintString(0, 0, "VOLUME WITH THRESHOLD", &font8x6, OLED_COLOR_REVERSED);
    volume_threshold_index = 0;
    volume_threshold = volume_threshold_array[volume_threshold_index];
    sprintf(buffer_8, "THRESHOLD:%.2f       ", volume_threshold);
    OLED_PrintString(0, 16, buffer_8, &font8x6, OLED_COLOR_NORMAL);
    break;
  case VOLUME_LED:
    OLED_PrintString(0, 0, "VOLUME LED           ", &font8x6, OLED_COLOR_REVERSED);
    break;
  }
  OLED_ShowFrame();
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart == &huart2)
  {
    split_and_convert(receive_data, numbers, Size);

    switch (numbers[0])
    {
    case 0:
      mode_select = FUNCTION_OFF;
      mode_init();
      set_led_color(1, numbers[1], numbers[2], numbers[3]);
      sprintf(buffer_1, "B:%d G:%d R:%d", numbers[1], numbers[2], numbers[3]);
      OLED_PrintString(0, 16, buffer_1, &font8x6, OLED_COLOR_NORMAL);
      break;
    case 4:
      mode_select = PULSE_MODE;
      mode_init();
      pulse_period_1 = numbers[1];
      pulse_period_2 = numbers[2];
      sprintf(buffer_1, "1:%dms", numbers[1]);
      sprintf(buffer_2, "2:%dms", numbers[2]);
      OLED_PrintString(0, 16, buffer_1, &font8x6, OLED_COLOR_NORMAL);
      OLED_PrintString(0, 24, buffer_2, &font8x6, OLED_COLOR_NORMAL);
      break;
    case 5:
      mode_select = BREATHING_MODE;
      mode_init();
      breathing_period = numbers[1];
      sprintf(buffer_1, "%dms", numbers[1]);
      OLED_PrintString(0, 16, buffer_1, &font8x6, OLED_COLOR_NORMAL);
      break;
    case 6:
      mode_select = ADJUSTABLE_BRIGHTNESS;
      mode_init();
      set_led_brightness(1, numbers[1] / 100.00);
      sprintf(buffer_1, "%.2f %%", numbers[1] / 100.00);
      OLED_PrintString(0, 16, buffer_1, &font8x6, OLED_COLOR_NORMAL);

      break;
    case 8:

      mode_select = POTENTIOMETER;
      mode_init();
      adc_threshold = numbers[1] / 100.00;
      sprintf(buffer_4, "THRESHOLD:%.2f       ", adc_threshold);
      OLED_PrintString(0, 16, buffer_4, &font8x6, OLED_COLOR_NORMAL);
      OLED_ShowFrame();

      break;
    case 9:
      mode_select = TEMPERATURE;
      mode_init();
      temperature_threshold = numbers[1] / 100.00;
      sprintf(buffer_8, "THRESHOLD:%.2f       ", temperature_threshold);
      OLED_PrintString(0, 16, buffer_8, &font8x6, OLED_COLOR_NORMAL);
      OLED_ShowFrame();
      break;
    case 11:
      mode_select = VOLUME_LED_THRESHOLD;
      mode_init();
      volume_threshold = numbers[1] / 100.00;
      sprintf(buffer_7, "THRESHOLD:%.2f       ", volume_threshold);
      OLED_PrintString(0, 16, buffer_7, &font8x6, OLED_COLOR_NORMAL);
      OLED_ShowFrame();
      break;
    }

    sprintf(send_data, "%d %d %d %d %d", numbers[0], numbers[1], numbers[2], numbers[3], numbers[4]);
    OLED_ShowFrame();
    HAL_UART_Transmit_DMA(&huart2, (uint8_t *)send_data, strlen(send_data));

    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, receive_data, sizeof(receive_data));
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
  }
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */

  // init ws2812
  HAL_Delay(100);
  HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t *)led_strip_data, sizeof(led_strip_data));
  HAL_Delay(100);
  for (int i = 0; i < 72; i++)
  {
    led_strip_data[i] = 30;
  }
  set_led_color(1, 0, 0, 255);
  HAL_Delay(200);
  set_led_color(1, 0, 0, 0);

  // oled_init
  OLED_Init();
  OLED_NewFrame();
  // OLED_PrintString(0, 0, "test message    ", &font8x6, OLED_COLOR_REVERSED);
  OLED_ShowFrame();

  AHT20_Init();

  // init usart
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, receive_data, sizeof(receive_data));
  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

  // init adc
  HAL_ADCEx_Calibration_Start(&hadc1);
  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
  adc_value = HAL_ADC_GetValue(&hadc1);
  adc_actual_value = (adc_value / 4095.0) * 3.3;


  HAL_ADCEx_Calibration_Start(&hadc2);
  HAL_ADC_Start(&hadc2);
  HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
  volume_value = HAL_ADC_GetValue(&hadc2);
  volume_actual_value = (volume_value / 4095.0) * 3.3;

  mode_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // function play
    switch (mode_select)
    {
    case PULSE_MODE:
    {
      uint32_t current_time = HAL_GetTick();
      uint8_t green_value = 0;
      uint8_t blue_value = 0;

      if (current_time - pulse_last_toggle_time_1 >= pulse_period_1 / 2)
      {
        pulse_last_toggle_time_1 = current_time;
        pulse_led_state_1 = !pulse_led_state_1;
      }
      green_value = pulse_led_state_1 ? 255 : 0;

      if (current_time - pulse_last_toggle_time_2 >= pulse_period_2 / 2)
      {
        pulse_last_toggle_time_2 = current_time;
        pulse_led_state_2 = !pulse_led_state_2;
      }
      blue_value = pulse_led_state_2 ? 255 : 0;

      set_led_color(1, 0, green_value, blue_value);
      break;
    }
    case BREATHING_MODE:
    {
      uint32_t current_time = HAL_GetTick();
      if (current_time - breathing_last_breathing_time >= (breathing_period / 100))
      {
        breathing_last_breathing_time = current_time;
        breathing_brightness += breathing_brightness_increment;
        if (breathing_brightness >= 0.5f)
        {
          breathing_brightness = 0.5f;
          breathing_brightness_increment = -breathing_brightness_increment;
        }
        else if (breathing_brightness <= 0.0f)
        {
          breathing_brightness = 0.0f;
          breathing_brightness_increment = -breathing_brightness_increment;
        }
        set_led_brightness(1, breathing_brightness);
      }
      break;
    case POTENTIOMETER:
      adc_value = HAL_ADC_GetValue(&hadc1);
      adc_actual_value = (adc_value / 4095.0) * 3.3;
      sprintf(buffer_3, "ADC VALUE:%.2f       ", adc_actual_value);

      OLED_PrintString(0, 8, buffer_3, &font8x6, OLED_COLOR_NORMAL);

      if (adc_actual_value >= adc_threshold)
      {
        OLED_PrintString(0, 24, "LED ON               ", &font8x6, OLED_COLOR_NORMAL);
        set_led_color(1, 255, 255, 255);
      }
      else
      {
        OLED_PrintString(0, 24, "LED OFF              ", &font8x6, OLED_COLOR_NORMAL);
        set_led_color(1, 0, 0, 0);
      }
      OLED_ShowFrame();
      break;
    case TEMPERATURE:
      AHT20_Measure();
      temperature = AHT20_Temperature();
      sprintf(buffer_5, "%.2fC", temperature);
      if (temperature >= temperature_threshold)
      {
        OLED_PrintString(0, 24, "LED ON               ", &font8x6, OLED_COLOR_NORMAL);
        set_led_color(1, 255, 255, 255);
      }
      else
      {
        OLED_PrintString(0, 24, "LED OFF              ", &font8x6, OLED_COLOR_NORMAL);
        set_led_color(1, 0, 0, 0);
      }
      OLED_PrintString(0, 8, buffer_5, &font8x6, OLED_COLOR_NORMAL);
      OLED_ShowFrame();
      break;
    case VOLUME_LED_THRESHOLD:
      volume_value = HAL_ADC_GetValue(&hadc2);
      volume_actual_value = ((volume_value / 4095.0) * 3.3 *12) +20;
      sprintf(buffer_6, "VOLUME VALUE:%.2f       ", volume_actual_value);

      OLED_PrintString(0, 8, buffer_6, &font8x6, OLED_COLOR_NORMAL);

      if (volume_actual_value >= volume_threshold)
      {
        OLED_PrintString(0, 24, "LED ON               ", &font8x6, OLED_COLOR_NORMAL);
        set_led_color(1, 255, 255, 255);
      }
      else
      {
        OLED_PrintString(0, 24, "LED OFF              ", &font8x6, OLED_COLOR_NORMAL);
        set_led_color(1, 0, 0, 0);
      }
      OLED_ShowFrame();
      break;
    case VOLUME_LED:
      volume_value = HAL_ADC_GetValue(&hadc2);
      volume_actual_value = (volume_value / 4095.0) * 3.3 * 12 +20;
      sprintf(buffer_6, "VOLUME VALUE:%.2f       ", volume_actual_value);

      OLED_PrintString(0, 8, buffer_6, &font8x6, OLED_COLOR_NORMAL);
      set_led_brightness(1, volume_actual_value / 3.30);
      OLED_ShowFrame();
      break;
    }
    }

    if (mode_trigger_state == 1)
    {
      mode_select = (mode_select + 1) % MAX_VALUE_MODE;
      mode_init();
    }
    if (key1_press_trigger_state == 1)
    {
      key1_press_trigger_state = 0;
      switch (mode_select)
      {
      case PRESS_ACTIVATE:
        set_led_color(1, 0, 0, 0);
        OLED_PrintString(0, 8, "LED OFF              ", &font8x6, OLED_COLOR_NORMAL);
        break;
      case RELEASE_ACTIVATE:
        set_led_color(1, 255, 255, 255);
        OLED_PrintString(0, 8, "LED ON               ", &font8x6, OLED_COLOR_NORMAL);
        break;

      case TOUCH_ACTIVATE:

        if (touch_state)
        {
          set_led_color(1, 0, 0, 0);
          OLED_PrintString(0, 8, "LED OFF              ", &font8x6, OLED_COLOR_NORMAL);
        }
        else
        {
          set_led_color(1, 255, 255, 255);
          OLED_PrintString(0, 8, "LED ON               ", &font8x6, OLED_COLOR_NORMAL);
        }
        touch_state = !touch_state;
        break;
      case PULSE_MODE:
        pulse_period_1_array_index++;
        pulse_period_1_array_index = pulse_period_1_array_index % 3;
        pulse_period_1 = pulse_period_array[pulse_period_1_array_index];
        show_progress(1, pulse_period_1_array_index + 1);
      case BREATHING_MODE:
        breathing_period_array_index++;
        breathing_period_array_index = breathing_period_array_index % 3;
        breathing_period = breathing_period_array[breathing_period_array_index];
        show_progress(1, breathing_period_array_index + 1);
        break;
      case RGB_LED:
        rgb_led_color = (rgb_led_color + 1) % MAX_VALUE_COLOR;
        switch (rgb_led_color)
        {
        case 0:
          set_led_color(1, 0, 0, 0);
          OLED_PrintString(0, 16, "LED OFF              ", &font8x6, OLED_COLOR_NORMAL);
          break;
        case 1:
          set_led_color(1, 0, 0, 255);
          OLED_PrintString(0, 16, "BLUE COLOR           ", &font8x6, OLED_COLOR_NORMAL);
          break;
        case 2:
          set_led_color(1, 255, 0, 0);
          OLED_PrintString(0, 16, "GREEN COLOR          ", &font8x6, OLED_COLOR_NORMAL);
          break;
        case 3:
          set_led_color(1, 0, 255, 0);
          OLED_PrintString(0, 16, "RED COLOR            ", &font8x6, OLED_COLOR_NORMAL);
          break;
        case 4:
          set_led_color(1, 255, 255, 255);
          OLED_PrintString(0, 16, "WHITE COLOR          ", &font8x6, OLED_COLOR_NORMAL);
          break;
        }
        break;
      case ADJUSTABLE_BRIGHTNESS:
        adjustable_brightness = (adjustable_brightness + 1) % 4;
        set_led_bright_code(1, adjustable_brightness);
        show_progress(1, adjustable_brightness);
        break;
      case POTENTIOMETER:
        adc_threshold_index = (adc_threshold_index + 1) % 3;
        adc_threshold = adc_threshold_array[adc_threshold_index];
        sprintf(buffer_4, "THRESHOLD:%.2f       ", adc_threshold);
        OLED_PrintString(0, 16, buffer_4, &font8x6, OLED_COLOR_NORMAL);
        break;
      case TEMPERATURE:
        temperature_threshold_index = (temperature_threshold_index + 1) % 3;
        temperature_threshold = temperature_threshold_array[temperature_threshold_index];
        sprintf(buffer_8, "THRESHOLD:%.2f       ", temperature_threshold);
        OLED_PrintString(0, 16, buffer_8, &font8x6, OLED_COLOR_NORMAL);
        break;
      case VOLUME_LED_THRESHOLD:
        volume_threshold_index = (volume_threshold_index + 1) % 3;
        volume_threshold = volume_threshold_array[volume_threshold_index];
        sprintf(buffer_7, "THRESHOLD:%.2f       ", volume_threshold);
        OLED_PrintString(0, 16, buffer_7, &font8x6, OLED_COLOR_NORMAL);
        break;
      }
      OLED_ShowFrame();
    }
    if (key1_release_trigger_state == 1)
    {
      key1_release_trigger_state = 0;
      switch (mode_select)
      {
      case PRESS_ACTIVATE:
        set_led_color(1, 255, 255, 255);
        OLED_PrintString(0, 8, "LED ON               ", &font8x6, OLED_COLOR_NORMAL);
        break;
      case RELEASE_ACTIVATE:
        set_led_color(1, 0, 0, 0);
        OLED_PrintString(0, 8, "LED OFF              ", &font8x6, OLED_COLOR_NORMAL);
        break;
      }
      OLED_ShowFrame();
    }

    if (key2_trigger_state == 1)
    {
      key2_trigger_state = 0;
      switch (mode_select)
      {
      case PULSE_MODE:
        pulse_period_2_array_index++;
        pulse_period_2_array_index = pulse_period_2_array_index % 3;
        pulse_period_2 = pulse_period_array[pulse_period_2_array_index];
        show_progress(2, pulse_period_2_array_index + 1);
        break;
      }

      OLED_ShowFrame();
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
