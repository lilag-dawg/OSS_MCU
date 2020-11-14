/*
 * algorithme_function.c
 *
 *  Created on: 11 nov. 2020
 *      Author: Guillaume
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fonction de l'algorithme
// Déclaration de la fonction qui change de vitesse pour un plus grand ratio
void Augmenter_ratio()
{
    printf ("Augmenter ratio\n");
    //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	//HAL_Delay(50);
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
}

// Déclaration de la fonction qui change de vitesse pour un plus petit ratio
void Diminuer_ratio()
{
    printf ("Diminuer ratio\n");
    //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	//HAL_Delay(50);
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
}

// Déclaration de la fonction qui nous donne le ratio en fonction des plateaux et des pignons
float Obtenir_ratio(int pignon, int plateau, int *cassette, int*pedalier)
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
    while (ratio_tmp <= ratio)
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
void Sprint(float puissance, float cadence, float vitesse, float ratio,int *pointeur_flag_sprint)
{
    printf ("Sprint à %f Watt\n", puissance);

    if (cadence > 130)
    {
    Augmenter_ratio();
    *pointeur_flag_sprint = 3;
    }
}

// Déclaration de la fonction qui gère les situations de repos
void Repos(float puissance, float cadence, float vitesse, float ratio, int *pointeur_flag_repos, int Diametre_roues, float *tab_ratio)
{
    printf ("Repos à %f Watt\n", puissance);

    int i = Obtenir_indice_ratio_min_repos(tab_ratio);
    //printf ("indice %d\n", i);
    //printf ("tab_ratio %f\n", tab_ratio[i]);

    int j = Obtenir_indice_ratio(ratio, tab_ratio);
    //printf ("indice %d\n", j);
    //printf ("ratio %f et tab_ratio %f\n", ratio, tab_ratio[j]);

    if(cadence > 1 && j > i)
        {
        float Cadence_theorique = (vitesse/3.6)/((float)Diametre_roues/1000*3.14159)*60/tab_ratio[j];
        printf ("cadence théorique %f\n", Cadence_theorique);
        if(Cadence_theorique < 80)
            {
            Diminuer_ratio();
            }

        if(Cadence_theorique > 100)
            {
            Augmenter_ratio();
            }
        }

}

// Déclaration de la fonction qui gère le système en utilisation normale
void Normale(float puissance, float cadence, float vitesse, float ratio, int *pointeur_flag_normale, int Diametre_roues, int Cadence_des, float *tab_ratio)
{
    printf ("Normale à %f Watt\n", puissance);

    int i = Obtenir_indice_ratio(ratio, tab_ratio);
    //printf ("indice %d\n", i);
    //printf ("ratio %f et tab_ratio %f\n", ratio, tab_ratio[i]);

    if(cadence < Cadence_des)
        {
        float Cadence_sup = (vitesse/3.6)/((float)Diametre_roues/1000*3.14159)*60/tab_ratio[i-1];
        printf ("cadence_sup %f\n", Cadence_sup);
        if(Cadence_sup < Cadence_des*1.04)
            {
            Diminuer_ratio();
            }
        }

    if(cadence > Cadence_des)
        {
        float Cadence_inf = (vitesse/3.6)/((float)Diametre_roues/1000*3.14159)*60/tab_ratio[i+1];
        printf ("cadence_inf à %f\n", Cadence_inf);
        if(Cadence_inf > Cadence_des*0.96)
            {
            Augmenter_ratio();
            }
        }
}

// Déclaration de la fonction de tri en ordre croissant pour des int
void ordonnerTableau_int(int tableau[],int tailleTableau)
{
    int i,t,k = 0;

    for(t = 1; t < tailleTableau; t++)
    {
        for(i=0; i < tailleTableau - 1; i++)
        {
            if(tableau[i] > tableau[i+1])
            {
            k= tableau[i] - tableau[i+1];
            tableau[i] -= k;
            tableau[i+1] += k;
            }
        }
    }
    //for(int j=0; j<tailleTableau; j++)
    //{
    //printf("%d ",tableau[j]);
    //}
    //printf("\n");
}

// Déclaration de la fonction de tri en ordre croissant pour des float
void ordonnerTableau_float(float tableau[],int tailleTableau)
{
    int i,t = 0;
    float k = 0;

    for(t = 1; t < tailleTableau; t++)
    {
        for(i=0; i < tailleTableau - 1; i++)
        {
            if(tableau[i] > tableau[i+1])
            {
            k= tableau[i] - tableau[i+1];
            tableau[i] -= k;
            tableau[i+1] += k;
            }
        }
    }
    //for(int j=0; j<tailleTableau; j++)
    //{
    //printf("%f ",tableau[j]);
    //}
    //printf("\n");
}
// Déclaration de la fonction d'initialisation du nombre de ratio séquentiel
void init_nbr_ratio(int nbr_pignon, int nbr_plateau, int *pointeur_nbr_ratio)
{
    if (nbr_pignon*nbr_plateau == 22)
        {
        *pointeur_nbr_ratio = 14;
        }
    if (nbr_pignon*nbr_plateau == 24)
        {
        *pointeur_nbr_ratio = 15;
        }
    if (nbr_pignon*nbr_plateau == 11)
        {
        *pointeur_nbr_ratio = 11;
        }
    if (nbr_pignon*nbr_plateau == 12)
        {
        *pointeur_nbr_ratio = 12;
        }
    //printf("%d\n ",*pointeur_nbr_ratio);
}

// Déclaration de la fonction d'initialisation de la table de ratios séquentiels disponibles
void init_tab_ratio(int nbr_pignon, int nbr_plateau, int *cassette, int *pedalier, int *pointeur_nbr_ratio, float *tab_ratio)
{
    for(int i=0; i<nbr_pignon; i++)
    {
    tab_ratio[(*pointeur_nbr_ratio-nbr_pignon) + i] = (float)pedalier[nbr_plateau-1]/cassette[(nbr_pignon-1)-i];
    }

    if (nbr_plateau == 2)
    {
        for(int i=0; i<(*pointeur_nbr_ratio-nbr_pignon); i++)
        {
        tab_ratio[i] = (float)pedalier[nbr_plateau-2]/cassette[(nbr_pignon-1)-i];
        }
    }

    //for(int j=0; j<*pointeur_nbr_ratio; j++)
    //{
    //printf("%f ",tab_ratio[j]);
    //}
}
