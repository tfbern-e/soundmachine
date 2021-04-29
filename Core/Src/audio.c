#include "audio.h"
#include "flash.h"
#include "main.h"
#include "pdm2pcm_glo.h"
#include "audio_fw_glo.h"

//#define DEBUG
//#define DEBUG_SINEPCM

extern DAC_HandleTypeDef hdac1;
extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern TIM_HandleTypeDef htim2;
extern CRC_HandleTypeDef hcrc;
extern SAI_HandleTypeDef hsai_BlockA1;

static uint8_t SineTable[] = {
        0x80, 0x83, 0x86, 0x89, 0x8c, 0x8f, 0x92, 0x95,
        0x98, 0x9b, 0x9e, 0xa2, 0xa5, 0xa7, 0xaa, 0xad,
        0xb0, 0xb3, 0xb6, 0xb9, 0xbc, 0xbe, 0xc1, 0xc4,
        0xc6, 0xc9, 0xcb, 0xce, 0xd0, 0xd3, 0xd5, 0xd7,
        0xda, 0xdc, 0xde, 0xe0, 0xe2, 0xe4, 0xe6, 0xe8,
        0xea, 0xeb, 0xed, 0xee, 0xf0, 0xf1, 0xf3, 0xf4,
        0xf5, 0xf6, 0xf8, 0xf9, 0xfa, 0xfa, 0xfb, 0xfc,
        0xfd, 0xfd, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xfd,
        0xfd, 0xfc, 0xfb, 0xfa, 0xfa, 0xf9, 0xf8, 0xf6,
        0xf5, 0xf4, 0xf3, 0xf1, 0xf0, 0xee, 0xed, 0xeb,
        0xea, 0xe8, 0xe6, 0xe4, 0xe2, 0xe0, 0xde, 0xdc,
        0xda, 0xd7, 0xd5, 0xd3, 0xd0, 0xce, 0xcb, 0xc9,
        0xc6, 0xc4, 0xc1, 0xbe, 0xbc, 0xb9, 0xb6, 0xb3,
        0xb0, 0xad, 0xaa, 0xa7, 0xa5, 0xa2, 0x9e, 0x9b,
        0x98, 0x95, 0x92, 0x8f, 0x8c, 0x89, 0x86, 0x83,
        0x80, 0x7c, 0x79, 0x76, 0x73, 0x70, 0x6d, 0x6a,
        0x67, 0x64, 0x61, 0x5d, 0x5a, 0x58, 0x55, 0x52,
        0x4f, 0x4c, 0x49, 0x46, 0x43, 0x41, 0x3e, 0x3b,
        0x39, 0x36, 0x34, 0x31, 0x2f, 0x2c, 0x2a, 0x28,
        0x25, 0x23, 0x21, 0x1f, 0x1d, 0x1b, 0x19, 0x17,
        0x15, 0x14, 0x12, 0x11, 0x0f, 0x0e, 0x0c, 0x0b,
        0x0a, 0x09, 0x07, 0x06, 0x05, 0x05, 0x04, 0x03,
        0x02, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02,
        0x02, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x09,
        0x0a, 0x0b, 0x0c, 0x0e, 0x0f, 0x11, 0x12, 0x14,
        0x15, 0x17, 0x19, 0x1b, 0x1d, 0x1f, 0x21, 0x23,
        0x25, 0x28, 0x2a, 0x2c, 0x2f, 0x31, 0x34, 0x36,
        0x39, 0x3b, 0x3e, 0x41, 0x43, 0x46, 0x49, 0x4c,
        0x4f, 0x52, 0x55, 0x58, 0x5a, 0x5d, 0x61, 0x64,
        0x67, 0x6a, 0x6d, 0x70, 0x73, 0x76, 0x79, 0x7c,
};

void AudioRecord(uint8_t ChannelToRecord) //Funktion zum Einlesen und Speichern des ADC Wertes
{
	//Button
	uint16_t PushButtonPin;
	GPIO_TypeDef *PushButtonPort;
	
	//Flash
	uint32_t FlashChannelStartAddress;
	uint32_t FlashChannelStopAddress;
	uint32_t FlashChannelAudioStartAddress;
	uint32_t FlashChannelActualAddress;
	extern uint8_t FlashWriteBuffer[];
	extern uint32_t FlashWriteBufferIndex;
	extern uint8_t WriteDoubleBufferIndex;
	
	//mic
	extern uint8_t MicWokeUp;

	//pdm & pcm
	extern uint8_t pdmbuffer[PDMBUFFERSIZE];
	extern uint16_t pcmbuffer[PCMBUFFERSIZE];
	extern uint8_t PCMBufferReady;
	extern uint8_t PCMReadyBufferIndex;
	extern uint8_t PDMReadyBufferIndex;
	extern uint8_t PDMBufferReady;	
	extern uint8_t Timer2AudioTrigger;
	extern PDM_Filter_Handler_t FilterHandler;
	PDM_Filter_Config_t FilterConfig;
	
	//REC-LED on
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	
	/*Enables and resets CRC-32 from STM32 HW */    
	__HAL_RCC_CRC_CLK_ENABLE();    
	CRC->CR = CRC_CR_RESET;
	
  //Initialize PDM Filter structure    
	FilterHandler.bit_order = PDM_FILTER_BIT_ORDER_MSB;    
	FilterHandler.endianness = PDM_FILTER_ENDIANNESS_LE;    
	FilterHandler.high_pass_tap = 2122358088; // Coff = 0.988 -> get this number as 0.988*(2^31-1)
	FilterHandler.out_ptr_channels = 1;    
	FilterHandler.in_ptr_channels  = 1;
	PDM_Filter_Init(&FilterHandler);

	FilterConfig.output_samples_number = PCMBUFFERSIZE;
	FilterConfig.mic_gain = 1;
	FilterConfig.decimation_factor = PDM_FILTER_DEC_FACTOR_64;
	PDM_Filter_setConfig(&FilterHandler, &FilterConfig);

	#ifdef DEBUG
	uint8_t temp[256];
	#endif
	
	//Start Adresse und Button
	if (ChannelToRecord == 1){
		FlashChannelStartAddress = FLASHCH1STARTADDRESS;
		PushButtonPin = GPIO_PIN_1;
		PushButtonPort = GPIOB;
	}		
	if (ChannelToRecord == 2){
		FlashChannelStartAddress = FLASHCH2STARTADDRESS;
		PushButtonPin = GPIO_PIN_2;
		PushButtonPort = GPIOB;
	}
	if (ChannelToRecord == 3){
		FlashChannelStartAddress = FLASHCH3STARTADDRESS;
		PushButtonPin = GPIO_PIN_3;
		PushButtonPort = GPIOB;		
	}
	if (ChannelToRecord == 4){
		FlashChannelStartAddress = FLASHCH4STARTADDRESS;
		PushButtonPin = GPIO_PIN_4;
		PushButtonPort = GPIOB;
	}
	
	//Audio Start- und Stop-Addresse berechnen
	FlashChannelAudioStartAddress = FlashChannelStartAddress + FLASH_HEADER_SIZE;	//Startadresse + Kopf
	FlashChannelStopAddress = FlashChannelStartAddress + FLASH_HEADER_SIZE + FLASH_AUDIO_SIZE - 0x01;
	
	//Aktuelle Adresse auf Startadresse des Audio-Sektoren-Bereich setzen
	FlashChannelActualAddress = FlashChannelAudioStartAddress;
	
	//64KB Block Erase (ca. 4 Sec Audio)
	Flash_64KBBlockErase(FlashChannelStartAddress);
	
	//flash buffer reset
	FlashWriteBufferIndex = 0;
	
	//Starte Mic-Recording
	MicWokeUp = FALSE;
	HAL_SAI_Receive_DMA(&hsai_BlockA1, pdmbuffer, PDMBUFFERSIZE);
	//HAL_SPI_Receive_DMA(&hspi1, pdmbuffer, PDMBUFFERSIZE);
	HAL_Delay(200);
	MicWokeUp = TRUE;
	
	//solange die entsprechende Channel-Taste gedrückt ist oder der Speicherbereich noch nicht voll ist
	while (/*(HAL_GPIO_ReadPin(PushButtonPort, PushButtonPin)) &&*/ (FlashChannelActualAddress < (FlashChannelStopAddress)))
	{	
		if (WriteDoubleBufferIndex == DOUBLEBUFFER_HIGH)
		{
			WriteDoubleBufferIndex = DOUBLEBUFFER_NONE;
			
			//write high buffer to flash
			Flash_WriteData(FlashChannelActualAddress, FLASHPAGESIZE,
				#ifdef DEBUG_SINEPCM 
				SineTable
				#else
				&FlashWriteBuffer[FLASHPAGESIZE]
				#endif
			);
			
			#ifdef DEBUG
			HAL_UART_Transmit(&huart1, FlashPageBuffer, FLASHPAGESIZE, 100);
			Flash_ReadData(FlashChannelActualAddress, FLASHPAGESIZE,  temp);
			#endif
			
			//Addresse + 1 Flash-Page (256B)
			FlashChannelActualAddress = FlashChannelActualAddress + FLASHPAGESIZE;		
		} 
		else if (WriteDoubleBufferIndex == DOUBLEBUFFER_LOW) 
		{
			WriteDoubleBufferIndex = DOUBLEBUFFER_NONE;
			
			//write low buffer to flash
			Flash_WriteData(FlashChannelActualAddress, FLASHPAGESIZE, 
				#ifdef DEBUG_SINEPCM 
				SineTable
				#else
				&FlashWriteBuffer[0]
				#endif
			);

			//Addresse + 1 Flash-Page (256B)
			FlashChannelActualAddress = FlashChannelActualAddress + FLASHPAGESIZE;			
		}		
	}
	
	//Stop SPI
	//HAL_SPI_Abort(&hspi1);
	
	//Stop SAI
	HAL_SAI_Abort(&hsai_BlockA1);


	//Endadresse berechnen
	FlashChannelStopAddress = FlashChannelActualAddress-1;
	
	//Restlicher PageBuffer löschen
	/*for(PageBufferIndex = PageBufferIndex; PageBufferIndex <= 255; PageBufferIndex++){
		PageBuffer[PageBufferIndex] = 0;
	}*/
	
	//Letzte Page schreiben
	//Flash_WriteData(FlashWriteAudioAddress, PageBufferIndex, PageWriteBuffer);
	
	//PageBuffer löschen
	for(FlashWriteBufferIndex = 0; FlashWriteBufferIndex <= 255; FlashWriteBufferIndex++){
		FlashWriteBuffer[FlashWriteBufferIndex] = 0;
	}
	
	//Länge des Audio-Streams in PageBuffer schreiben
	FlashWriteBuffer[0] = ((FlashChannelStopAddress & 0x00ff0000) >> 16);
	FlashWriteBuffer[1] = ((FlashChannelStopAddress & 0x0000ff00) >> 8);
	FlashWriteBuffer[2] = ((FlashChannelStopAddress & 0x000000ff) >> 0);
	
	//Header-Sector löschen
	//Flash_SectorErase(FlashChannelStartAddress);

	//flash Header schreiben
	Flash_WriteData(FlashChannelStartAddress, 3, FlashWriteBuffer);

	#ifdef DEBUG
	Flash_ReadData(FlashChannelStartAddress, 3,  temp);
	#endif
	
	//REC-LED off
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
}

void AudioPlay(uint8_t ChannelToPlay)
{
	uint32_t FlashChannelActualAddress;
	uint32_t FlashChannelAudioStartAddress;
	uint32_t FlashChannelAudioStopAddress;
	uint32_t FlashChannelStartAddress;
	uint32_t FlashChannelStopAddress;
	extern uint8_t FlashReadBuffer[];
	extern uint16_t FlashReadBufferIndex;
	extern uint8_t ReadDoubleBufferIndex;
	
	//Play-LED on
	//HAL_GPIO_WritePin(H2_GPIO_Port, H2_Pin, GPIO_PIN_SET);
	
	//Channel Start Adresse
	if (ChannelToPlay == 1){
		FlashChannelStartAddress = FLASHCH1STARTADDRESS;
	}
	if (ChannelToPlay == 2){
		FlashChannelStartAddress = FLASHCH2STARTADDRESS;
	}
	if (ChannelToPlay == 3){
		FlashChannelStartAddress = FLASHCH3STARTADDRESS;
	}
	if (ChannelToPlay == 4){
		FlashChannelStartAddress = FLASHCH4STARTADDRESS;
	}
	
	//Channel Stop Adress
	FlashChannelStopAddress = FlashChannelStartAddress + FLASH_HEADER_SIZE + FLASH_AUDIO_SIZE - 0x1;
	
	//Audio Start Address
	FlashChannelAudioStartAddress = FlashChannelStartAddress + FLASH_HEADER_SIZE;	//Startadresse + Kopf

	//Audio Stop Address
	Flash_ReadData(FlashChannelStartAddress, 3, FlashReadBuffer);
	FlashChannelAudioStopAddress = ((FlashReadBuffer[0]<<16) | (FlashReadBuffer[1]<<8) | (FlashReadBuffer[2]<<0));

	//Aktuelle Adresse auf Startadresse des Audio-Sektoren-Bereich setzen
	FlashChannelActualAddress = FlashChannelAudioStartAddress;
	
	//fill low & high buffer
	Flash_ReadData_DMA(FlashChannelActualAddress, FLASHPAGESIZE * 2,  &FlashReadBuffer[0]);
	FlashChannelActualAddress = FlashChannelActualAddress + 2 * FLASHPAGESIZE;
	ReadDoubleBufferIndex = DOUBLEBUFFER_NONE;
	FlashReadBufferIndex = 0;
	
	HAL_Delay(10);
	
	//HAL_UART_Transmit(&huart1, FlashReadBuffer, FLASHPAGESIZE*2, 100);
	#ifdef DEBUG
	HAL_UART_Transmit(&huart1, FlashReadBuffer, FLASHPAGESIZE*2, 100);
	#endif

	//start DAC
	HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
	
	//enable AMP
	HAL_GPIO_WritePin(AMP_EN_GPIO_Port, AMP_EN_Pin, GPIO_PIN_RESET);
	
	//start TIM2
	HAL_TIM_Base_Start_IT(&htim2);
	
	while ((FlashChannelActualAddress <= FlashChannelAudioStopAddress) && (FlashChannelActualAddress <= FlashChannelStopAddress)){

		#ifdef DEBUG
		HAL_UART_Transmit(&huart1, ReadBuffer, FLASHPAGESIZE, 100);
		#endif
		
		if (ReadDoubleBufferIndex == DOUBLEBUFFER_HIGH)
		{
			ReadDoubleBufferIndex = DOUBLEBUFFER_NONE;
			
			//fill high buffer and incr. flash address
			Flash_ReadData_DMA(FlashChannelActualAddress, FLASHPAGESIZE,  &FlashReadBuffer[FLASHPAGESIZE]);
			FlashChannelActualAddress = FlashChannelActualAddress + FLASHPAGESIZE;		
		} 
		else if (ReadDoubleBufferIndex == DOUBLEBUFFER_LOW) 
		{
			ReadDoubleBufferIndex = DOUBLEBUFFER_NONE;
			
			//fill low buffer and incr. flash address
			Flash_ReadData_DMA(FlashChannelActualAddress, FLASHPAGESIZE,  &FlashReadBuffer[0]);
			FlashChannelActualAddress = FlashChannelActualAddress + FLASHPAGESIZE;			
		}
	}
	
	//Stoppe TIM2
	HAL_TIM_Base_Stop(&htim2);
	
	//Disable AMP
	HAL_GPIO_WritePin(AMP_EN_GPIO_Port, AMP_EN_Pin, GPIO_PIN_SET);
	
	//Stop DAC
	HAL_DAC_Stop(&hdac1, DAC_CHANNEL_1);
	
	//Play-LED off
	//HAL_GPIO_WritePin(H2_GPIO_Port, H2_Pin, GPIO_PIN_RESET);
}

void SendAudioData(uint8_t ChannelToSend)
{
	uint32_t FlashChannelActualAddress;
	uint32_t FlashChannelAudioStartAddress;
	uint32_t FlashChannelAudioStopAddress;
	uint32_t FlashChannelStartAddress;
	uint32_t FlashChannelStopAddress;
	uint8_t ReadBuffer[256]; 
	
	//Play-LED on
	//HAL_GPIO_WritePin(H2_GPIO_Port, H2_Pin, GPIO_PIN_SET);
	
	//Channel Start Adresse
	if (ChannelToSend == 1){
		FlashChannelStartAddress = FLASHCH1STARTADDRESS;
	}
	if (ChannelToSend == 2){
		FlashChannelStartAddress = FLASHCH2STARTADDRESS;
	}
	if (ChannelToSend == 3){
		FlashChannelStartAddress = FLASHCH3STARTADDRESS;
	}
	if (ChannelToSend == 4){
		FlashChannelStartAddress = FLASHCH4STARTADDRESS;
	}
	
	//Channel Stop Adress
	FlashChannelStopAddress = FlashChannelStartAddress + FLASH_HEADER_SIZE + FLASH_AUDIO_SIZE - 0x1;
	
	//Audio Start Address
	FlashChannelAudioStartAddress = FlashChannelStartAddress + FLASH_HEADER_SIZE;	//Startadresse + Kopf

	//Audio Stop Address
	Flash_ReadData(FlashChannelStartAddress, 3, ReadBuffer);
	FlashChannelAudioStopAddress = ((ReadBuffer[0]<<16) | (ReadBuffer[1]<<8) | (ReadBuffer[2]<<0));

	//Aktuelle Adresse auf Startadresse des Audio-Sektoren-Bereich setzen
	FlashChannelActualAddress = FlashChannelAudioStartAddress;	
	
	while((FlashChannelActualAddress <= FlashChannelAudioStopAddress) && (FlashChannelActualAddress <= FlashChannelStopAddress)){
		Flash_ReadData(FlashChannelActualAddress, FLASHPAGESIZE,  ReadBuffer);
		FlashChannelActualAddress = FlashChannelActualAddress + FLASHPAGESIZE;
		

		//HAL_UART_Transmit(&huart1, ReadBuffer, FLASHPAGESIZE, 100);
	}
	
	//Play-LED off
	//HAL_GPIO_WritePin(H2_GPIO_Port, H2_Pin, GPIO_PIN_RESET);	
}

