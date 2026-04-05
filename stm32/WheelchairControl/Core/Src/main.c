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
#include "dac.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>  // 用于 sprintf
#include <string.h> // 用于 strlen
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

// 1. ADC 数据存储 (DMA 自动搬运)
volatile uint16_t adc_values[2];

// 2. 调试打印缓冲区
char uart_buf[100];

// 3. 树莓派命令接收相关
uint8_t rx_buffer[1];
volatile char pi_command = 'S'; // 默认停止

// 4. 速度控制相关
uint8_t current_gear = 3;       // 当前档位 (默认3档)
float speed_ratio = 1.0f;       // 速度系数

// 5. 核心阈值定义 
#define JOY_CENTER  3102        // 中点电压对应值
#define DEADZONE    200         // 死区

// 6. 偏离值定义 (解决前后速度不对称问题)
// 我们保留约 0.6V 的安全余量
// 0.6V 对应的 ADC 值大约是 745
// 所以 DEV_DOWN = 3102 - 745 = 2357
#define DEV_DOWN    2300 

// 3.3V 对应的 ADC 值是 4095
// 我们限制在 3.0V 左右 (安全余量)
// 3.0V 对应 ADC 3720
// DEV_UP = 3720 - 3102 = 618
#define DEV_UP      600

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

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
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  // 启动 ADC1 并开启 DMA 模式，数据存入 adc_values 数组
HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_values, 2);
  // // 启动 DAC 通道 1 (PA4)
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
  // // 启动 DAC 通道 2 (PA5)
  HAL_DAC_Start(&hdac, DAC_CHANNEL_2);

  HAL_UART_Receive_IT(&huart2, rx_buffer, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /*
    * 此时，DMA 正在后台自动、连续地：
    * 1. 转换 PA0 (X轴) -> 存入 adc_values[0]
    * 2. 转换 PA1 (Y轴) -> 存入 adc_values[1]
    * 3. 循环往复...
    */

    // 1. 获取当前摇杆的物理位置
    uint16_t x_raw = adc_values[0];
    uint16_t y_raw = adc_values[1];

    // 2. 准备 DAC 输出变量
    uint16_t dac_x = JOY_CENTER;
    uint16_t dac_y = JOY_CENTER;

    // 3. 安全仲裁逻辑
    if ((x_raw > JOY_CENTER + DEADZONE || x_raw < JOY_CENTER - DEADZONE) ||
        (y_raw > JOY_CENTER + DEADZONE || y_raw < JOY_CENTER - DEADZONE))
    {
        // 【状态 A：人工接管】
        dac_x = x_raw;
        dac_y = y_raw;
    }
    else
    {
        // 【状态 B：语音控制】

        uint16_t val_fast_side = (uint16_t)(DEV_DOWN * speed_ratio); // 前进/左转用这个

        switch (pi_command)
        {
            case 'F': // 前进 (0V方向，原本很快，需要运算限速)
                dac_x = JOY_CENTER; 
                dac_y = JOY_CENTER - val_fast_side; 
                break;

            case 'B': // 后退 (3.3V方向，原本就很慢，不要运算，全速输出)
                dac_x = JOY_CENTER; 
                dac_y = JOY_CENTER + DEV_UP; // 直接加最大偏离值(990)，尽力输出 3.3V
                break;

            case 'L': // 左转 (0V方向，同前进，需要运算)
                dac_x = JOY_CENTER - val_fast_side; 
                dac_y = JOY_CENTER; 
                break;

            case 'R': // 右转 (3.3V方向，同后退，不要运算)
                dac_x = JOY_CENTER + DEV_UP; // 直接输出 3.3V
                dac_y = JOY_CENTER; 
                break;

            case 'S': // 停止
            default:
                dac_x = JOY_CENTER; 
                dac_y = JOY_CENTER; 
                break;
        }
    }

    // 执行 DAC 输出
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_x);
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, dac_y);

    // --- 修改调试打印 ---
    // 打印当前的 "档位 (Gear)" 和 "指令 (CMD)"
    if (HAL_UART_GetState(&huart1) == HAL_UART_STATE_READY)
    {
      int len = sprintf(uart_buf, "Gear:%u (%d%%) | CMD:%c | X:%u | Y:%u\r\n", 
        current_gear, (int)(speed_ratio * 100), pi_command, dac_x, dac_y);
        HAL_UART_Transmit(&huart1, (uint8_t*)uart_buf, len, 10);
    }

    HAL_Delay(50);

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

// UART 接收完成回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        char cmd = rx_buffer[0];

        // --- 【加减速逻辑】 ---
        if (cmd == 'D') // Decelerate (减速/降档)
        {
            if (current_gear > 1) {
                current_gear--; // 降一档
            }
            // 如果已经是 1档，就不动了
        }
        else if (cmd == 'A') // Accelerate (加速/升档)
        {
            if (current_gear < 3) {
                current_gear++; // 升一档
            }
            // 如果已经是 3档，就不动了
        }
        
        // --- 【运动指令逻辑】 ---
        // 只有 F, B, L, R, S 才会改变运动状态
        else if (cmd == 'F' || cmd == 'B' || cmd == 'L' || cmd == 'R' || cmd == 'S') 
        {
            pi_command = cmd;
        }

        // --- 【统一更新速度系数】 ---
        // 根据当前的档位，刷新 speed_ratio
        switch (current_gear)
        {
            case 1: speed_ratio = 0.3f; break; // 1档: 30%
            case 2: speed_ratio = 0.6f; break; // 2档: 60%
            case 3: speed_ratio = 1.0f; break; // 3档: 100%
        }

        // 重新开启中断
        HAL_UART_Receive_IT(&huart2, rx_buffer, 1);
    }
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
