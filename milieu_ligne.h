#ifndef __MILIEU_LIGNE_H__
#define __MILIEU_LIGNE_H__

#include "MPC5604B_M27V.h"
#include "extern_globals.h"

void moyenne_glissante(int8_t* valeurs);
void milieu_ligne(uint8_t* milieu, uint8_t* incertitude);

uint8_t milieu_ligne_pepino(void);
void moyennage_cam(uint16_t cam[], float moy[]);

#endif
