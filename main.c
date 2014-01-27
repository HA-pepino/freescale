#include "MPC5604B_M27V.h"
#include "camera.h"
#include "milieu_ligne.h"
#include "controle.h"
#include "moteur.h"
#include "delay.h"
#include "globals.h"
#include "extern_globals.h"
#include "liaison_serie.h"
#include "reset.h"

//pepino
#include <stdio.h>
#include <string.h>

int main(void) {

    uint8_t reset = 1, Servo_F = 0, Moteur_ON = 0, Moteur_OFF = 0;
   	uint16_t titi = 0;
   	uint8_t milieu_pepino = 0;
   	uint32_t bouton, old_bouton;
   	uint16_t camera_valeurs_brutes_buff[128];
   	uint32_t but_cam;
    /******************************************************
    Tps acquisition cam + traitement = 4,2 ms
    Tps de mise à jour du servo = 20 ms
    Tps de la boucle principale: main_timer_period 
    
    Sources d'interruption:
    
        PIT: Channel 0 -> Boucle principale (10ms)
        
        EMIOS: Emios0.Channel 11 -> Compteur de front montant
    *******************************************************/
                            
                            
    /************** Initialisation variables **************/
   
    main_timer_period = 0.01; // Boucle principale: 0.010 s
  
    init();
  
    // Initialisation variables moteurs
    moteur_derniere_erreur = 0;
    moteur_integrale = 0;
    moteur_compteur = 0;
    
    // Initialisation variables servo
    /*
    pos_min_servo = 0x3d0;	//0x3b0 = real min
    pos_max_servo = 0x610;	//0x630 = real max
    pos_milieu_servo = 0x4f0;
    */
    pos_min_servo = 0x3b0;	//0x3b0 = real min
    pos_max_servo = 0x5f0;	//0x630 = real max
    pos_milieu_servo = 0x4d0;
   //pos_max_servo = 0x4d0;  pos_min_servo = 0x4d0;
   
   	old_bouton = SIU.PGPDI[2].R & 0x80000000;
    /**************** Scheduled algorithm *****************/

 
    while(1)
    {
  		
        // Ici est le code de reset
        //SIU.PGPDO[0].R = 0x00000000;		// Desactive les 2 moteurs
        //reload();
        do
        {
            // Boutons de d�marage moteur
            bouton = SIU.PGPDI[2].R & 0x80000000;
            // si on appuie sur le bouton, �a inverse le moteur
            if (bouton == 0 && old_bouton != 0)
            {
            	Moteur_ON = (Moteur_ON == 0);
            }
            old_bouton = bouton;
            
            but_cam = SIU.PGPDI[2].R & 0x40000000;
            
            reset = ! (SIU.PGPDI[2].R & 0x10000000);  // Bouton 4
            SIU.GPDO[69].B.PDO = 1;     // LED 2 OFF

            
            // if(compteur_acquisitions_invalides > 500)  Moteur_OFF = 1;
            
            // gestion de la camera
            Acquisition_Camera();
            // detection du milieu de la ligne avec les valurs jutes r�cup�r�es
           	milieu_pepino =  milieu_ligne_pepino();
           	// transmission debug du milieu de ligne
           /*	TransmitData("mil : ");
           	TransmitCharacter(48 + milieu_pepino/100);
           	TransmitCharacter(48 + (milieu_pepino/10)%10);
           	TransmitCharacter(48 + milieu_pepino%10);
           	TransmitCharacter(' ');*/
           	
           	// asservissement en direction
            Controle_Direction(milieu_pepino);
            //Controle_Direction(64);
            objectif_vitesse = 0.1;
            // asservissement en vitesse           
            Asserv_Vitesse(Moteur_ON);
			
			// transmission d�bug de l'activation des moteurs
			/*
           	TransmitData("m_ON :");
           	TransmitCharacter(48 + Moteur_ON);
           	TransmitCharacter(' ');
           	*/
           	
           //	printfloat(-1234.567*0.00001);
           	
           	//TransmitCharacter(' ');
		/*	printfloat(-50.65);
           	TransmitCharacter(' ');
			printfloat(50000,05);
           	TransmitCharacter(' ');
			printfloat(50,65);
           	TransmitCharacter(' ');
			printfloat(-0,00000000546);*/
		

			//transmission s�rie, pour eviter de trop prendre de temps, on la fait petit � petit (8 par 8)
            // on commence par recopier les valeur de la camera au d�but dans _buff
            // ensuite on print caract�re par caract�re dans une petite chaine pour la mise en forme de la liason s�rie
            
            if (titi == 0 )
            {
            	TransmitData("\nstart\n");
            	for (titi = 0; titi< 128; titi++)
            	{
            		if (but_cam == 0)
            			camera_valeurs_brutes_buff[titi] = 	camera_valeurs_brutes1[titi];
            		else 
            			camera_valeurs_brutes_buff[titi] = 	camera_valeurs_brutes2[titi];
            		
            	}
            	titi = 0;
            }
            if (titi < 128)
            {
            	
	            do 
	            {
	            //	ecrire_chaine(chaine_courte, camera_valeurs_brutes_buff[tototiti]);
	            	
	            //	TransmitData(chaine_courte);
	            	TransmitCharacter('"');
	            	printhex16(camera_valeurs_brutes_buff[titi]);
	            	TransmitCharacter('"');
	            		
	           		TransmitCharacter('\n');
	            	titi++;
	            }while (titi%8 != 0);
            }
           	//TransmitCharacter('\n');
            else if (titi == 150) titi = 0;
            else titi++;


           	
           	
	        //TransmitCharacter('\n');
            
            


            //camera_valeurs_brutes 
            //TransmitCharacter();
            while(! main_fin_boucle) asm("wait"); // attend l'interruption sur timer pour redemarer un tour de boucle // Evite de revenir dans la boucle quand il y a des interruptions sur le capteur de vitesse
            main_fin_boucle = 0;
            
            SIU.GPDO[68].B.PDO = !	SIU.GPDO[68].B.PDO;

        }
        while(! reset );
        delay(10000000);
    	moteur_integrale = 0;
    }	
}



//transmission s�rie, pour eviter de trop prendre de temps, on la fait petit � petit (8 par 8)
            // on commence par recopier les valeur de la camera au d�but dans _buff
            // ensuite on print caract�re par caract�re dans une petite chaine pour la mise en forme de la liason s�rie
            
            /*if (tototiti == 0 )
            {
            	TransmitData("\nstart\n");
            	for (tototiti = 0; tototiti< 128; tototiti++)
            	{
            		camera_valeurs_brutes_buff[tototiti] = 	camera_valeurs_brutes[tototiti];
            	}
            	tototiti = 0;
            }
            if (tototiti < 128)
            {
            	
	            do 
	            {
	            //	ecrire_chaine(chaine_courte, camera_valeurs_brutes_buff[tototiti]);
	            	
	            //	TransmitData(chaine_courte);
	            	TransmitCharacter('"');
	            	printhex16(camera_valeurs_brutes_buff[tototiti]);
	            	TransmitCharacter('"');
	            		
	           		TransmitCharacter('\n');
	            	tototiti++;
	            }while (tototiti%8 != 0);
            }
           	//TransmitCharacter('\n');
            else if (tototiti == 150) tototiti = 0;
            else tototiti++;*/

