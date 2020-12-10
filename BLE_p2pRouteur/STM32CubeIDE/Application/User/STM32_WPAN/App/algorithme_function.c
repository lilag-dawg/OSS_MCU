/*
 * algorithme_function.c
 *
 *  Created on: 11 nov. 2020
 *      Author: Guillaume
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_entry.h"
#include "app_common.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32_lpm.h"
#include "stm32_seq.h"
#include "dbg_trace.h"
#include "hw_conf.h"
#include "otp.h"
#include "saveToFlash.h"
#include "timer.h"
#include "algorithme_function.h"
/* USER CODE END Includes */


// Constante
uint16_t ftp; // reçu de l'app
int Diametre_roues = 700; // reçu de l'app
uint16_t Cadence_des; // reçu de l'app
float shifting_responsiveness; // reçu de l'app
uint16_t bpm_des; // reçu de l'app

int nbr_pignon; // reçu de l'app
int nbr_plateau; // reçu de l'app

uint8_t cassette[sizeof(sprockets)]; // reçu de l'app
uint8_t pedalier[sizeof(cranksets)]; // reçu de l'app

int nbr_ratio = 14; // valeur par défaut

float tab_ratio[nbr_ratio_max];

// Drapeaux d'interruption
int flag = 0;

int flag_changement_ratio = 0;

// Pointeurs données précédentes

int plateau_precedent = 0;

int pignon_precedent = 8;

// Fonction de l'algorithme
// Déclaration de la fonction qui change de vitesse pour un plus grand ratio
void Augmenter_ratio()
{
    //printf ("\n\r\n\r Augmenter ratio \n\r\n\r ");
    flag_changement_ratio = 1;
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
      startRelayTimer(GPIO_PIN_6);
      //HAL_Delay(50);
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
}

// Déclaration de la fonction qui change de vitesse pour un plus petit ratio
void Diminuer_ratio()
{
    //printf ("\n\r\n\r Diminuer ratio \n\r\n\r");
    flag_changement_ratio = 1;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
    startRelayTimer(GPIO_PIN_7);
	//HAL_Delay(50);
	//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
}

// Déclaration de la fonction qui nous donne le ratio en fonction des plateaux et des pignons
float Obtenir_ratio(int pignon, int plateau)
{
    float ratio = (float)pedalier[plateau]/cassette[pignon];
    //printf ("ratio %f\n", ratio);
    return ratio;



}

// Déclaration de la fonction qui nous donne l'indice du ratio
int Obtenir_indice_ratio(float ratio, float *tab_ratio)
{
    int i = 0;
    float ratio_tmp = 0;
    while (ratio_tmp < ratio)
        {
        //printf ("indice %d\n", i);
        //printf ("ratio %f et tab_ratio %f\n", ratio, tab_ratio[i]);
        i++;
        ratio_tmp = tab_ratio[i];
        }
    if ( (tab_ratio[i]-ratio) > (ratio-tab_ratio[i-1]))
        {
        i = i-1;
        }
        //printf ("indice %d\n", i);
        //printf ("ratio %f et tab_ratio %f\n", ratio, tab_ratio[i]);
    return i;
}

// Déclaration de la fonction qui nous donne l'indice du ratio minimum en cas de repos
int Obtenir_indice_ratio_min_repos(float *tab_ratio)
{
    int i = 0;
    float ratio_tmp = 0;
    while (ratio_tmp < 2)
        {
        i++;
        ratio_tmp = tab_ratio[i];
        //printf ("indice du ratio min lors du repos %d\n", indice_ratio_min_repos);
        //printf ("tab_ratio %f\n", tab_ratio[i]);
        }
    return i;
}

// Déclaration de la fonction qui gère les situations de sprints
void Sprint(float puissance, float cadence, float vitesse, float ratio)
{
    printf ("Sprint à %f Watt\n", puissance);

    if (cadence > 130)
    {
    Augmenter_ratio();
    flag = 3;
    }
}

// Déclaration de la fonction qui gère les situations de repos
void Repos(float puissance, float cadence, float vitesse, float ratio)
{
    printf ("Repos a %f Watt\n\r", puissance);

    int i = Obtenir_indice_ratio_min_repos(tab_ratio);
    //printf ("indice %d\n", i);
    //printf ("tab_ratio %f\n", tab_ratio[i]);

    int j = Obtenir_indice_ratio(ratio, tab_ratio);
    //printf ("indice %d\n", j);
    //printf ("ratio %f et tab_ratio %f\n", ratio, tab_ratio[j]);

    if(cadence > 1 && j > i)
        {
        float Cadence_theorique = (vitesse/3.6)/((float)Diametre_roues/1000*3.14159)*60/tab_ratio[j];
        //printf ("cadence théorique %f\n", Cadence_theorique);
        if(Cadence_theorique < 80 && j != 0)
            {
            Diminuer_ratio();
            }

        if(Cadence_theorique > 100 && j != (nbr_ratio-1))
            {
            Augmenter_ratio();
            }
        }

}

// Déclaration de la fonction qui gère le système en utilisation normale
void Normale(float puissance, float cadence, float vitesse, float ratio)
{
    float facteur_lim_sup = 1.04; // Le facteur doit être supérieur ou égal à 1
    float facteur_lim_inf = 0.96; // Le facteur doit être inférieur ou égal à 1

    printf ("Normale à %f Watt\n", puissance);

    int i = Obtenir_indice_ratio(ratio, tab_ratio);
    //printf ("indice %d\n", i);
    //printf ("ratio %f et tab_ratio %f\n", ratio, tab_ratio[i]);

    if(cadence < (Cadence_des * facteur_lim_inf) && i != 0)
        {
        float Cadence_sup = (vitesse/3.6)/((float)Diametre_roues/1000*3.14159)*60/tab_ratio[i-1];
        //printf ("cadence_sup %f\n", Cadence_sup);
        if(Cadence_sup < (Cadence_des * facteur_lim_sup))
            {
            Diminuer_ratio();
            }
        }

    if(cadence > (Cadence_des * facteur_lim_sup) && i != (nbr_ratio-1))
        {
        float Cadence_inf = (vitesse/3.6)/((float)Diametre_roues/1000*3.14159)*60/tab_ratio[i+1];
        //printf ("cadence_inf à %f\n", Cadence_inf);
        if(Cadence_inf > Cadence_des * facteur_lim_inf)
            {
            Augmenter_ratio();
            }
        }
}

// Déclaration de la fonction d'initialisation du nombre de ratio séquentiel
void init_nbr_ratio()
{
    if (nbr_pignon*nbr_plateau == 22)
        {
        nbr_ratio = 14;
        }
    if (nbr_pignon*nbr_plateau == 24)
        {
        nbr_ratio = 15;
        }
    if (nbr_pignon*nbr_plateau == 11)
        {
        nbr_ratio = 11;
        }
    if (nbr_pignon*nbr_plateau == 12)
        {
        nbr_ratio = 12;
        }
    //printf("%d\n ",*pointeur_nbr_ratio);
}

// Déclaration de la fonction d'initialisation de la table de ratios séquentiels disponibles
void init_tab_ratio()
{
    for(int i=0; i<nbr_pignon; i++)
    {
    tab_ratio[(nbr_ratio-nbr_pignon) + i] = (float)pedalier[nbr_plateau-1]/cassette[(nbr_pignon-1)-i];
    }

    if (nbr_plateau == 2)
    {
        for(int i=0; i<(nbr_ratio-nbr_pignon); i++)
        {
        tab_ratio[i] = (float)pedalier[nbr_plateau-2]/cassette[(nbr_pignon-1)-i];
        }
    }

    //for(int j=0; j<*pointeur_nbr_ratio; j++)
    //{
    //printf("%f ",tab_ratio[j]);
    //}
}

// Déclaration de la fonction de gestion des changements de vitesse manuel
void gestion_changement_vitesse_manuel (int pignon, int plateau)
{
    if (pignon != pignon_precedent && flag_changement_ratio != 1)
        {
            //printf("Changement de vitesse manuel\n");
            //printf("pignon %d et pignon precedent %d\n", pignon, *pointeur_pignon_precedent);
            flag = 6;
            flag_changement_ratio = 0;
        }

    if (plateau != plateau_precedent && flag_changement_ratio != 1)
        {
            //printf("Changement de vitesse manuel\n");
            //printf("plateau %d et plateau precedent %d\n", plateau, *pointeur_plateau_precedent);
            flag = 6;
            flag_changement_ratio = 0;
        }

    pignon_precedent = pignon;
    plateau_precedent = plateau;
    flag_changement_ratio = 0;
}

void updateAlgoSettingsFromFlash(settings_t *readSettings) {

	gearsStructToArray(&readSettings->sprockets, &readSettings->cranksets);

	ftp = readSettings->preferences.ftp;
	Cadence_des = readSettings->preferences.desiredRpm;
	shifting_responsiveness = readSettings->preferences.shiftingResponsiveness;
	bpm_des = readSettings->preferences.desiredBpm;
}

void gearsStructToArray(sprockets *sprocketStruct, cranksets *cranksetStruct) {

	//sprockets
	  memset(cassette, 0, sizeof(sprockets));
	  memcpy(cassette, sprocketStruct, sizeof(sprockets));

	  nbr_pignon = sizeof(sprockets);
	  for(int i = 0; i < sizeof(sprockets); i++) {
		if(cassette[i] == 0)  {
			nbr_pignon--;
		}
	  }

	//cranksets
	  uint8_t pedalierTemp[sizeof(cranksets)];
	  memset(pedalierTemp, 0, sizeof(cranksets));
	  memcpy(pedalierTemp, cranksetStruct, sizeof(cranksets));

	  memset(pedalier, 0, sizeof(cranksets));

	  int i = 0, k = sizeof(pedalierTemp) -1;
	  nbr_plateau = sizeof(pedalierTemp);
	  while(k >= 0) {
		  if(pedalierTemp[k] != 0) {
			  pedalier[i++] = pedalierTemp[k];
		  }
		  else {
			  nbr_plateau--;
		  }
		  k--;
	  }
}

int* getFlag() {
	return &flag;
}

uint16_t* getFtp() {
	return &ftp;
}
