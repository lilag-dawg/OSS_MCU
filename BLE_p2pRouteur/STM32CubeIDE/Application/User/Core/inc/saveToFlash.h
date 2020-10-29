#ifndef __SAVETOFLASH_H
#define __SAVETOFLASH_H

#ifdef __cplusplus
extern "C" {
#endif


/****************** Example ***************************************/
/*
  struct settings settingsToWrite;
  memset(&settingsToWrite, 0 , sizeof(settingsToWrite));
  settingsToWrite.preferences.ftp = 111;
  settingsToWrite.preferences.targetEffort = 12;
  settingsToWrite.preferences.shiftingResponsiveness = 14.78;
  settingsToWrite.preferences.desiredRpm = 32;
  settingsToWrite.preferences.desiredBpm = 7;

  settingsToWrite.cranksets.bigGear = 45;
  settingsToWrite.cranksets.gear2 = 35;

  settingsToWrite.sprockets.smallGear = 3;
  settingsToWrite.sprockets.gear2 = 5;
  settingsToWrite.sprockets.gear3 = 6;
  settingsToWrite.sprockets.gear4 = 7;
  settingsToWrite.sprockets.gear5 = 9;
  settingsToWrite.sprockets.gear6 = 11;
  settingsToWrite.sprockets.gear7 = 13;
  settingsToWrite.sprockets.gear8 = 16;
  settingsToWrite.sprockets.gear9 = 20;
  settingsToWrite.sprockets.gear10 = 23;
  settingsToWrite.sprockets.gear11 = 28;

  saveToFlash((uint8_t*) &settingsToWrite, sizeof(settingsToWrite));

  struct settings readSettings;
  readFlash((uint8_t*)&readSettings);
  */


#include "string.h"
#include "main.h"

#define FLASH_START 0X08000000
#define FLASH_STORAGE 0x08030000
#define page_size 0x1000
#define FLASH_GRANULAR_SIZE 0X8



struct preferences {
	uint16_t ftp;// : 10;
	uint16_t targetEffort;// : 10;
	float shiftingResponsiveness;
	uint16_t desiredRpm;// : 8;
	uint16_t desiredBpm;// : 8;
};

struct cranksets {
	uint8_t bigGear;
	uint8_t gear2;
	uint8_t gear3;
};

struct sprockets {
	uint8_t smallGear;
	uint8_t gear2;
	uint8_t gear3;
	uint8_t gear4;
	uint8_t gear5;
	uint8_t gear6;
	uint8_t gear7;
	uint8_t gear8;
	uint8_t gear9;
	uint8_t gear10;
	uint8_t gear11;
	uint8_t gear12;
	uint8_t gear13;
};

struct sensor {
	char name[19];
	uint8_t macAddress[5];
};

struct settings {
	struct preferences preferences;
	struct cranksets cranksets;
	struct sprockets sprockets;
	struct sensor sensors[50];
};


void saveToFlash(uint8_t *data, uint32_t size);
void readFlash(uint8_t* data);


#endif
