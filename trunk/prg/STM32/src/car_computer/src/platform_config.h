/**
  ******************************************************************************
  * @file    USART/DMA_Interrupt/platform_config.h 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Evaluation board specific configuration file.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H


/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line corresponding to the STMicroelectronics evaluation board
   used to run the example */
#if !defined (USE_STM32100B_EVAL) && !defined (USE_STM3210B_EVAL) &&  !defined (USE_STM3210E_EVAL) &&  !defined (USE_STM32100E_EVAL) && !defined(STM32F10X_HD)
 //#define USE_STM32100B_EVAL
 //#define USE_STM3210B_EVAL
 //#define USE_STM3210E_EVAL
 //#define USE_STM32100E_EVAL
 #define USE_STM3210C_EVAL 
#endif

/* Define the STM32F10x hardware depending on the used evaluation board */
#if defined(USE_STM3210B_EVAL) || defined (USE_STM32100B_EVAL)

  #define USARTy                   USART1
  #define USARTy_GPIO              GPIOA
  #define USARTy_CLK               RCC_APB2Periph_USART1
  #define USARTy_GPIO_CLK          RCC_APB2Periph_GPIOA
  #define USARTy_RxPin             GPIO_Pin_10
  #define USARTy_TxPin             GPIO_Pin_9
  #define USARTy_Tx_DMA_Channel    DMA1_Channel4
  #define USARTy_Tx_DMA_FLAG       DMA1_FLAG_TC4
  #define USARTy_DR_Base           0x40013804

  #define USARTz                   USART2
  #define USARTz_GPIO              GPIOD
  #define USARTz_CLK               RCC_APB1Periph_USART2
  #define USARTz_GPIO_CLK          RCC_APB2Periph_GPIOD
  #define USARTz_RxPin             GPIO_Pin_6
  #define USARTz_TxPin             GPIO_Pin_5
  #define USARTz_Tx_DMA_Channel    DMA1_Channel7
  #define USARTz_Tx_DMA_FLAG       DMA1_FLAG_TC7
  #define USARTz_DR_Base           0x40004404
  #define USARTz_IRQn              USART2_IRQn

#elif defined (USE_STM3210E_EVAL) || defined (USE_STM32100E_EVAL)

  #define USARTy                   USART1
  #define USARTy_GPIO              GPIOA
  #define USARTy_CLK               RCC_APB2Periph_USART1
  #define USARTy_GPIO_CLK          RCC_APB2Periph_GPIOA
  #define USARTy_RxPin             GPIO_Pin_10
  #define USARTy_TxPin             GPIO_Pin_9
  #define USARTy_Tx_DMA_Channel    DMA1_Channel4
  #define USARTy_Tx_DMA_FLAG       DMA1_FLAG_TC4
  #define USARTy_DR_Base           0x40013804

  #define USARTz                   USART2
  #define USARTz_GPIO              GPIOA
  #define USARTz_CLK               RCC_APB1Periph_USART2
  #define USARTz_GPIO_CLK          RCC_APB2Periph_GPIOA
  #define USARTz_RxPin             GPIO_Pin_3
  #define USARTz_TxPin             GPIO_Pin_2
  #define USARTz_Tx_DMA_Channel    DMA1_Channel7
  #define USARTz_Tx_DMA_FLAG       DMA1_FLAG_TC7
  #define USARTz_DR_Base           0x40004404
  #define USARTz_IRQn              USART2_IRQn

#elif defined USE_STM3210C_EVAL

  #define USARTy                   USART2
  #define USARTy_GPIO              GPIOD
  #define USARTy_CLK               RCC_APB1Periph_USART2
  #define USARTy_GPIO_CLK          RCC_APB2Periph_GPIOD
  #define USARTy_RxPin             GPIO_Pin_6
  #define USARTy_TxPin             GPIO_Pin_5
  #define USARTy_Tx_DMA_Channel    DMA1_Channel7
  #define USARTy_Tx_DMA_FLAG       DMA1_FLAG_TC7
  #define USARTy_DR_Base           0x40004404

  #define USARTz                   USART3
  #define USARTz_GPIO              GPIOC
  #define USARTz_CLK               RCC_APB1Periph_USART3
  #define USARTz_GPIO_CLK          RCC_APB2Periph_GPIOC
  #define USARTz_RxPin             GPIO_Pin_11
  #define USARTz_TxPin             GPIO_Pin_10
  #define USARTz_Tx_DMA_Channel    DMA1_Channel2
  #define USARTz_Tx_DMA_FLAG       DMA1_FLAG_TC2
  #define USARTz_DR_Base           0x40004804
  #define USARTz_IRQn              USART3_IRQn

#elif defined(STM32F10X_HD)


	#define USART1_GPIO              	GPIOA
	#define USART1_CLK               	RCC_APB2Periph_USART1
	#define USART1_GPIO_CLK          	RCC_APB2Periph_GPIOA
	#define USART1_RxPin             	GPIO_Pin_10
	#define USART1_TxPin             	GPIO_Pin_9
	#define USART1_DR_Base           	0x40013804
	#define USART1_Tx_DMA_Channel    	DMA1_Channel4
	#define USART1_Tx_DMA_IRQ_Handler   DMA1_Channel4_IRQHandler
	#define USART1_Tx_DMA_FLAG       	DMA1_FLAG_TC4
	#define USART1_Rx_IRQ_Handler		USART1_IRQHandler

	#define	USART1_RX_BUFFER_SIZE		16

	#define CONSOLE_USART			 1


#endif

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __PLATFORM_CONFIG_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
