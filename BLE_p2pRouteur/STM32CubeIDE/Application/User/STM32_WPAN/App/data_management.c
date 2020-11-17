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
#define NBR_RATIO		15

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

    int CrankValueSent[4];
    int WheelValueSent[6];

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
    currentWheelData = wheelCircumference*((wheelValue[0]*(16777216)) + (wheelValue[1]*(65536)) + (wheelValue[2]*(256)) + (wheelValue[3]*(1)));

    prevWheelEvent = currentWheelEvent;
    currentWheelEvent = ((wheelValue[4]*(256)) + (wheelValue[5]*(1)))/1024; // On obtient le current event en secondes

    if(currentWheelEvent < prevWheelEvent){
        currentWheelEvent = currentWheelEvent + 64; // Ce if est pour gerer le cas ou nous avons un overflow
    }

    float kmhValue = 3.6*((currentWheelData - prevWheelData)/(currentWheelEvent - prevWheelEvent)); // le 64 est pour passer de RPS a RPM
    bikeDataInformation.speed.value = kmhValue;
    bikeDataInformation.speed.time = 0;    // a changer avec le timer
    printf("Votre vitesse est: %f km/h\n\r", kmhValue);
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

	if(indexCrankRevValue > bufferMaxValue){
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
			bikeDataInformation.cadence.time = 0;    // a changer avec le timer
			printf("Votre RMP est: %f RPM\n\r", rpmValue);
		}
	}

}


void algoCases(void){
//	float puissance = bikeDataInformation.power.value;
	float puissance = 240;
	float cadence = bikeDataInformation.cadence.value;
	float vitesse = bikeDataInformation.speed.value;
//	float plateau = bikeDataInformation.pinion_fd.value;
//	float pignon = bikeDataInformation.pinion_rd.value;
	float plateau = 0;
	float pignon = 5;
	float tab_ratio[NBR_RATIO];

	if (init == true){
		ordonnerTableau_int(cassette,nbr_pignon);

		ordonnerTableau_int(pedalier,nbr_plateau);

		init_nbr_ratio(nbr_pignon, nbr_plateau, pointeur_nbr_ratio);


		init_tab_ratio(nbr_pignon, nbr_plateau, cassette, pedalier, pointeur_nbr_ratio, tab_ratio);
		init = false;
	}

	// Algorithme de changements de vitesses automatisés
	if (*pointeur_flag_sprint == 0 && *pointeur_flag_repos == 0 && *pointeur_flag_normale == 0 && *pointeur_flag_manuel == 0)
	{
		float ratio = Obtenir_ratio(pignon, plateau, cassette, pedalier);

		if (puissance >= ftp*1.51)
		{
			Sprint(puissance, cadence, vitesse, ratio, pointeur_flag_sprint);
		}

		if (puissance <= ftp*0.24)
		{
			Repos(puissance, cadence, vitesse, ratio, pointeur_flag_repos, Diametre_roues, tab_ratio);
		}

		if (puissance < ftp*1.51 && puissance > ftp*0.24)
		{
			Normale(puissance, cadence, vitesse, ratio, pointeur_flag_normale, Diametre_roues, Cadence_des, tab_ratio);
		}
	}
	else
	{
		if (*pointeur_flag_sprint != 0)
		{
			*pointeur_flag_sprint = *pointeur_flag_sprint-1;
		}

		if (*pointeur_flag_repos != 0)
		{
			*pointeur_flag_repos = *pointeur_flag_repos-1;
		}

		if (*pointeur_flag_normale != 0)
		{
			*pointeur_flag_normale = *pointeur_flag_normale-1;
		}

		if (*pointeur_flag_manuel != 0)
		{
			*pointeur_flag_manuel = *pointeur_flag_manuel-1;
		}
	}
}

int GetRatio(int *tableau, int *pCassette, int *pPlateaux){

if(tableau[3]<1 || tableau[3]>2){
	if(tableau[5]<1 || tableau[5]>11){
		return -1;
	}
}
else{
	*pPlateaux = tableau[3];
	*pCassette = tableau[5];
	return 1;
}
}

