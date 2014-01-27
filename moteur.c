#include "moteur.h"
#include "constantes.h"
#include "liaison_serie.h"
#include "extern_globals.h"
#include "Config_PIT.h"

void Compteur_Moteur(void)
{
	moteur_compteur ++;
	
	EMIOS_0.CH[11].CSR.B.FLAG = 0x1;
}

void Asserv_Vitesse(uint8_t Moteur_ON)
{
	double erreur;
	double derivee;
	int16_t commande;
	static uint8_t frein_ON = 0;
	static double moteur_compteur_glissant = 0;
	static double consigne_pos = 0, current_pos= 0, int_erreur_pos = 0, last_erreur_pos = 0;
	double erreur_pos, derivee_pos, increment_pos;
	uint8_t mode = 2;
	static uint8_t indice_tableau_moteur_lisse = 0;
	static uint16_t tableau_moteur_compteur[10] = {0};
	uint16_t somme_tableau;
	uint8_t toto;
	uint16_t avancement_voiture;
	
	
	avancement_voiture = moteur_compteur;
	moteur_compteur = 0;
    
    
    if (Moteur_ON == 0)	SIU.PGPDO[0].R = 0x00000000;	// Desactive les 2 moteurs
    else 				SIU.PGPDO[0].R = 0x0000C000;		// Active les 2 moteurs
    
    if (Moteur_ON == 0)
    {
    	moteur_derniere_erreur = 0;
    	moteur_compteur_glissant = 0;
    	last_erreur_pos = 0;
    	
    }
   /* else if (mode == 0 || mode == 1)
    {
   		moteur_compteur_glissant = (float)(0.9* moteur_compteur_glissant + 0.1 * avancement_voiture);
   		//mode simple
   		if (mode == 0)	erreur = objectif_vitesse - (float)avancement_voiture;
   		//mode glissant
   		else			erreur = objectif_vitesse - moteur_compteur_glissant;
   		
   		// gestion derivee
		derivee = erreur - moteur_derniere_erreur;
		moteur_derniere_erreur = erreur;
		
		moteur_integrale +=  erreur;
		// eviter les overshoot de l'intégrale
		if (moteur_integrale > MAX_MOTEUR_INTEGRALE) moteur_integrale = MAX_MOTEUR_INTEGRALE;
		if (moteur_integrale < -(MAX_MOTEUR_INTEGRALE/10)) moteur_integrale = -MAX_MOTEUR_INTEGRALE/10;
		
		commande = (int16_t)(MOTEUR_KP * erreur 
			//+ MOTEUR_KD * derivee 
			+ MOTEUR_KI * moteur_integrale);
			
    }*/
    else if (mode == 2)
    {
	    //mode position
	    // calcul de là ou la voiture veut etre
	    consigne_pos += objectif_vitesse * main_timer_period;
	    
	    //lissage des valeurs de moteur_compteur sur les 10 derières valeurs
	    if (indice_tableau_moteur_lisse == 9)
	    	indice_tableau_moteur_lisse = 0;
	   	else 
	   		indice_tableau_moteur_lisse++;
	
	    tableau_moteur_compteur[indice_tableau_moteur_lisse] = avancement_voiture;
	   	somme_tableau = 0;
	   	for (toto=0;toto<10;toto++)
	   		somme_tableau += tableau_moteur_compteur[toto];

	   		
	   	
	    // calcul de l'avancement actuel de la voiture
	    increment_pos = M_BY_TIC * somme_tableau * 0.1;
	  /*  TransmitData("inc : ");
	    printfloat(increment_pos);*/
	    current_pos = current_pos + increment_pos;
	   /* TransmitData("pos : ");
	    printfloat(current_pos);
	    TransmitData("cons : ");
	    printfloat(consigne_pos);*/
	    
	    
	    //erreur entre voulue et reelle
	    erreur_pos = consigne_pos - current_pos;
	   /* TransmitData("err : ");
	    printfloat(erreur_pos);*/
	    
	    //intégrale
	    int_erreur_pos += erreur_pos;
	    //dérivée
		derivee_pos = erreur_pos - last_erreur_pos;
		last_erreur_pos = erreur_pos;
		// calcul de la commande en conséquence
		commande = (int16_t)(
			  MOTEUR_KP_POS * erreur_pos
			+ MOTEUR_KD_POS * derivee_pos 
			+ MOTEUR_KI_POS * int_erreur_pos
			);	
		TransmitData("commande : ");
	    TransmitCharacter(48 + commande/100);
       	TransmitCharacter(48 + (commande/10)%10);
       	TransmitCharacter(48 + commande%10);
       	TransmitCharacter(' ');
		
    }
    else
    {
    	commande = 0;
    }
    
    
    //TransmitData(" mod : ");
    //TransmitCharacter(mode+48);
   	//TransmitCharacter(' ');
	
    /*
	TransmitCharacter((uint8_t)(48 + erreur_pos/100));
    TransmitCharacter((uint8_t)(48 + (erreur_pos/10)%10));
    TransmitCharacter((uint8_t)(48 + erreur_pos%10));
	*/
	
	
    // objectif_vitesse
    commande = commande < 0 ? 0 : commande;
    if (commande < 0) commande = 0;
    else if (commande > 600) commande = 600;
    
    commande = 0;
    //Moteurs en série:
    //EMIOS_0.CH[6].CBDR.R = EMIOS_0.CH[6].CADR.R + 800;// Moteurs en serie
    //Moteurs en parallèle : 
    EMIOS_0.CH[6].CBDR.R = EMIOS_0.CH[6].CADR.R + commande;//HBridge gauche
    EMIOS_0.CH[7].CBDR.R = EMIOS_0.CH[7].CADR.R + commande;//HBridge Droit



}
	/*
	// "machine à etat" : passe de état moteur ON  frein OFF
	//								  à moteur OFF frein ON
	// 		via un état intermédiaire   moteur OFF frein OFF pour eviter les court jus
	if (Moteur_ON == 1) 
    {	
    	if (commande > 0 && frein_ON == 1)
    	{
    		SIU.PGPDO[0].R = 0x00000000;	// Desactive les 2 moteurs
    		frein_ON = 0;
    		// frein OFF
    	}
    	else if (commande > 0)
    	{
    		SIU.PGPDO[0].R = 0x0000C000;		// Active les 2 moteurs
    		// frein OFF
    	}
    	else if (commande < 0 && frein_ON == 0)
    	{
    		SIU.PGPDO[0].R = 0x00000000;	// Desactive les 2 moteurs
    		frein_ON = 1;
    		// frein OFF
    	}
    	else	// if commande < 0
    	{
    		SIU.PGPDO[0].R = 0x00000000;	// Desactive les 2 moteurs
       		// frein ON
    	}
    }	
    else 
    {	
    	SIU.PGPDO[0].R = 0x00000000;
   		// frein OFF
    	    
    }		// Desactive les 2 moteurs */