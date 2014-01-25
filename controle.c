#include "controle.h"
#include "constantes.h"
#include "milieu_ligne.h"
#include "liaison_serie.h"
#include "extern_globals.h"

#include <math.h>

#define max(x,y) (x<y?y:x)

void Controle_Direction(uint8_t milieu_ligne) {

   uint8_t centre_camera = 64; //centre camera
   int16_t derivee;
   objectif_vitesse = 0.5;

   /* coefs du PD */
   int16_t Kp = 1; // coef proportionnel
   int16_t Kd = 10; // coef dérivé

   /* Calcul de l'erreur en fontion du milieu de la ligne */
   controle_derniere_erreur = erreur;
   int16_t erreur = centre_camera - milieu_ligne;

   /* calcul de la dérivée de l'erreur */
   derivee = erreur - controle_derniere_erreur;

   /********************************************************/
   /* application du PD pour l'orientation des roues */
   float commande = pos_milieu_servo;

   float terme_derivee = Kd * derivee;
   /* limitation du terme dérivée */
   if (terme_derivee < -1000){
      terme_derivee = -1000;
   } else if (terme_derivee > 1000) {
      terme_derivee = 1000;
   }
   commande += terme_derivee;

   /********************************************************/

   /* limitation des commandes servo */
   if (commande < pos_min_servo) {
      commande_bornee = pos_min_servo;
   } else if (commande > pos_max_servo) {
      commande_bornee = pos_max_servo;
   } else {
      commande_bornee = (uint16_t) commande;
   }

   /* modif de la vitesse en fonction de la derivée */
   objectif_vitesse = 0.7 - 0.05*(abs(derivee));

   /* application de la commande */
   EMIOS_0.CH[4].CBDR.R = commande_bornee;
}
