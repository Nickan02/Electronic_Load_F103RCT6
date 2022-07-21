/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
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
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "key.h"
#include "GUI.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Mode1(void);
void Mode2(void);
void Mode3(void);
void Tim3_Deal(void);
uint32_t Get_Adc_Val(uint16_t *adcxbuf,uint16_t adcx);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint16_t adcxbuf[5];
extern uint16_t	Count;
float Get_temp_f;
uint16_t	Get_temp_u16;
double  temp=0,	//ADC缓存
        CC_Value=0,
        CR_Vol=0,
        CR_Value=0,//DAC设置值
				temp2=0;

double  CW_Vol=0,
        CW_Value=0,
        CW_P=0.3,
        CW_I=0,	 //DAC设置值
				tempI;

uint16_t  Value_Voltage=0;	//输出电压值
double  Voltage=0;	//采集电压值
uint16_t	dac_value1=0;	//dac模式1输入值
uint16_t	dac_value2=1000;	//dac模式2输入值
uint16_t	dac_value3=2000;	//dac模式2输入值
double Converted_current;		//实际采集电流值
double	Converted_current_Value;//实际输出电流值

double	Value_current=0.1;				//目标电流值
double  CR_I;
	char Buffer[64];
uint8_t	key=0;
uint8_t	Mode=1;
uint32_t Value_Res=50;
uint16_t inc=0;
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
   HAL_TIM_Base_Start_IT(&htim3);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
	MX_DMA_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_TIM3_Init();
  
  /* USER CODE BEGIN 2 */
   
		HAL_ADC_ConvCpltCallback(&hadc1);
    HAL_ADCEx_Calibration_Start (&hadc1);
    HAL_ADC_Start_DMA (&hadc1,(uint32_t *)adcxbuf,4);
	  Lcd_Init();    
    delay_init(72);	     //延时函数初始化
    Lcd_Clear(GRAY2);
		KEY_Init(); 				//按键初始化
		 HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,dac_value1);//初始值为0
		HAL_DAC_Start(&hdac,DAC_CHANNEL_1);//开启DAC通道
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
				Tim3_Deal();
        Gui_DrawFont_GBK16(0,0,BLUE,GRAY0,  "S12:+  	   S9:-");
        Gui_DrawFont_GBK16(0,16,BLUE,GRAY0, "S11:MOD+S10:MOD-");
				if(Count%2==0)
					key=KEY_Scan(0);
        if(key == KEY1_PRES)
        {
					Lcd_Clear(GRAY2);
            Mode++;
            if (Mode > 3)	Mode = 3;
        }
        else if (key == KEY3_PRES)
        {
					Lcd_Clear(GRAY2);
            Mode--;
            if (Mode < 1)	Mode = 1;
        }
        switch (Mode) {
        case 1:
            Mode1();
        case 2:
            Mode2();
        case 3:
            Mode3();
       
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
uint32_t Get_Adc_Val(uint16_t *adcxbuf,uint16_t adcx)
{
    uint32_t ADCValue = 0;
	uint16_t i;

    for( i=0; i<15; i++)
    {
        ADCValue += adcxbuf[adcx];
    }
    ADCValue = ADCValue/15;
    return ADCValue;
}
void Mode1(void)
{
    if(Mode==1&&Count%3==0) 	//WKUP/KEY1按下了,或者定时时间到了
    {
		
        if(key==KEY2_PRES)
        {
            if(Value_current<1.01)Value_current+=0.01;
        }
        else if(key==KEY4_PRES)
        {
            if(Value_current>0.00)Value_current-=0.01;
            else dac_value1=0;
        }

        // Gui_DrawFont_GBK16(0,48,BLUE,GRAY0, "DAC NOW:0000");

        
        /*-------显示DAC的CC模式输出电压------*/						//显示"DAC SET"
        sprintf(Buffer,"DAC SET:%5.03fV",CC_Value);
        Gui_DrawFont_GBK16(0,32,BLUE,GRAY0,  Buffer);
        /*-----显示DAC的CC模式当前输入值-----*/							//显示"DAC NOW"
        sprintf(Buffer,"DAC NOW:%4d",dac_value1);
        Gui_DrawFont_GBK16(0,48,BLUE,GRAY0, Buffer);

        sprintf(Buffer,"ADC IN :%5.03fV",Voltage);
        Gui_DrawFont_GBK16(0,64,BLUE,GRAY0, Buffer);
        /*--------显示CC模式目标电压--------*/							//显示"CC_TAG"
        sprintf(Buffer,"CC_TAG :%5.02fA",Value_current);
        Gui_DrawFont_GBK16(0,80,BLUE,GRAY0, Buffer);
    }
}
void Mode2(void)
{
    if(Mode==2&&Count%3==0) 	//WKUP/KEY1按下了,或者定时时间到了
    {
				
			if(key==KEY2_PRES)
        {
              if(Value_Res<15000)Value_Res+=10;
        }
        else if(key==KEY4_PRES)
        {
             if(Value_Res>15)Value_Res-=10.0;
                else Value_Res=10;
        }
			 sprintf(Buffer,"CR_R :%06d",Value_Res);
				Gui_DrawFont_GBK16(0,80,BLUE,GRAY0, Buffer,0);//显示"CR_R"
        /*--------显示CR转化电流值--------*/							//显示"CR_I"  
		
			 sprintf(Buffer,"CR_I :%5.03f",CR_I);
				Gui_DrawFont_GBK16(0,96,BLUE,GRAY0, Buffer);
			
        /*------显示ADC采集电压值-----------*/						//"显示ADC IN"
				sprintf(Buffer,"ADC IN :%5.03fV",Voltage);
        Gui_DrawFont_GBK16(0,64,BLUE,GRAY0, Buffer);
        
				/*-------显示DAC的CR模式输出电压------*/						//显示"DAC SET"
       
				sprintf(Buffer,"DAC SET:%5.02fV",CR_Value);
        Gui_DrawFont_GBK16(0,32,BLUE,GRAY0,Buffer);
        /*-----显示DAC的CR模式当前输入值-----*/							//显示"DAC NOW"    
				sprintf(Buffer,"DAC NOW:%4d",dac_value2);
        Gui_DrawFont_GBK16(0,48,BLUE,GRAY0, Buffer);
    }
}
void Mode3(void)
{
    if(Mode==3&&Count%3==0) 	//WKUP/KEY1按下了,或者定时时间到了
    {
			
			if(key==KEY2_PRES)
        {
               if(CW_P<30.0)CW_P+=0.2;
        }
        else if(key==KEY4_PRES)
        {
           if(CW_P>0.2)CW_P-=0.2;
                else CW_P= 0.10;
        }
			  /*--------显示CW转化电流值--------*/							//显示"CW_P"
		
        Get_temp_f = CW_P;
				sprintf(Buffer,"CW_P :%5.02fW",CW_P);
				Gui_DrawFont_GBK16(0,80,BLUE,GRAY0, Buffer);
			
				   /*--------显示CW转化电流值--------*/							//显示"CW_I"
        Get_temp_f = CW_I;
				sprintf(Buffer,"CW_I :%5.03fA",CW_I);
				Gui_DrawFont_GBK16(0,96,BLUE,GRAY0, Buffer);
        
        /*------显示ADC采集电压值-----------*/						//"显示ADC IN"
        Get_temp_f = Voltage;
        sprintf(Buffer,"ADC IN :%5.03fV",Voltage);
        Gui_DrawFont_GBK16(0,64,BLUE,GRAY0, Buffer);
			
				/*-------显示DAC的CW模式输出电压------*/						//显示"DAC SET"
      
        sprintf(Buffer,"DAC SET:%5.03fV",CW_Value);
        Gui_DrawFont_GBK16(0,32,BLUE,GRAY0,Buffer);

        /*-----显示DAC的CW模式当前输入值-----*/							//显示"DAC NOW"
       
				sprintf(Buffer,"DAC NOW:%4d",dac_value3);
        Gui_DrawFont_GBK16(0,48,BLUE,GRAY0, Buffer);
		}
	}


void Tim3_Deal(void)
{
	
    if(Count % 10 == 0)
    {
			//	dac_value2 = 2190.4*CR_I+135.0;
        /*------------采集电压值-----------*/
        temp=Get_Adc_Val(adcxbuf,1);												//得到ADC采集值			
        Voltage=(float)temp*(3.3/4096)*11.196;								//得到ADC转换值
			
			if(Voltage>30)
			{
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
			}
			
        /*------------------------------------*/

        if(Mode == 1)
        {
            /*----------CC模式恒流步增----------*/
            dac_value1 = 2029.1*Value_current+75.8;
            HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,dac_value1);//设置DAC值
            /*----------------------------------*/

            /*-----------CC输出电压值-----------*/
            Value_Voltage	= HAL_DAC_GetValue(&hdac,DAC_CHANNEL_1);		//读取前面设置DAC的值
            CC_Value=(float)Value_Voltage*(3.3/4096);			         		//得到DAC电压值
//					Converted_current_Value = CC_Value * 1.02f;								//得到硬件转换电流值
            /*------------------------------------*/
        }
					else if(Mode == 2)
				{
					   /*----------CR模式----------*/
						HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,dac_value2);//设置DAC值
            /*----------------------------------*/
					
					  /*-----------CR输出电压值-----------*/
						CR_Vol	= HAL_DAC_GetValue(&hdac,DAC_CHANNEL_1);		//读取前面设置DAC的值
						CR_Value=(float)CR_Vol*(3.3/4096);			         		//得到DAC电压值
//					Converted_current_Value = CC_Value * 1.02f;					//得到硬件转换电流值
						/*------------------------------------*/
					
					  /*-----------CR模式电流---------------*/       
						CR_I = (float)Voltage / Value_Res;
						dac_value2 = 2029.1*CR_I+75.8;

						/*------------------------------------*/
				}
				else if(Mode == 3)
				{
					   /*----------CW模式----------*/
						HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,dac_value3);//设置DAC值
            /*----------------------------------*/
					
					  /*-----------CW输出电压值-----------*/
						CW_Vol	= HAL_DAC_GetValue(&hdac,DAC_CHANNEL_1);		//读取前面设置DAC的值
						CW_Value=(float)CW_Vol*(3.3/4096);			         		//得到DAC电压值
						
						/*------------------------------------*/
					
					  /*-----------CW模式电流---------------*/       
						CW_I = CW_P / Voltage; 
						dac_value3 = 2029.1*CW_I+75.8;
						
						/*------------------------------------*/
				}
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
