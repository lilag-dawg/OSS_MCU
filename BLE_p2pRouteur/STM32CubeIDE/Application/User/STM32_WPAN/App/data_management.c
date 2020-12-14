/*
 * data_management.c
 *
 *  Created on: 8 nov. 2020
 *      Author: Isam
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <float.h>

#include "data_management.h"
#include "algorithme_function.h"
#include "timer.h"

#define bufferMaxValue	10

extern BikeDataInformation_t bikeDataInformation;

float prevWheelData = 0;
float currentWheelData = 0;
float prevWheelEvent = 0;
float currentWheelEvent = 0;

float prevCrankData = 0;
float currentCrankData = 0;
float prevCrankEvent = 0;
float currentCrankEvent = 0;

float wheelCircumference = 2.105;
float rpmValue = 0;

float crankRevValue[bufferMaxValue][4] = {0};
int indexCrankRevValue = 0;

float wheelRevValue[bufferMaxValue][6] = {0};
int indexWheelRevValue = 0;
bool init = true;

void switchCase(int* value, SensorType_t sensorType){

    int CrankValueSent[4] = {0};
    int WheelValueSent[6] = {0};

    switch (value[0]){
        case 1:
            // set values to be sent
            WheelValueSent[0] = value[4];
            WheelValueSent[1] = value[3];
            WheelValueSent[2] = value[2];
            WheelValueSent[3] = value[1];
            WheelValueSent[4] = value[6];
            WheelValueSent[5] = value[5];

            // send values
            if(sensorType != CSC_SENSOR){
              wheelRevFunction(WheelValueSent);
            }

            break;

        case 2:
            // set values to be sent
            CrankValueSent[0] = value[2];
            CrankValueSent[1] = value[1];
            CrankValueSent[2] = value[4];
            CrankValueSent[3] = value[3];

            // send values
            if(sensorType == CSC_SENSOR){
               crankRevFunction(CrankValueSent);
            }

            break;

        case 3:
            // set values to be sent
            WheelValueSent[0] = value[4];
            WheelValueSent[1] = value[3];
            WheelValueSent[2] = value[2];
            WheelValueSent[3] = value[1];
            WheelValueSent[4] = value[6];
            WheelValueSent[5] = value[5];
            CrankValueSent[0] = value[8];
            CrankValueSent[1] = value[7];
            CrankValueSent[2] = value[10];
            CrankValueSent[3] = value[9];

            // send values
            if(sensorType != CSC_SENSOR){
              wheelRevFunction(WheelValueSent);
            }
            else{
               crankRevFunction(CrankValueSent);
            }


            break;

        default:
            printf("No service");
    }
}

void wheelRevFunction(int* wheelValue){

     prevWheelData = currentWheelData;
    currentWheelData = wheelCircumference*2*((wheelValue[0]<<24) + (wheelValue[1]<<16) + (wheelValue[2]<<8) + (wheelValue[3]));

    prevWheelEvent = currentWheelEvent;
    currentWheelEvent = ((wheelValue[4]<<8) + (wheelValue[5])); // On obtient le current event en secondes

    if(currentWheelEvent < prevWheelEvent){
        currentWheelEvent = currentWheelEvent + (64*1024); // Ce if est pour gerer le cas ou nous avons un overflow
    }

    if ((currentWheelData - prevWheelData) > 0){
    	float diffWheelData = currentWheelData - prevWheelData;
    	float diffWheelEvent = currentWheelEvent - prevWheelEvent;
		float kmhValue = (3.6*1024)*(diffWheelData/diffWheelEvent);

		bikeDataInformation.speed.value = kmhValue;
		//printf("Votre vitesse est: %f km/h\n\r", kmhValue);
		//printf("Age vitesse : %f\n\r", bikeDataInformation.speed.time);
		bikeDataInformation.speed.time = getSensorsTime();    // a changer avec le timer
    }
    else{
    	bikeDataInformation.speed.value = 0;
    	bikeDataInformation.speed.time = getSensorsTime();    // a changer avec le timer
    }
}

void crankRevFunction(int* CrankValue){
	bikeDataInformation.cadence.time = getSensorsTime();
    // a changer avec le timer
	/* mettre la valeur dans le buffer */
	for(int i = 0; i < 4; i++){
		crankRevValue[indexCrankRevValue][i] = CrankValue[i];
	}

	indexCrankRevValue ++;

	bool isBufferAllSame = true;

	for (int j = 0; j<(bufferMaxValue-1);j++){
		for(int k = 0; k < 4; k++){
			if (crankRevValue[j][k] == crankRevValue[j+1][k]){
				continue;
			}
			else{
				isBufferAllSame = false;
				break;
			}
		}
		if (isBufferAllSame == false){
			break;
		}
	}

	if(indexCrankRevValue > bufferMaxValue-1){
		indexCrankRevValue = 0;
	}
	else{
	}


	/* --------------------------------*/
	if (isBufferAllSame == true){
		rpmValue = 0;
	}

	else{
		/* Traiter la donnee recue */
		prevCrankData = currentCrankData;
		currentCrankData = ((CrankValue[0]<<8) + (CrankValue[1]));

		double CurrentEvent1 = CrankValue[2]<<8;
		double CurrentEvent2 = CrankValue[3];
		prevCrankEvent = currentCrankEvent;
		currentCrankEvent = (CurrentEvent1 + CurrentEvent2)/1024; // On obtient le current event en secondes

		if (currentCrankData <= prevCrankData){
			rpmValue = 0;
			bikeDataInformation.cadence.value = rpmValue;
		}
		else{
			if(currentCrankEvent < prevCrankEvent){
				currentCrankEvent = currentCrankEvent + 64; // Ce if est pour gerer le cas ou nous avons un overflow
			}

			rpmValue = ((currentCrankData-prevCrankData)/(currentCrankEvent-prevCrankEvent))*60;
			bikeDataInformation.cadence.value = rpmValue;
			//printf("Votre cadence est: %f rpm\n\r", rpmValue);
			//printf("Age cadence : %f\n\r", bikeDataInformation.cadence.time);
		}
	}

}

void powerFunction(int* powerData){
	float powerValue = (powerData[3]*256) + powerData[2];
	int WheelValueSent[6] = {0};

    WheelValueSent[0] = powerData[9];
    WheelValueSent[1] = powerData[8];
    WheelValueSent[2] = powerData[7];
    WheelValueSent[3] = powerData[6];
    WheelValueSent[4] = powerData[11];
    WheelValueSent[5] = powerData[10];

    wheelRevFunction(WheelValueSent);



	bikeDataInformation.power.value = powerValue;
	bikeDataInformation.power.time = getSensorsTime();    // a changer avec le timer
	printf("power : %f\n\r",bikeDataInformation.power.value);
	//printf("Age power : %f\n\r", bikeDataInformation.power.time);
}

void algoCases(void){
    float puissance = bikeDataInformation.power.value;
	float cadence = bikeDataInformation.cadence.value;
	float vitesse = bikeDataInformation.speed.value;
	float plateau = bikeDataInformation.pinion_fd.value;
	float pignon = bikeDataInformation.pinion_rd.value;





	uint32_t Timer_puissance = getSensorsTime()-bikeDataInformation.power.time;
	uint32_t Timer_cadence = getSensorsTime()-bikeDataInformation.cadence.time;
	uint32_t Timer_vitesse = getSensorsTime()-bikeDataInformation.speed.time;

	//printf("\n\r\n\r Power :  %f  Cadence : %f   Vitesse : %f", puissance, cadence, vitesse);
	//printf("\n\r\n\r Age Power :  %d  Age Cadence : %d   Age Vitesse : %d", Timer_puissance, Timer_cadence, Timer_vitesse);


	if (init == true){

		init_nbr_ratio();


		init_tab_ratio();
		init = false;
	}

	//gestion_changement_vitesse_manuel (pignon, plateau, pointeur_pignon_precedent, pointeur_plateau_precedent, pointeur_flag_changement_ratio, pointeur_flag);

	    // Algorithme de changements de vitesses automatisés
		int* flag = getFlag();
		uint16_t* ftp = getFtp();
	    if (*flag == 0 && Timer_puissance < 3000 && Timer_cadence < 3000 && Timer_vitesse < 3000)
	      {
	    	printf("\n\r ok\n\r");
	        float ratio = Obtenir_ratio(pignon, plateau);

	        if (puissance >= (float)*ftp*1.51)
	        {
	        Sprint(puissance, cadence, vitesse, ratio);
	        }

	        else if (puissance <= (float)*ftp*0.24)
	        {
	        Repos(puissance, cadence, vitesse, ratio);
	        }

	        else if (puissance < (float)*ftp*1.51 && puissance > (float)*ftp*0.24)
	        {
	        Normale(puissance, cadence, vitesse, ratio);
	        }
	      }
	    else
	      {
	        if (*flag != 0)
	        {
	        *flag = *flag-1;
	        }
	      }
}

void GetRatio(int *tableau){

	if(tableau[3]<1 || tableau[3]>2){
		if(tableau[5]<1 || tableau[5]>11){

		}
	}
	else{
		switch(tableau[3]){
			case (1):
			{
				bikeDataInformation.pinion_fd.value = 0.0;
				bikeDataInformation.pinion_fd.time = getSensorsTime();
				break;
			}
			case (2):
			{
				bikeDataInformation.pinion_fd.value = 1.0;
				bikeDataInformation.pinion_fd.time = getSensorsTime();
				break;
			}
		}

		switch(tableau[5]){
					case (1):
					{
						bikeDataInformation.pinion_rd.value = 10.0;
						bikeDataInformation.pinion_rd.time = getSensorsTime();
						break;
					}
					case (2):
					{
						bikeDataInformation.pinion_rd.value = 9.0;
						bikeDataInformation.pinion_rd.time = getSensorsTime();
						break;
					}
					case (3):
					{
						bikeDataInformation.pinion_rd.value = 8.0;
						bikeDataInformation.pinion_rd.time = getSensorsTime();
						break;
					}
					case (4):
					{
						bikeDataInformation.pinion_rd.value = 7.0;
						bikeDataInformation.pinion_rd.time = getSensorsTime();
						break;
					}
					case (5):
					{
						bikeDataInformation.pinion_rd.value = 6.0;
						bikeDataInformation.pinion_rd.time = getSensorsTime();
						break;
					}
					case (6):
					{
						bikeDataInformation.pinion_rd.value = 5.0;
						bikeDataInformation.pinion_rd.time = getSensorsTime();
						break;
					}
					case (7):
					{
						bikeDataInformation.pinion_rd.value = 4.0;
						bikeDataInformation.pinion_rd.time = getSensorsTime();
						break;
					}
					case (8):
					{
						bikeDataInformation.pinion_rd.value = 3.0;
						bikeDataInformation.pinion_rd.time = getSensorsTime();
						break;
					}
					case (9):
					{
						bikeDataInformation.pinion_rd.value = 2.0;
						bikeDataInformation.pinion_rd.time = getSensorsTime();
						break;
					}
					case (10):
					{
						bikeDataInformation.pinion_rd.value = 1.0;
						bikeDataInformation.pinion_rd.time = getSensorsTime();
						break;
					}
					case (11):
					{
						bikeDataInformation.pinion_rd.value = 0.0;
						bikeDataInformation.pinion_rd.time = getSensorsTime();
						break;
					}
				}
		//printf("plateau : %f	cassette : %f\n\r",bikeDataInformation.pinion_fd.value, bikeDataInformation.pinion_rd.value);

	}
}

