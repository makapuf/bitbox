//******************************************************************************
// STM32F4 Discovery SDCard + FatFs Test - CLIVE - SOURCER32@GMAIL.COM
//******************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4_discovery_sdio_sd.h"

#define DBG

//******************************************************************************

void NVIC_Configuration(void);
void RCC_Configuration(void);
void GPIO_Configuration(void);
void USART2_Configuration(void);

//******************************************************************************

#include "ff.h"
#include "diskio.h"

FRESULT res;
FILINFO fno;
FIL fil;
DIR dir;
FATFS fs32;
char* path;

#if _USE_LFN
    static char lfn[_MAX_LFN + 1];
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;
#endif

//******************************************************************************

char *dec32(unsigned long i)
{
  static char str[16];
  char *s = str + sizeof(str);

  *--s = 0;

  do
  {
    *--s = '0' + (char)(i % 10);
    i /= 10;
  }
  while(i);

  return(s);
}

//******************************************************************************

int main(void)
{
  /*!< At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */

  NVIC_Configuration(); /* Interrupt Config */

#ifdef DBG
	RCC_Configuration();

	GPIO_Configuration();

  USART2_Configuration();

	puts("FatFs Testing");
#endif

	memset(&fs32, 0, sizeof(FATFS));

	res = f_mount(0, &fs32);

#ifdef DBG
	if (res != FR_OK)
		printf("res = %d f_mount\n", res);
#endif
	
	memset(&fil, 0, sizeof(FIL));
	
	res = f_open(&fil, "MESSAGE.TXT", FA_READ);

#ifdef DBG
	if (res != FR_OK)
		printf("res = %d f_open MESSAGE.TXT\n", res);
#endif
	
	if (res == FR_OK)
	{
		UINT Total = 0;

		while(1)
		{
			BYTE Buffer[512];
			UINT BytesRead;
			UINT i;

			res = f_read(&fil, Buffer, sizeof(Buffer), &BytesRead);

#ifdef DBG
			if (res != FR_OK)
				printf("res = %d f_read MESSAGE.TXT\n", res);
#endif
			
			if (res != FR_OK)
				break;

			Total += BytesRead;

#ifdef DBGX
			for(i=0; i<BytesRead; i++)
				putchar(Buffer[i]);
#endif
			
			if (BytesRead < sizeof(Buffer))
				break;
		}

		res = f_close(&fil); // MESSAGE.TXT

#ifdef DBG
		if (res != FR_OK)
			printf("res = %d f_close MESSAGE.TXT\n", res);

		printf("Total = %d\n", Total);
#endif

    res = f_open(&fil, "LENGTH.TXT", FA_CREATE_ALWAYS | FA_WRITE);

#ifdef DBG
		if (res != FR_OK)
			printf("res = %d f_open LENGTH.TXT\n", res);
#endif
	
    if (res == FR_OK)
    {
      UINT BytesWritten;
      char crlf[] = "\r\n";
      char *s = dec32(Total);

      res = f_write(&fil, s, strlen(s), &BytesWritten);

      res = f_write(&fil, crlf, strlen(crlf), &BytesWritten);

  		res = f_close(&fil); // LENGTH.TXT

#ifdef DBG			
  		if (res != FR_OK)
	  		printf("res = %d f_close LENGTH.TXT\n", res);
#endif			
    }
	}

  res = f_open(&fil, "DIR.TXT", FA_CREATE_ALWAYS | FA_WRITE);

#ifdef DBG
	if (res != FR_OK)
		printf("res = %d f_open DIR.TXT\n", res);
#endif
	
  if (res == FR_OK)
  {
    UINT BytesWritten;

		path = "";

		res = f_opendir(&dir, path);

#ifdef DBG
		if (res != FR_OK)
			printf("res = %d f_opendir\n", res);
#endif
	
		if (res == FR_OK)
		{
			while(1)
			{
        char str[256];
        char *s = str;
				char *fn;

				res = f_readdir(&dir, &fno);

#ifdef DBG
				if (res != FR_OK)
					printf("res = %d f_readdir\n", res);
#endif
			
				if ((res != FR_OK) || (fno.fname[0] == 0))
					break;

#if _USE_LFN
				fn = *fno.lfname ? fno.lfname : fno.fname;
#else
				fn = fno.fname;
#endif

#ifdef DBG
				printf("%c%c%c%c ",
					((fno.fattrib & AM_DIR) ? 'D' : '-'),
					((fno.fattrib & AM_RDO) ? 'R' : '-'),
					((fno.fattrib & AM_SYS) ? 'S' : '-'),
					((fno.fattrib & AM_HID) ? 'H' : '-') );

				printf("%10d ", fno.fsize);

				printf("%s/%s\n", path, fn);
#endif
				
		  	*s++ = ((fno.fattrib & AM_DIR) ? 'D' : '-');
				*s++ = ((fno.fattrib & AM_RDO) ? 'R' : '-');
  			*s++ = ((fno.fattrib & AM_SYS) ? 'S' : '-');
	  		*s++ = ((fno.fattrib & AM_HID) ? 'H' : '-');

        *s++ = ' ';

        strcpy(s, dec32(fno.fsize));
        s += strlen(s);

        *s++ = ' ';

        strcpy(s, path);
        s += strlen(s);

        *s++ = '/';

        strcpy(s, fn);
        s += strlen(s);

        *s++ = 0x0D;
        *s++ = 0x0A;
        *s++ = 0;

        res = f_write(&fil, str, strlen(str), &BytesWritten);
			}
		}

  	res = f_close(&fil); // DIR.TXT

#ifdef DBG		
 		if (res != FR_OK)
  		printf("res = %d f_close DIR.TXT\n", res);
#endif		
  }

  while(1); /* Infinite loop */
}

//******************************************************************************

void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**************************************************************************************/

void RCC_Configuration(void)
{
  /* --------------------------- System Clocks Configuration -----------------*/
  /* USART2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

  /* GPIOA clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
}

/**************************************************************************************/

void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*-------------------------- GPIO Configuration ----------------------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Connect USART pins to AF */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);  // USART2_TX
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);  // USART2_RX
}

/**************************************************************************************/

void USART2_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;

  /* USARTx configuration ------------------------------------------------------*/
  /* USARTx configured as follow:
        - BaudRate = 115200 baud
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART2, &USART_InitStructure);

  USART_Cmd(USART2, ENABLE);
}

//******************************************************************************
// Hosting of stdio functionality through USART2
//******************************************************************************

#include <rt_misc.h>

#pragma import(__use_no_semihosting_swi)

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
	static int last;

	if ((ch == (int)'\n') && (last != (int)'\r'))
	{
		last = (int)'\r';

  	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);

 	  USART_SendData(USART2, last);
	}
	else
		last = ch;

	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);

  USART_SendData(USART2, ch);

  return(ch);
}

int fgetc(FILE *f)
{
	char ch;

	while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);

	ch = USART_ReceiveData(USART2);

  return((int)ch);
}

int ferror(FILE *f)
{
  /* Your implementation of ferror */
  return EOF;
}

void _ttywrch(int ch)
{
	static int last;

	if ((ch == (int)'\n') && (last != (int)'\r'))
	{
		last = (int)'\r';

  	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);

 	  USART_SendData(USART2, last);
	}
	else
		last = ch;

	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);

  USART_SendData(USART2, ch);
}

void _sys_exit(int return_code)
{
label:  goto label;  /* endless loop */
}

//******************************************************************************

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  while(1); /* Infinite loop */
}
#endif

//******************************************************************************
