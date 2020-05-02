/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <math.h>
#include "usbd_cdc_if.h"
#include "Utils/terminal.h"
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
static int adc_start = 0;
static volatile int adc_sampled = 0;
static int adc_index = 0;
static uint16_t adc_samples[160];
static float raw_vin[80];
static float raw_current[80];
static int measure_index = 0;
static float vin_measure[8];
static float current_measure[8];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void adc_sample()
{
  adc_start = 8;
}

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
  MX_USB_DEVICE_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  printf("House Power Monitor\n");
  setbuf(stdout, NULL);
  terminal_init("pwr$ ");

  USART1->CR1 |= USART_CR1_RXNEIE;
  USART1->CR3 |= USART_CR3_EIE;

  HAL_TIM_GenerateEvent(&htim1, TIM_EVENTSOURCE_CC1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t tick = HAL_GetTick() + 2000;
  while (1)
  {
    terminal_run();
    HAL_Delay(100);

    if(tick < HAL_GetTick())
    {
      tick = HAL_GetTick() + 2000;

      if(adc_start == 0)
        adc_start = 8;
    }

    if(adc_sampled)
    {
      float max_vin = -99999;
      float min_vin = 99999;
      float max_current = -99999;
      float min_current = 99999;
      int index = 0;
      for (int k = 0; k < 160; ++k)
      {
        raw_vin[index] = adc_samples[k] * 0.000805860805860806;
        if(raw_vin[index] > max_vin)
          max_vin = raw_vin[index];
        if(raw_vin[index] < min_vin)
          min_vin = raw_vin[index];

        k++;
        raw_current[index] = adc_samples[k] * 0.000805860805860806;
        if(raw_current[index] > max_current)
          max_current = raw_current[index];
        if(raw_current[index] < min_current)
          min_current = raw_current[index];

        index++;
      }

      for (int k = 0; k < 80; ++k)
      {
        float offset_vin = min_vin + ((max_vin - min_vin) / 2.0);
        float offset_current = min_current + ((max_current - min_current) / 2.0);
        raw_vin[k] -= offset_vin;
        raw_current[k] -= offset_current;
        // printf("%0.4f,%0.4f\n", raw_vin[k], raw_current[k]);
      }


      float mean_vin = 0;
      float mean_current = 0;
      for (int k = 0; k < 80; ++k)
      {
        mean_vin += (raw_vin[k] * raw_vin[k]);
        mean_current += (raw_current[k] * raw_current[k]);
      }

      mean_vin /= 80.0;
      mean_current /= 80.0;

      float vin = sqrtf(mean_vin) * 365.524293;
      float current = sqrtf(mean_current) * 25.61865904;
      // printf("V: %5.4f A: %5.4f\n", vin, current);

      vin_measure[measure_index] = vin;
      current_measure[measure_index] = current;
      measure_index++;

      adc_sampled = 0;
      adc_start--;
    }
    else if(adc_start > 0)
    {
      HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);
    }

    if(measure_index >= 8)
    {
      measure_index = 0;

      float vin = 0;
      float current = 0;
      for (int k = 0; k < 8; ++k)
      {
        vin += vin_measure[k];
        current += current_measure[k];
      }
      vin /= 8;
      current /= 8;
      printf(" V: %4.4f A: %4.4f\n", vin, current);
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

  /** Initializes the CPU, AHB and APB busses clocks 
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
  /** Initializes the CPU, AHB and APB busses clocks 
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  // HAL_GPIO_TogglePin(SAMPLE_PULSE_GPIO_Port, SAMPLE_PULSE_Pin);
  if(adc_index >= 160)
  {
    HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1);
    adc_sampled = 1;
    adc_index = 0;
  }

  if(adc_sampled == 0)
  {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    adc_samples[adc_index] = HAL_ADC_GetValue(&hadc1);
    adc_index++;
  }

}

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
 set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
 * @brief Retargets the C library printf function to the USART.
 * @param None
 * @retval None
 */
PUTCHAR_PROTOTYPE
{
  if(ch == '\n')
  {
    uint8_t cr = '\r';
    HAL_UART_Transmit(&huart1, &cr, 1, 0xFFFF);
  }

  if(ch == '\r')
  {
    uint8_t cr = '\n';
    HAL_UART_Transmit(&huart1, &cr, 1, 0xFFFF);
  }


 HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

return ch;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
