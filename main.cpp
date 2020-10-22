#include "mbed.h"

#define fech 40000 // Fréquence d'échantillonnage souhaitée
#define NB_COEFF 21 // Nombre de coefficients du filtre RIF
//du filtre RIF pour gérer la pile glissante d'echantillons

Serial pc(USBTX, USBRX);

AnalogIn entree_analogique(PC_3); // A2 connecteur Arduino
AnalogOut sortie_analogique(PA_5); // D13 Arduino
PwmOut sortie(PE_9); // D6 Arduino
DigitalOut visu(PG_9) ; // D0 Arduino
Ticker ADC_DAC ;

// Filtre RIF passe_bas fc = 2000 Hz généré avec Octave fe réelle = 40 kHz
// Fe = 1 dans Octave --> Fc Octavve = 2000/40000 = 0.075
// b = fir1(20,0.075) ; 21 = nb coeffients dans b
float coeff_filtre[NB_COEFF] = {0.0026603,0.0045069,0.0091798,0.017556,
                                0.029809,0.045263,0.062429,0.079237,0.093403,
                                0.10286,0.10619,0.10286,0.093403,0.079237,
                                0.062429,0.045263,0.029809,0.017556,0.0091798,
                                0.0045069,0.0026603
                               } ;
// Passe pas généré avec Matlab outil filterDesign
// Lowpass, FIR leas-square, fe = 40 kHz, ordre 20,
// fpass = 1000 Hz, fstop = 2000 Hz
/*float coeff_filtre[NB_COEFF] = {
    0.01815973036,  0.02545970678,  0.03320529684,  0.04110899568,  0.04885519296,
    0.05611754954,  0.06257762015,   0.0679435432,  0.07196751237,  0.07446079701,
    0.07530529052,  0.07446079701,  0.07196751237,   0.0679435432,  0.06257762015,
    0.05611754954,  0.04885519296,  0.04110899568,  0.03320529684,  0.02545970678,
    0.01815973036
};*/

// Temps de traitement d'un échantillon 1.3 us
float fir(float xn)
{
    static float in[NB_COEFF + 1] ;
    int k ;
    float yn = 0 ;
    in[0] = xn ; // Entrée de l'échantillon courant au début du tableau
    for (k = NB_COEFF - 1 ; k >=0 ; k--) {
        yn += in[k]*coeff_filtre[k] ;
        in[k+1] = in[k] ; // Glissement des échantillons dans la pile après utilisation
    }
    return(yn);
}

// Deux versions avec des pointeurs, moins performantes que la version de base
// Temps de traitement d'un échantillon = 1.44 us
/*
float fir(float xn)
{
    static float in[TAILLE_TAB] ;
    float *pt_in , *pt_coeff ;
    int k ;
    float yn = 0 ;
    pt_in = in ;
    *pt_in = xn ;
    pt_in = in + NB_COEFF - 1 ;
    pt_coeff = coeff_filtre + NB_COEFF - 1 ;

    for (k = NB_COEFF - 1 ; k >=0 ; k--) {
        yn += *pt_in * (*pt_coeff) ;
        *(pt_in + 1) = *(pt_in) ;
        pt_in-- ;
        pt_coeff--;
    }
    return(yn);
}
*/
// Temps de traitement d'un échantillon = 1.96 us
/*float fir(float xn)
{
    static float in[TAILLE_TAB] ;
    float *pt_in , *pt_coeff ;
    int k ;
    float yn = 0 ;
    pt_in = in ;
    *pt_in = xn ;
    pt_in = in + NB_COEFF - 1 ;
    pt_coeff = coeff_filtre + NB_COEFF - 1 ;

    for (k = NB_COEFF - 1 ; k >=0 ; k--) {
        yn += *pt_in-- * (*pt_coeff--) ;
        *(pt_in) = *(pt_in - 1) ;
    }
    return(yn);
}*/

void filtrage()
{
    float x,y ;
    x = entree_analogique.read();
    visu = 1 ;
    y = fir(x) ;
    visu = 0 ;
    sortie_analogique.write(y) ;
}

int main()
{

    pc.printf("\nEssai filtrage numerique temps reel fech = %d Hz\n",fech);
    pc.printf("Connectez la sortie D6 (signal a filtrer) sur l'entree A2 \n");
    pc.printf("Visualisez signal filtre sur la broche D13 \n");
    pc.printf("Mesure du temps de traitement du filtrage sur D0 \n");
    pc.printf("Traitement en cours : \n");
    ADC_DAC.attach_us(&filtrage,1000000/fech);
    sortie.period_us(2000);
    sortie.write(0.5f) ; // rapport cyclique 1/2
    while(1) {
    }
}
