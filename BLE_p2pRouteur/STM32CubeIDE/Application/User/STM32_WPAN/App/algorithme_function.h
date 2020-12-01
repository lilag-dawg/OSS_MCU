/*
 * algorithme_function.h
 *
 *  Created on: 11 nov. 2020
 *      Author: Guillaume
 */

#ifndef APPLICATION_USER_STM32_WPAN_APP_ALGORITHME_FUNCTION_H_
#define APPLICATION_USER_STM32_WPAN_APP_ALGORITHME_FUNCTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "saveToFlash.h"

#define nbr_ratio_max		15

// Fonction de l'algorithme
// Déclaration de la fonction qui change de vitesse pour un plus grand ratio
void Augmenter_ratio();

// Déclaration de la fonction qui change de vitesse pour un plus petit ratio
void Diminuer_ratio();

// Déclaration de la fonction qui nous donne le ratio en fonction des plateaux et des pignons
float Obtenir_ratio(int pignon, int plateau);

// Déclaration de la fonction qui nous donne l'indice du ratio
int Obtenir_indice_ratio(float ratio, float *tab_ratio);

// Déclaration de la fonction qui nous donne l'indice du ratio minimum en cas de repos
int Obtenir_indice_ratio_min_repos(float *tab_ratio);

// Déclaration de la fonction qui gère les situations de sprints
void Sprint(float puissance, float cadence, float vitesse, float ratio);

// Déclaration de la fonction qui gère les situations de repos
void Repos(float puissance, float cadence, float vitesse, float ratio);

// Déclaration de la fonction qui gère le système en utilisation normale
void Normale(float puissance, float cadence, float vitesse, float ratio);

// Déclaration de la fonction d'initialisation du nombre de ratio séquentiel
void init_nbr_ratio();

// Déclaration de la fonction d'initialisation de la table de ratios séquentiels disponibles
void init_tab_ratio();

// Déclaration de la fonction de gestion des changements de vitesse manuel
void gestion_changement_vitesse_manuel (int pignon, int plateau);

void updateAlgoSettingsFromFlash(settings_t *readSettings);

void gearsStructToArray(sprockets *sprocketStruct, cranksets *cranksetStruct);

int* getFlag();

uint16_t* getFtp();

#endif /* APPLICATION_USER_STM32_WPAN_APP_ALGORITHME_FUNCTION_H_ */
