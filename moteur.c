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
	float erreur;
	float derivee;
	int16_t commande;
	static uint8_t frein_ON = 0;
	static float moteur_compteur_glissant = 0;
	static float consigne_pos = 0, current_pos= 0, int_erreur_pos = 0, last_erreur_pos = 0;
	float erreur_pos, derivee_pos;
	uint8_t mode = 0;
    
    if (Moteur_ON == 0)
    {
    	moteur_derniere_erreur = 0;
    	moteur_compteur_glissant = 0;
    	last_erreur_pos = 0;
    	
    }
    else if (mode == 0 || mode == 1)
    {
   		moteur_compteur_glissant = (float)(0.9* moteur_compteur_glissant + 0.1 * moteur_compteur);
   		//mode simple
   		if (mode == 0)	erreur = objectif_vitesse - (float)moteur_compteur;
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
    }
    else
    {
	    //mode position
	    consigne_pos += objectif_vitesse * main_timer_period;
	    current_pos += M_BY_TIC * moteur_compteur;
	    erreur_pos = consigne_pos - current_pos;
	    
		derivee_pos = erreur_pos - last_erreur_pos;
		last_erreur_pos = erreur_pos;
    }
    
    
    //moteur_compteur = 0;
    

	
	commande = (int16_t)(MOTEUR_KP * erreur //+ MOTEUR_KD * derivee 
	+ MOTEUR_KI * moteur_integrale);

    
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
    	    
    }		// Desactive les 2 moteurs
	
	*/
    // objectif_vitesse
    commande = commande < 0 ? 0 : commande;
    if (commande < 0) commande = 0;
    else if (commande > 600) commande = 600;
    
	SIU.PGPDO[0].R = 0x00000000;
    
    //Moteurs en série:
    //EMIOS_0.CH[6].CBDR.R = EMIOS_0.CH[6].CADR.R + 800;// Moteurs en serie
    //Moteurs en parallèle : 
    EMIOS_0.CH[6].CBDR.R = EMIOS_0.CH[6].CADR.R + commande;//HBridge gauche
    EMIOS_0.CH[7].CBDR.R = EMIOS_0.CH[7].CADR.R + commande;//HBridge Droit

	//moteur_compteur = 0;

	// on indique que l'interruption est finie
	//PIT_ClearFlag(1);

}
