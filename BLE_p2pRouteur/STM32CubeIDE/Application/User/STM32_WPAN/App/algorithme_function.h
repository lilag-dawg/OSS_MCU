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

// Constante
float ftp = 246; // reçu de l'app
int Diametre_roues = 700; // reçu de l'app
int Cadence_des = 90; // reçu de l'app

int const nbr_pignon = 11; // reçu de l'app
int const nbr_plateau = 2; // reçu de l'app

int cassette[11] = {11, 12, 13, 14, 15, 17, 19, 21, 24, 27, 30}; // reçu de l'app
int pedalier[2] = {52, 36}; // reçu de l'app

int nbr_ratio = 14; // valeur par défaut
int *pointeur_nbr_ratio = &nbr_ratio; // valeur par défaut

// Drapeaux d'interruption
int flag_sprint = 0;
int *pointeur_flag_sprint = &flag_sprint;

int flag_repos = 0;
int *pointeur_flag_repos = &flag_repos;

int flag_normale = 0;
int *pointeur_flag_normale = &flag_normale;

int flag_manuel = 0;
int *pointeur_flag_manuel = &flag_manuel;


// Fonction de l'algorithme
// Déclaration de la fonction qui change de vitesse pour un plus grand ratio
void Augmenter_ratio();

// Déclaration de la fonction qui change de vitesse pour un plus petit ratio
void Diminuer_ratio();

// Déclaration de la fonction qui nous donne le ratio en fonction des plateaux et des pignons
float Obtenir_ratio(int pignon, int plateau, int *cassette, int*pedalier);

// Déclaration de la fonction qui nous donne l'indice du ratio
int Obtenir_indice_ratio(float ratio, float *tab_ratio);

// Déclaration de la fonction qui gère les situations de sprints
void Sprint(float puissance, float cadence, float vitesse, float ratio,int *pointeur_flag_sprint);

// Déclaration de la fonction qui gère les situations de repos
void Repos(float puissance, float cadence, float vitesse, float ratio, int *pointeur_flag_repos, float *tab_vitesse, float *tab_ratio, int *pointeur_nbr_ratio);

// Déclaration de la fonction qui gère le système en utilisation normale
void Normale(float puissance, float cadence, float vitesse, float ratio, int *pointeur_flag_normale, int Diametre_roues, int Cadence_des, float *tab_ratio);

// Déclaration de la fonction de tri en ordre croissant pour des int
void ordonnerTableau_int(int tableau[],int tailleTableau);

// Déclaration de la fonction de tri en ordre croissant pour des float
void ordonnerTableau_float(float tableau[],int tailleTableau);

// Déclaration de la fonction d'initialisation du nombre de ratio séquentiel
void init_nbr_ratio(int nbr_pignon, int nbr_plateau, int *pointeur_nbr_ratio);

// Déclaration de la fonction d'initialisation de la table de ratios séquentiels disponibles
void init_tab_ratio(int nbr_pignon, int nbr_plateau, int *cassette, int *pedalier, int *pointeur_nbr_ratio, float *tab_ratio);

// Déclaration de la fonction d'initialisation de la table donnant la vitesse optimale de chaque ratios séquentiels disponibles
void init_tab_vitesse(float *tab_vitesse, int *pointeur_nbr_ratio, float *tab_ratio, int Diametre_roues, int Cadence_des);

// Déclaration de la fonction d'initialisation de la table de ratios
void init_tab_ratio_repos(int nbr_pignon, int nbr_plateau, int *cassette, int *pedalier, float *tab_ratio_repos);

// Déclaration de la fonction d'initialisation de la table donnant la vitesse optimale de chaque ratios possible
void init_tab_vitesse_repos(int nbr_pignon, int nbr_plateau, float *tab_vitesse_repos, float *tab_ratio_repos, int Diametre_roues, int Cadence_des);

#endif /* APPLICATION_USER_STM32_WPAN_APP_ALGORITHME_FUNCTION_H_ */
