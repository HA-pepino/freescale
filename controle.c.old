#include "controle.h"
#include "constantes.h"
#include "milieu_ligne.h"
#include "liaison_serie.h"
#include "extern_globals.h"

#include <math.h>


#define max(x,y) (x<y?y:x)
 
void Controle_Direction(uint8_t milieu_pepino)
{
   /* uint8_t pos_milieu;
    uint8_t incertitude;
	int8_t erreur;*/
	int16_t derivee;
	float commande;
	static float commande_derivee = 0;
 	uint16_t commande_bornee;
	static uint32_t compteur_ligne_arrivee = 0; // On regarde combien de fois on croise la ligne d'arrivé
    static int toto =0;
	int16_t erreur_pepino;
    objectif_vitesse = 0.5;
	/*
    milieu_ligne(&pos_milieu, &incertitude);
    compteur_ligne_arrivee++;

    if(incertitude < CONTROLE_INCERTITUDE_PALIER)
    {
        controle_derniere_position = pos_milieu;
        compteur_acquisitions_invalides = 0;
    }
    else
    {
    	//pos_milieu = controle_derniere_position;
    	//compteur_acquisitions_invalides++;
    	pos_milieu = 60;
    }
    if(code_arret_cam() == 1)
    {
        if (compteur_ligne_arrivee >= COMPTEUR_AVANT_ARRIVEE) // 30 secondes après le démarage de la voiture => compteur d'arrêt ligne actif
    	{
    		compteur_acquisitions_invalides = 1000;
    	    compteur_ligne_arrivee = 0;   	    
    	} 
    	SIU.GPDO[71].B.PDO = 0;   	    	
    }
    
    SIU.GPDO[71].B.PDO = compteur_ligne_arrivee >= COMPTEUR_AVANT_ARRIVEE;
	*/


    /*objectif_vitesse = max(6-abs((int16_t)(pos_milieu)-64)/10, 2);
    
    commande = (64 - (int16_t)pos_milieu) / (objectif_vitesse);
    */
    
    // PID pour la direction
    // zieger-nicols
    //erreur = (signed char) (64 - ((int16_t)pos_milieu)); 
    
	erreur_pepino = 64 - milieu_pepino;
	
	objectif_vitesse = 0.7 - 0.02*(abs(erreur_pepino));
	if (objectif_vitesse < 0.1) objectif_vitesse = 0.1;
	derivee = erreur_pepino - controle_derniere_erreur;

	
	
	controle_derniere_erreur = erreur_pepino;
	
	controle_integrale += erreur_pepino;
 	if (controle_integrale > MAX_CONTROLE_INTEGRALE) controle_integrale = MAX_CONTROLE_INTEGRALE;
 	else if (controle_integrale < - MAX_CONTROLE_INTEGRALE) controle_integrale = -MAX_CONTROLE_INTEGRALE;
 	
    //controle_derniere_erreur = erreur;
    
    //commande =(short)( CONTROLE_MILIEU_SERVO + CONTROLE_KP * erreur + CONTROLE_KD*derivee + CONTROLE_KI*controle_integrale);
    
    commande = pos_milieu_servo;
    commande += CONTROLE_KP * erreur_pepino;
    //commande += CONTROLE_KI * controle_integrale;
    //commande += CONTROLE_KD * derivee;
    //commande_derivee = 50 * derivee + 0.9* commande_derivee;
    //commande += commande_derivee;
    
    if(commande < pos_min_servo) commande_bornee = pos_min_servo;
    else if (commande > pos_max_servo) commande_bornee = pos_max_servo; 
    else commande_bornee = (uint16_t)commande; 

    
    /*
    if (toto > 1500) toto = 0;
    else toto ++;*/
   /* toto = moteur_compteur*100;
    
    if (toto > 1000)  EMIOS_0.CH[4].CBDR.R = pos_max_servo;
    else if (toto > 500) EMIOS_0.CH[4].CBDR.R = pos_milieu_servo;
    else EMIOS_0.CH[4].CBDR.R = pos_min_servo;*/
	EMIOS_0.CH[4].CBDR.R = commande_bornee;
	
	//EMIOS_0.CH[4].CBDR.R = pos_milieu_servo + commande * AMPLITUDE_SERVO;
	
}
