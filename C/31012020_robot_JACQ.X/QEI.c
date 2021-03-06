#include <stdio.h>
#include <stdlib.h>
#include <p33EP512GM706.h>
#include <math.h>
#include "QEI.h"
#include "Robot.h"
#include "IO.h"
#include "UART_protocole.h"
#include "Utilities.h"
#include "timer.h"
#include "Send.h"

//#define DISTROUES 0.2812
//#define PI 3.014159265358979323846264338327950288419716939937510582
#define FREQ_ECH_QEI 250


double QeiDroitPosition_T_1, QeiDroitPosition, QeiGauchePosition_T_1, QeiGauchePosition, delta_d, delta_g, delta_theta, dx, QEI2RawValue;
int compteur = 0;


void InitQEI1()
    {
        QEI1IOCbits.SWPAB = 1; //QEAx and QEBx are swapped
        QEI1GECL = 0xFFFF;
        QEI1GECH = 0xFFFF;
        QEI1CONbits.QEIEN = 1; // Enable QEI Module
    }

void InitQEI2() 
    {
        QEI2IOCbits.SWPAB = 1; //QEAx and QEBx a r e not swapped
        QEI2GECL = 0xFFFF;
        QEI2GECH = 0xFFFF;
        QEI2CONbits.QEIEN = 1; // Enable QEI Module
    }

void QEIUpdateData()
    {
        // on sauvgarde les ancinees valeurs 
        QeiDroitPosition_T_1 = QeiDroitPosition;
        QeiGauchePosition_T_1 = QeiGauchePosition;

        //On r�actualise les valeurs des pos i t i o n s
        long QEI1RawValue =POS1CNTL;
        QEI1RawValue+=((long)POS1HLD<<16);

        long QEI2RawValue = POS2CNTL;
        QEI2RawValue += ((long)POS2HLD<<16);
        // Conversion en mm ( r\? e g l \ ? e pour l a t a i l l e de s r o u e s c o d e u s e s )
        QeiDroitPosition = POINT_TO_METER*QEI1RawValue ;
        QeiGauchePosition = -POINT_TO_METER*QEI2RawValue;

        //calcul des delta
        delta_d = QeiDroitPosition - QeiDroitPosition_T_1; // Vd
        delta_g = QeiGauchePosition - QeiGauchePosition_T_1; //Vg

        //calcul de theta
        delta_theta = (delta_d - delta_g)/(DISTROUES); //  vitesse angulaire
        dx =(delta_d + delta_g)/2; // Vl

        //calcul des vitesses
        robotState.vitesseDroitFromOdometry = delta_d*FREQ_ECH_QEI;
        robotState.vitesseGaucheFromOdometry = delta_g*FREQ_ECH_QEI;


        robotState.vitesseLineaireFromOdometry = (robotState.vitesseDroitFromOdometry + robotState.vitesseGaucheFromOdometry)/2 ;
        robotState.vitesseAngulaireFromOdometry = delta_theta*FREQ_ECH_QEI;

        //Mise � jour du postionnement  terrain � t-1
        robotState.xPosFromOdometry_1 = robotState.xPosFromOdometry ;
        robotState.yPosFromOdometry_1 = robotState.yPosFromOdometry ;
        robotState.angleRadianFromOdometry_1 = robotState.angleRadianFromOdometry ;

        //// Calcul des points dans les referentiel du terrain
        robotState.xPosFromOdometry = robotState.xPosFromOdometry_1 + (robotState.vitesseLineaireFromOdometry/FREQ_ECH_QEI) * cos(robotState.angleRadianFromOdometry_1);
        robotState.yPosFromOdometry = robotState.yPosFromOdometry_1 + (robotState.vitesseLineaireFromOdometry/FREQ_ECH_QEI) * sin(robotState.angleRadianFromOdometry_1);
        robotState.angleRadianFromOdometry = robotState.angleRadianFromOdometry_1 + delta_theta;//robotState.vitesseAngulaireFromOdometry;
        if ( robotState.angleRadianFromOdometry > PI )
            robotState.angleRadianFromOdometry -= 2*PI ;
        if ( robotState.angleRadianFromOdometry < -PI )
            robotState.angleRadianFromOdometry += 2*PI ;
        
        compteur++;
        if(compteur == 25)
        {
            compteur = 0;
            SendPositionData();
            SendVitesseData();
        }
    }     
            
 



