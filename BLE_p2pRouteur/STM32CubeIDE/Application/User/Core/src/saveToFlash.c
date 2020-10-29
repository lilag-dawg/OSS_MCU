/* Includes ------------------------------------------------------------------*/
#include <saveToFlash.h>
/* Private typedef -----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/


void saveToFlash(uint8_t *data, uint32_t size)
{
	volatile uint32_t data_length = size/FLASH_GRANULAR_SIZE + ((size % FLASH_GRANULAR_SIZE) != 0);
	volatile uint16_t pages = (size/page_size) + ((size%page_size) != 0);
	uint64_t data_to_FLASH[data_length];
	memset((uint8_t*)data_to_FLASH, 0, size);

	for(int i = 0; i< data_length; i++)
	{ //sending data from lsb to msb (lsb stored first in flash)
		data_to_FLASH[i] = 	(uint64_t)(*(data+i*8))+
							(uint64_t)(*(data+i*8+1) << 8)+
							(uint64_t)(*(data+i*8+2) << 16)+
							(uint64_t)(*(data+i*8+3) << 24)+
							(uint64_t)((uint64_t)*(data+i*8+4) << 32)+
							(uint64_t)((uint64_t)*(data+i*8+5) << 40)+
							(uint64_t)((uint64_t)*(data+i*8+6) << 48)+
							(uint64_t)((uint64_t)*(data+i*8+7) << 56);
	}

	  // Unlock the Flash to enable the flash control register access
	  HAL_FLASH_Unlock();

	  // Allow Access to option bytes sector
	  HAL_FLASH_OB_Unlock();

	  // Fill EraseInit structure
	  FLASH_EraseInitTypeDef EraseInitStruct;
	  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	  EraseInitStruct.Page = (FLASH_STORAGE-FLASH_START)/page_size;
	  EraseInitStruct.NbPages = pages;
	  uint32_t PageError;

	  volatile uint32_t index=0;

	  volatile HAL_StatusTypeDef status;

	  //Workaround: Reset FLASH_SR_OPTVERR bit by software at each startup
	  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

	  status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
	  while(index < data_length)
	  {
		  if (status == HAL_OK)
		  {
			  status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, FLASH_STORAGE+(index*FLASH_GRANULAR_SIZE), data_to_FLASH[index]);
			  if(status == HAL_OK)
			  {
				  index++;
			  }
		  }
	  }

	  HAL_FLASH_OB_Lock();
	  HAL_FLASH_Lock();
}

void readFlash(uint8_t* data)
{
	volatile uint64_t read_data;
	volatile uint32_t read_cnt=0;
	do
	{
		read_data = *(uint64_t*)(FLASH_STORAGE + read_cnt);
		if(read_data != 0xFFFFFFFFFFFFFFFF)
		{
			data[read_cnt] = (uint8_t)read_data;
			data[read_cnt + 1] = (uint8_t)(read_data >> 8);
			data[read_cnt + 2] = (uint8_t)(read_data >> 16);
			data[read_cnt + 3] = (uint8_t)(read_data >> 24);
			data[read_cnt + 4] = (uint8_t)(read_data >> 32);
			data[read_cnt + 5] = (uint8_t)(read_data >> 40);
			data[read_cnt + 6] = (uint8_t)(read_data >> 48);
			data[read_cnt + 7] = (uint8_t)(read_data >> 56);
			read_cnt += 8;
		}
	}while(read_data != 0xFFFFFFFFFFFFFFFF);
}
