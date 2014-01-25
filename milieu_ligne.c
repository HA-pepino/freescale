
#include "milieu_ligne.h"
#include "constantes.h"

#define max(i, j) (i > j ? i : j)
#define min(i,j) (i > j ? j : i)

uint8_t milieu_ligne_pepino(void)
{
	//camera_valeurs_brutes
	static uint8_t centre = 67;
	float camera_moyenne[128];
	uint8_t new_min_loc, min_loc_ok = 0; 
	uint8_t erreur_min = 0, tour_de_boucle = 0;
	float loc, prec, pprec, next, nnext;
	moyennage_cam(camera_valeurs_brutes, camera_moyenne);
	new_min_loc = centre;// reprise de l'ancienne valeur

	
	
	while (min_loc_ok != 1 && erreur_min != 1)
	{
		loc = camera_moyenne[new_min_loc];
		prec = camera_moyenne[new_min_loc - 1];
		pprec = camera_moyenne[new_min_loc - 2];
		next = camera_moyenne[new_min_loc + 1];
		nnext = camera_moyenne[new_min_loc + 2];
	
		// 1 pente decroissante (dans le sens des [i]
		if (loc < prec && loc > next)
			{	new_min_loc ++; 	}	
		// 2 pente croissante
		else if (loc > prec && loc < next)
			{	new_min_loc --;		}
		// 3 bas 
		else if (loc < prec && loc < next)
			{	min_loc_ok = 1;		}
		// avec loc = next
		// 4 creux
		else if (loc < prec && loc < nnext)
		{	min_loc_ok = 1;			}
		// 5 decroissante ou plateau avec rebord à prec
		else if (loc < prec)
			{	new_min_loc ++; 	}	
		// avec loc = prec
		// 6 creux
		else if (loc < pprec && loc < next)
		{	min_loc_ok = 1;			}
		// 7 croissante ou plateau avec rebord à next
		else if (loc < next)
			{	new_min_loc --;		}
		// avec loc = next et prec
		// 8 creux
		else if (loc < pprec && loc < nnext)
		{	min_loc_ok = 1;			}
		// 9 decroissante ou plateau avec rebord à pprec
		else if (loc < pprec)
			{	new_min_loc ++; 	}
		// 10 croissante ou plateau avec rebord à nnext
		else if (loc < nnext)
			{	new_min_loc --;		}
		// 11 : plat de 5 pixels de long
		else 
		{
			erreur_min = 1;	
			new_min_loc = centre; // on reprends l'ancienne valeur
		}
		
		tour_de_boucle ++;
		// si on tourne depuis trop longtemps, on arrete, et si on s'approche trop des bords aussi
		if (tour_de_boucle > 200 || new_min_loc < 10 || new_min_loc > 117)
		{
			erreur_min = 1;	
			new_min_loc = centre; // on reprends l'ancienne valeur
		}
	}
	//if (erreur_min == 1)
	//	TransmitData("! ");
	
	centre = new_min_loc;
	return centre;
	
}

void moyennage_cam(uint16_t cam[], float moy[])
{
	// fonction qui fait une moyenne glissante sur 128 mots,
	// avec une largeur de moyenne de 7 soit 3 de chaque coté de la variable actuelle
	// * 0.14, c'est (presque) pareil que diviser par 7
	// gestion des cas début et fin à part
	char i;
	moy [0] = (float)0.14*(	 4*cam[0] + cam[1] + cam[2] + cam[3] );
	moy [1] = (float)0.14*(	 3*cam[0] + cam[1] + cam[2] + cam[3] + cam[4] );
	moy [2] = (float)0.14*(	 2*cam[0] + cam[1] + cam[2] + cam[3] + cam[5] + cam[4] );
	for (i = 3; i< 125; i++)
	{
		moy [i] = (float)0.14*(	cam[i-3] + cam[i-2] + cam[i-1] + cam[i] + cam[i+1] + cam[i+2] + cam[i+3] );
	}
	moy [125] = (float)0.14*(  2*cam[127] + cam[126] + cam[125] + cam[124] + cam[123] + cam[121] );
	moy [126] = (float)0.14*(  3*cam[127] + cam[126] + cam[125] + cam[124] + cam[123] );
	moy [127] = (float)0.14*(  4*cam[127] + cam[126] + cam[125] + cam[124] );
}


// plus de pepino
void moyenne_glissante(int8_t* valeurs)
{
    uint8_t i;
    int8_t previous = *valeurs;
    int8_t temp;

    *valeurs = (previous*2 + *(valeurs + 1))/3;

	for(i = 1; i < 125; i++)
    {
        temp = *(valeurs + i);
        *(valeurs + i) = (temp + previous + *(valeurs + i + 1)) / 3;
        previous = temp;
    }

    *(valeurs + 125) = ((*(valeurs + 125))*2 + previous)/3;
}
    

void milieu_ligne(uint8_t* milieu, uint8_t* incertitude)
{
    int8_t valeurs[126];
	uint8_t pos_max = 0;
	uint8_t pos_min = 0;
	uint8_t max_hors_ligne = 0;
	uint8_t i;
	
	#ifdef DEBUG_LIGNE
	TransmitCharacter(0x42); // Délimiteur pour l'affichage en python
	#endif

	// calcul de la dérivée
    for(i = 0; i < 126; i++)
   	{
        valeurs[i] = (int8_t)camera_valeurs[i + 2] - (int8_t)camera_valeurs[i];
    }
    
    valeurs[0] = 0; // hack parce que le premier pixel fait de la merde
	

	
	// recherche du min / du max
	for(i = 0; i < 126; i++)
	{
		if(valeurs[i] > valeurs[pos_max])
			pos_max = i;
		if(valeurs[i] < valeurs[pos_min])
			pos_min = i;
		
		#ifdef DEBUG_LIGNE
		TransmitCharacter(valeurs[i]);
		#endif
		
	}
    	
	*milieu =(pos_min + pos_max) / 2;
	
	// recherche du plus gros pic en dehors de la ligne (incertitude)
	for(i = 0; i < 126; i++)
	{
		if(abs((int8_t)(*milieu) - (int8_t)i) > LARGEUR_LIGNE + DELTA_LARGEUR_LIGNE && abs(valeurs[i]) > max_hors_ligne)
			max_hors_ligne = (unsigned char) (abs(valeurs[i]));
	}
	
	if(abs((int8_t)pos_min - (int8_t)pos_max - LARGEUR_LIGNE) < DELTA_LARGEUR_LIGNE)
		*incertitude = (unsigned char) ( 100 * max_hors_ligne / min(valeurs[pos_max], -valeurs[pos_min]) );
	else
		*incertitude = 250;
	
	#ifdef DEBUG_LIGNE
	TransmitCharacter(pos_min);
	TransmitCharacter(pos_max);
	TransmitCharacter(*incertitude);
	TransmitCharacter(*milieu);
	#endif

}