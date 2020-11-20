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

#define nbr_ratio_max		15
#define nbr_pignon_max		12
#define nbr_plateau_max		2

// Constante
// Constante
float ftp = 246; // reçu de l'app
int Diametre_roues = 700; // reçu de l'app
int Cadence_des = 90; // reçu de l'app

int nbr_pignon = 11; // reçu de l'app
int nbr_plateau = 2; // reçu de l'app

int cassette[nbr_pignon_max] = {11, 12, 13, 14, 15, 17, 19, 21, 24, 27, 30, 0}; // reçu de l'app
int pedalier[nbr_plateau_max] = {52, 36}; // reçu de l'app

int nbr_ratio = 14; // valeur par défaut
int *pointeur_nbr_ratio = &nbr_ratio; // valeur par défaut

float tab_ratio[nbr_ratio_max];

// Drapeaux d'interruption
int flag = 0;
int *pointeur_flag = &flag;

int flag_changement_ratio = 0;
int *pointeur_flag_changement_ratio = &flag_changement_ratio;

// Pointeurs données précédentes

int plateau_precedent = 0;
int *pointeur_plateau_precedent = &plateau_precedent;

int pignon_precedent = 8;
int *pointeur_pignon_precedent = &pignon_precedent;

// Fonction de l'algorithme
// Déclaration de la fonction qui change de vitesse pour un plus grand ratio
void Augmenter_ratio(int *pointeur_flag_changement_ratio);

// Déclaration de la fonction qui change de vitesse pour un plus petit ratio
void Diminuer_ratio(int *pointeur_flag_changement_ratio);

// Déclaration de la fonction qui nous donne le ratio en fonction des plateaux et des pignons
float Obtenir_ratio(int pignon, int plateau, int *cassette, int *pedalier);

// Déclaration de la fonction qui nous donne l'indice du ratio
int Obtenir_indice_ratio(float ratio, float *tab_ratio);

// Déclaration de la fonction qui nous donne l'indice du ratio minimum en cas de repos
int Obtenir_indice_ratio_min_repos(float *tab_ratio);

// Déclaration de la fonction qui gère les situations de sprints
void Sprint(float puissance, float cadence, float vitesse, float ratio, int *pointeur_flag_changement_ratio, int *pointeur_flag);

// Déclaration de la fonction qui gère les situations de repos
void Repos(float puissance, float cadence, float vitesse, float ratio, int *pointeur_nbr_ratio, int Diametre_roues, float *tab_ratio, int *pointeur_flag_changement_ratio);

// Déclaration de la fonction qui gère le système en utilisation normale
void Normale(float puissance, float cadence, float vitesse, float ratio, int *pointeur_nbr_ratio, int Diametre_roues, int Cadence_des, float *tab_ratio, int *pointeur_flag_changement_ratio);

// Déclaration de la fonction de tri en ordre croissant pour des int
void ordonnerTableau_int(int tableau[],int tailleTableau);

// Déclaration de la fonction de tri en ordre croissant pour des float
void ordonnerTableau_float(float tableau[],int tailleTableau);

// Déclaration de la fonction d'initialisation du nombre de ratio séquentiel
void init_nbr_ratio(int nbr_pignon, int nbr_plateau, int *pointeur_nbr_ratio);

// Déclaration de la fonction d'initialisation de la table de ratios séquentiels disponibles
void init_tab_ratio(int nbr_pignon, int nbr_plateau, int *cassette, int *pedalier, int *pointeur_nbr_ratio, float *tab_ratio);

// Déclaration de la fonction de gestion des changements de vitesse manuel
void gestion_changement_vitesse_manuel (int pignon, int plateau, int *pointeur_pignon_precedent, int*pointeur_plateau_precedent, int *pointeur_flag_changement_ratio, int *pointeur_flag);

#endif /* APPLICATION_USER_STM32_WPAN_APP_ALGORITHME_FUNCTION_H_ */
