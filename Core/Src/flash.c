#include "flash.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;

uint8_t Flash_ReadStatus(uint8_t StatusRegister)
{
	uint8_t spiTransmitBuffer;
	uint8_t spiReceiveBuffer;
	
	if(StatusRegister == 1){
		spiTransmitBuffer = 0x05;
	} else if (StatusRegister == 2){
		spiTransmitBuffer = 0x35;
	} else if (StatusRegister == 3){
		spiTransmitBuffer = 0x15;
	}		
	
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET); //CS low
	HAL_SPI_TransmitReceive(&hspi1, &spiTransmitBuffer, &spiReceiveBuffer, 2, 100); 
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET); //CS high
	
	return spiReceiveBuffer;
}

uint16_t Flash_ReadManufacturerDevID(void)
{
	uint8_t spiTransmitBuffer[4];
	uint8_t spiReceiveBuffer[3];
	uint8_t mid;
	uint8_t id;
	
	spiTransmitBuffer[0] = 0x90;
	spiTransmitBuffer[1] = 0x00;
	spiTransmitBuffer[2] = 0x00;
	spiTransmitBuffer[3] = 0x00;
	
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET); //CS low
	HAL_SPI_TransmitReceive(&hspi1, spiTransmitBuffer, spiReceiveBuffer, 7, 1); 
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET); //CS high
	
	if(spiReceiveBuffer[2] == 0x00){
		mid = spiReceiveBuffer[0];
		id = spiReceiveBuffer[2];
	} else {
		return 0;
	}
	
	return (mid << 8) & id;
}

uint8_t Flash_WriteEnable(void)
{
	uint8_t spiTransmitBuffer;
	
	spiTransmitBuffer = 0x06;
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET); //CS low
	HAL_SPI_Transmit(&hspi1, &spiTransmitBuffer, 1, 1);
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET); //CS high
	
	return 1;
}

uint8_t Flash_WriteDisable(void)
{
	uint8_t spiTransmitBuffer;
	
	spiTransmitBuffer = 0x04;
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET); //CS low
	HAL_SPI_Transmit(&hspi1, &spiTransmitBuffer, 1, 1);
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET); //CS high
	
	return 1;
}

uint8_t Flash_WriteInProgess(void)
{
	uint8_t spiTransmitBuffer;
	uint8_t spiReceiveBuffer = 0;
	
	//Instr.Code
	spiTransmitBuffer = 0x05;	
	
	//Check Write-in-Progress Bit
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET); //CS low
	HAL_SPI_Transmit(&hspi1, &spiTransmitBuffer, 1, 1);
	HAL_SPI_Receive(&hspi1, &spiReceiveBuffer, 1, 1);
	while((spiReceiveBuffer & 0x01) == 0x01){
		HAL_SPI_Receive(&hspi1, &spiReceiveBuffer, 1, 1);	
	}	
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET); //CS high
	
	return 0;
}

uint8_t Flash_4KBSectorErase(uint32_t Address)
{
	uint8_t spiTransmitBuffer[4];
	
	//Instr.-Code + Address
	spiTransmitBuffer[0] = 0x20;
	spiTransmitBuffer[1] = (Address&0x00ff0000)>>16;
	spiTransmitBuffer[2] = (Address&0x0000ff00)>>8;	
	spiTransmitBuffer[3] = (Address&0x000000ff);

	Flash_WriteEnable();
	
	//do it
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, spiTransmitBuffer, 4, 10);	
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
	
	Flash_WriteDisable();
	
	//Check Write-in-Progress Bit
	Flash_WriteInProgess();
				
	return 1;
}

uint8_t Flash_64KBBlockErase(uint32_t Address)
{
	uint8_t spiTransmitBuffer[4];
	
	//Instr.-Code + Address
	spiTransmitBuffer[0] = 0xD8;
	spiTransmitBuffer[1] = (Address&0x00ff0000)>>16;
	spiTransmitBuffer[2] = (Address&0x0000ff00)>>8;	
	spiTransmitBuffer[3] = (Address&0x000000ff);

	Flash_WriteEnable();
	
	//do it
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, spiTransmitBuffer, 4, 10);	
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
	
	Flash_WriteDisable();
	
	//Check Write-in-Progress Bit
	Flash_WriteInProgess();
				
	return 1;	
}

uint8_t Flash_ReadData(uint32_t Address, uint16_t DataSize, uint8_t * Data)
{
	uint8_t spiTransmitBuffer[4];
	
	//HAL_GPIO_WritePin(H6_GPIO_Port, H6_Pin, GPIO_PIN_SET);

	spiTransmitBuffer[0] = 0x03;
	spiTransmitBuffer[1] = ((Address&0x00ff0000)>>16);
	spiTransmitBuffer[2] = ((Address&0x0000ff00)>>8);
	spiTransmitBuffer[3] = (Address&0x000000ff);
	
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET); //CS low
	HAL_SPI_Transmit(&hspi1, spiTransmitBuffer, 4, 100);
	HAL_SPI_Receive(&hspi1, Data, DataSize, 100);
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);	//CS high
	
	//HAL_GPIO_WritePin(H6_GPIO_Port, H6_Pin, GPIO_PIN_RESET);
	
	return 1;
}

uint8_t Flash_ReadData_DMA(uint32_t Address, uint16_t DataSize, uint8_t * Data)
{
	uint8_t spiTransmitBuffer[4];
	
	//HAL_GPIO_WritePin(H6_GPIO_Port, H6_Pin, GPIO_PIN_SET);

	spiTransmitBuffer[0] = 0x03;
	spiTransmitBuffer[1] = ((Address&0x00ff0000)>>16);
	spiTransmitBuffer[2] = ((Address&0x0000ff00)>>8);
	spiTransmitBuffer[3] = (Address&0x000000ff);
	
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET); //CS low
	HAL_SPI_Transmit(&hspi1, spiTransmitBuffer, 4, 1);
	while (HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY)
	{;}
	HAL_SPI_Receive_DMA(&hspi1, Data, DataSize);
	//CS high --> HAL_SPI_RxCpltCallback 	
	
	return 1;
}

uint8_t Flash_WriteData(uint32_t Address, uint32_t DataSize, uint8_t *Data)
{	
	//HAL_GPIO_WritePin(H5_GPIO_Port, H5_Pin, GPIO_PIN_SET);
	
	if(DataSize > 256){
		return 0;
	}
	
	uint8_t spiTransmitBuffer[4];
	
	spiTransmitBuffer[0] = 0x02;
	spiTransmitBuffer[1] = ((Address&0x00ff0000)>>16);
	spiTransmitBuffer[2] = ((Address&0x0000ff00)>>8);
	spiTransmitBuffer[3] = ((Address&0x000000ff)>>0);
	
	/*for (uint16_t i = 0; i<DataSize; i++){
		spiTransmitBuffer[4+i] = *Data;
		Data++;
	}*/
	
	Flash_WriteEnable();
	
	//Page Programm
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET); //CS
	HAL_SPI_Transmit(&hspi1, spiTransmitBuffer, 4, 10);
	HAL_SPI_Transmit(&hspi1, Data, DataSize, 10);
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET); //CS
	
	Flash_WriteDisable();

	//Check Write-in-Progress Bit
	Flash_WriteInProgess();
	
	//HAL_GPIO_WritePin(H5_GPIO_Port, H5_Pin, GPIO_PIN_RESET);

	return 1;
}

uint8_t Flash_WriteData_DMA(uint32_t Address, uint32_t DataSize, uint8_t *Data)
{
	//HAL_GPIO_WritePin(H5_GPIO_Port, H5_Pin, GPIO_PIN_SET);
	
	if(DataSize > 256){
		return 0;
	}
	
	uint8_t spiTransmitBuffer[4];
	
	spiTransmitBuffer[0] = 0x02;
	spiTransmitBuffer[1] = ((Address&0x00ff0000)>>16);
	spiTransmitBuffer[2] = ((Address&0x0000ff00)>>8);
	spiTransmitBuffer[3] = ((Address&0x000000ff)>>0);
	
	Flash_WriteEnable();
	
	//Page Programm
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET); //CS low
	HAL_SPI_Transmit(&hspi1, spiTransmitBuffer, 4, 1);
	while (HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY)
	{;}
	HAL_SPI_Transmit_DMA(&hspi1, Data, DataSize);
	//CS high --> HAL_SPI_TxCpltCallback
	
	Flash_WriteDisable();

	return 1;
}

void FlashTest(void)
{
		//Flash-Test
	uint8_t TestWriteBuffer[256];
	uint8_t TestReadBuffer[256];
	uint8_t TestReadBufferDMA[256];
	uint32_t BufferIndex;
	uint8_t FlashMID;
	
	for(BufferIndex = 0; BufferIndex < 256; BufferIndex++){
		TestWriteBuffer[BufferIndex] = BufferIndex;
		TestReadBuffer[BufferIndex] = 0x00;
	}

	FlashMID = Flash_ReadManufacturerDevID();	
	FlashMID++; //use it
	Flash_4KBSectorErase(0x00000000);
	Flash_ReadData(0x00000000, 256, TestReadBuffer);
	Flash_WriteData(0x00000000, 256, TestWriteBuffer);
	Flash_ReadData(0x00000000, 256, TestReadBuffer);
	Flash_ReadData_DMA(0x00000000, 256, TestReadBufferDMA);
	Flash_4KBSectorErase(0x00000000);
	Flash_ReadData_DMA(0x00000000, 256, TestReadBufferDMA);	
	HAL_Delay(1000);
	Flash_WriteData_DMA(0x00000000, 256, TestWriteBuffer);
	HAL_Delay(1000);
	Flash_ReadData_DMA(0x00000000, 256, TestReadBufferDMA);
	HAL_Delay(1000);
	
	while(1)
	{;}
}
