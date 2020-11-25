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
#include "app_ble.h"

#define bufferMaxValue	10

struct BikeDataInformation_t bikeDataInformation = {0};

float prevWheelData = 0;
float currentWheelData = 0;
float prevWheelEvent = 0;
float currentWheelEvent = 0;

float prevCrankData = 0;
float currentCrankData = 0;
float prevCrankEvent = 0;
float currentCrankEvent = 0;

float wheelCircumference = 2.2;
float rpmValue = 0;

float crankRevValue[bufferMaxValue][4] = {0};
int indexCrankRevValue = 0;

float wheelRevValue[bufferMaxValue][6] = {0};
int indexWheelRevValue = 0;
bool init = true;

void switchCase(int* value){

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
            wheelRevFunction(WheelValueSent);
            break;

        case 2:
            // set values to be sent
            CrankValueSent[0] = value[2];
            CrankValueSent[1] = value[1];
            CrankValueSent[2] = value[4];
            CrankValueSent[3] = value[3];

            // send values
            crankRevFunction(CrankValueSent);
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
            wheelRevFunction(WheelValueSent);
            crankRevFunction(CrankValueSent);
            break;

        default:
            printf("No service");
    }
}

void wheelRevFunction(int* wheelValue){

     prevWheelData = currentWheelData;
    currentWheelData = wheelCircumference*((wheelValue[0]<<24) + (wheelValue[1]<<16) + (wheelValue[2]<<8) + (wheelValue[3]));

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
		//bikeDataInformation.speed.time = getSensorsTime();    // a changer avec le timer
		//printf("Votre vitesse est: %f km/h\n\r", kmhValue);
    }
}

void crankRevFunction(int* CrankValue){

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
			rpmValue = rpmValue;
		}
		else{
			if(currentCrankEvent < prevCrankEvent){
				currentCrankEvent = currentCrankEvent + 64; // Ce if est pour gerer le cas ou nous avons un overflow
			}

			rpmValue = ((currentCrankData-prevCrankData)/(currentCrankEvent-prevCrankEvent))*60;
			bikeDataInformation.cadence.value = rpmValue;
			//printf("Votre cadence est: %f rpm\n\r", rpmValue);
			bikeDataInformation.cadence.time = getSensorsTime();    // a changer avec le timer
		}
	}

}

void powerFunction(int* powerData){
	float powerValue = powerData[1] + (powerData[0]*256);
	bikeDataInformation.power.value = powerValue;
	//bikeDataInformation.power.time = getSensorsTime();    // a changer avec le timer
	//printf("power : %f\n\r",bikeDataInformation.power.value);
}

void algoCases(void){
    float puissance = bikeDataInformation.power.value;
//  float puissance = 240;
	float cadence = bikeDataInformation.cadence.value;
	float vitesse = bikeDataInformation.speed.value;
	float plateau = bikeDataInformation.pinion_fd.value;
	float pignon = bikeDataInformation.pinion_rd.value;
//	float plateau = 0;
//	float pignon = 5;
	//float Timer_puissance = bikeDataInformation.power.time;
	//float Timer_cadence = bikeDataInformation.cadence.time;
	//float Timer_vitesse = bikeDataInformation.speed.time;

	float Timer_puissance = 1000;
	float Timer_cadence = 1000;
	float Timer_vitesse = 1000;



	if (init == true){
		ordonnerTableau_int(cassette,nbr_pignon);

		ordonnerTableau_int(pedalier,nbr_plateau);

		init_nbr_ratio(nbr_pignon, nbr_plateau, pointeur_nbr_ratio);


		init_tab_ratio(nbr_pignon, nbr_plateau, cassette, pedalier, pointeur_nbr_ratio, tab_ratio);
		init = false;
	}

	//gestion_changement_vitesse_manuel (pignon, plateau, pointeur_pignon_precedent, pointeur_plateau_precedent, pointeur_flag_changement_ratio, pointeur_flag);

	    // Algorithme de changements de vitesses automatisés
	    if (*pointeur_flag == 0 && Timer_puissance < 3000 && Timer_cadence < 3000 && Timer_vitesse < 3000)
	      {
	        float ratio = Obtenir_ratio(pignon, plateau, cassette, pedalier);

	        if (puissance >= ftp*1.51)
	        {
	        Sprint(puissance, cadence, vitesse, ratio, pointeur_flag_changement_ratio, pointeur_flag);
	        }

	        else if (puissance <= ftp*0.24)
	        {
	        Repos(puissance, cadence, vitesse, ratio, pointeur_nbr_ratio, Diametre_roues, tab_ratio, pointeur_flag_changement_ratio);
	        }

	        else if (puissance < ftp*1.51 && puissance > ftp*0.24)
	        {
	        Normale(puissance, cadence, vitesse, ratio, pointeur_nbr_ratio, Diametre_roues, Cadence_des, tab_ratio, pointeur_flag_changement_ratio);
	        }
	      }
	    else
	      {
	        if (*pointeur_flag != 0)
	        {
	        *pointeur_flag = *pointeur_flag-1;
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
		bikeDataInformation.cadence.time = 0;    // a changer avec le timer
		//printf("plateau : %f	cassette : %f\n\r",bikeDataInformation.pinion_fd.value, bikeDataInformation.pinion_rd.value);
	}
}

