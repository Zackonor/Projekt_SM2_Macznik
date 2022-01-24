	
// "SAILOR" - projekt Systemy Mikroprocesorowe 2
// Wykonanie: Sylwester Mącznik
	
#include "MKL05Z4.h"
#include "pit.h"
#include "uart.h"
#include "i2c.h"
#include "frdm_bsp.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "system_MKL15Z4.h"

// Definicja liczby Pi
#define M_PI 3.14159265358979323846  

// Deklaracja zmiennych używanych w wyliczeniu kierunku zwrotu magnetometru
static char temp1[99];
static uint8_t readout = 0;
static uint8_t arrayXYZ[6];
static int16_t x;
static int16_t y;
static int16_t z;
static double heading;
static double heading_degrees;
static double declination_angle = 0.1;

// Deklaracja zmiennych używanych w określeniu prędkości wiatru
static uint16_t tpm1Diff = 0;
static uint16_t tpm1New = 0;
static uint16_t tpm1Old = 0;
static float herz;
static float us = 0.0000026;
static uint32_t interr_current;
static uint32_t interr_previous;


void TPM1_IRQHandler(void);


void TPM1_IRQHandler(void) 
	{
		interr_current++;
		tpm1Old = tpm1New;
		tpm1New = TPM1->CONTROLS[1].CnV & 0xFFFF;  
		tpm1Diff = (tpm1New - tpm1Old)/2;	/* obliczenie różnicy, uwzględnienie dwóch zboczy */
		TPM1->CONTROLS[1].CnSC |= TPM_CnSC_CHF_MASK; //usunięcie flagi
	}



void PIT_IRQHandler()
{
	
	if(interr_current == interr_previous) //wyzerowanie tpm1Diff jeżeli pomiar jest stały przez 0.5s
	{
		tpm1Diff = 0;
		interr_current = 0;
	}
		
	interr_previous = interr_current;
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;		
	NVIC_ClearPendingIRQ(PIT_IRQn); // Skasowanie flagi żądania przerwania
}


// Funkcja odczytująca odpowiednie rejestry magnetometru w celu uzyskania wartości osi X,Y,Z
void ReadMag(void)
	{
		if ((readout & 0x1) == 1)
	{
		I2C_ReadReg(0x0D, 0x0, &arrayXYZ[0]);
		I2C_ReadReg(0x0D, 0x1, &arrayXYZ[1]);
		I2C_ReadReg(0x0D, 0x2, &arrayXYZ[2]);
		I2C_ReadReg(0x0D, 0x3, &arrayXYZ[3]);
		I2C_ReadReg(0x0D, 0x4, &arrayXYZ[4]);
		I2C_ReadReg(0x0D, 0x5, &arrayXYZ[5]);
	}
	}

//Funkcja wykonująca obliczenie kierunku w jakim zwrócony jest magnetometr w stopniach 
void CalcMag(void)
{
		heading = atan2(y,x);
		heading += declination_angle;
		
		if(heading < 0)
		{
			heading = heading + (2*M_PI);
		}
		if(heading > 2*M_PI)
		{
			heading = heading - (2*M_PI);
		}
		heading_degrees = (heading*(180))/M_PI;
		

}

int main (void)
{
	
	
	PIT_Init();							// Inicjalizacja licznika PIT0
	
	// Konfiguracja TPM1 w trybie Input Capture
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;		//  Wybór maski TPM1 w rejestrze SCGC6 
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);//  Wybór zegara MCGFLLCLK 
	
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; //  Podłączenie zegara do Portu B
	PORTB->PCR[13] = PORT_PCR_MUX(2);  //  Ustawienie Multipleksera na TPM1 dla PTB13
	
	TPM1->SC |= TPM_SC_PS(7);  // Preskaler 128				
	TPM1->SC |= TPM_SC_CMOD(1);	// Counter zlicza z częstotliwością zegara MCGFLLCLK/128
	
	TPM1->SC &= ~TPM_SC_CPWMS_MASK; // Zliczanie w górę		
	TPM1->CONTROLS[1].CnSC &= ~ (TPM_CnSC_MSA_MASK | TPM_CnSC_MSB_MASK); // MSA i MSB - 0:0
	TPM1->CONTROLS[1].CnSC |= (TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK); //zbocze narastające i opadające
  
	TPM1->CONTROLS[1].CnSC |= TPM_CnSC_CHIE_MASK; 
	
	NVIC_SetPriority(TPM1_IRQn, 1);  // Ustawienie priorytetu przerwania

	NVIC_ClearPendingIRQ(TPM1_IRQn); // Skasowanie flagi
	NVIC_EnableIRQ(TPM1_IRQn);	 		// Odblokowanie przerwania
	
	
													
	UART_Init(9600);												// Inicjalizacja UART, BaudRate = 9600 
	I2C_Init();															// Inicjalizacja I2C
I2C_WriteReg(0x0D, 0xB, 0x01); // Ustawienie częstotliwości odświeżania danych przez magnetometr
I2C_WriteReg(0x0D, 0x09,0x01); // Ustawienie magnetometru w tryb pracy ciągłej
	
	while(1)
	{
		
		I2C_ReadReg (0x0D, 0x06, &readout);// Dane gotowe do odczytu
		ReadMag();
		x = arrayXYZ[0] | (arrayXYZ[1]<<8);  //Połączenie LSB i MSB rejestrów każdej z Osi
		y = arrayXYZ[2] | (arrayXYZ[3]<<8);
		z = arrayXYZ[4] | (arrayXYZ[5]<<8);
		
		CalcMag();
		DELAY(30);
		
		//Przesył informacji UART
		sprintf(temp1, "%02f, %d",heading_degrees, tpm1Diff);
		UART_Println(temp1);
		
	}
}

