// Port D será responsável para detectar o pulso de echo

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/debug.h"


#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif


uint32_t larguraPulsoMicro = 0;
uint32_t nivel = 0;
uint32_t sysCounter = 0;
uint32_t difClocks = 0;
int contaBordas = 0;
int bordaSubida, bordaDescida;
int delayMicros = 20;


void confGPIO(void);
void initTimer3(void);
void initTimer1(void);
uint32_t lerNivel (void);


int main(){

    confGPIO();
    initTimer3();
    initTimer1();
    while(1){
        larguraPulsoMicro = (1000000*difClocks)/120000000;
        nivel = (larguraPulsoMicro*34300)/(2*1000000);
		if (sysCounter == 1)
			GPIO_PORTB_AHB_DATA_R &= ~(0x10);   // Pino em nível baixo
		else if (sysCounter == 2)
			GPIO_PORTB_AHB_DATA_R |= (0x10);    // Pino de trigger em nível alto
		else if (sysCounter == 3){
			TIMER3_CTL_R |= (0x1);              // Habilita o temporizador 3A
			GPIO_PORTB_AHB_DATA_R &= ~(0x10);   // Pino em nível baixo
		}
		else if (sysCounter >= 100)
			sysCounter = 0;
		}
		if (contaBordas == 1)
			bordaSubida = TIMER3_TAR_R;                                 // Registra o tempo da borda de subida
		else if (contaBordas >= 2){
			bordaDescida = TIMER3_TAR_R;
			difClocks = bordaDescida;
			TIMER3_TAV_R = 0x0;                                         // Reseta contagem
			contaBordas = 0;
			TIMER3_CTL_R &= ~0x1;                                       // Desabilita o Timer 3A
		}
    }
}

void confGPIO(void){

    SYSCTL_RCGCTIMER_R |= (0x2) | (0x8);        // Habilita o periférico do Timer bloco 1 e 3
    SYSCTL_RCGCGPIO_R |= 0x8 | 0x2;             // Habilita a porta D e B

    GPIO_PORTB_AHB_DIR_R |= (0x10);             // PB4 como saída
    GPIO_PORTB_AHB_DEN_R |= (0x10);             // PB4 como pino digital. Trigger do sensor HCSR-04

    GPIO_PORTD_AHB_DIR_R &= ~(1<<2);            // PD2 é uma entrada
    GPIO_PORTD_AHB_IS_R &= ~(1<<2);             // Sensível a bordas
    GPIO_PORTD_AHB_IBE_R = (1<<2);              // Sensível as duas bordas
    GPIO_PORTD_AHB_IM_R = (1<<2);               // Interrupção na máscara
    GPIO_PORTD_AHB_DEN_R = (1<<2);              // PD2 é um pino digital
    NVIC_EN0_R |= 1<<3;                         // Interrupção na porta D

}

void initTimer1(void){

    TIMER1_CTL_R &= ~0x1;           // Desabilita o Timer antes da inicialização
    TIMER1_CFG_R = 0x4;             // Timer 16 bits
    TIMER1_TAMR_R = 0x2;            // Modo periódico e contador decrescente
    TIMER1_TAILR_R = delayMicros;   // Intervalo do temporizador A
    TIMER1_TAPR_R = 120;            // Prescaler de 120
    TIMER1_CTL_R |= (0x1);          // Habilita o temporizador 1A
    TIMER1_IMR_R = (0x1);           // Habilita a interrupção por timeout
    NVIC_EN0_R |= (0x200000);       // Configura a interrupção na NVIC de T1 A

}

void initTimer3(void){

    TIMER3_CTL_R &= ~0x1;           // Desabilita o Timer antes da inicialização
    TIMER3_CFG_R = 0x0;             // Timer 32 bits
    TIMER3_TAMR_R = 0x12;           // Modo periódico e contador crescente
    TIMER3_TAILR_R = 0xFFFFFFFE;    // Intervalo do temporizador 3 A
    TIMER3_CTL_R |= (1<<1);         // Pausa o tempo ao pausar no debug

}

void intTimer1Handler(void){

    TIMER1_ICR_R |= 0x1;                    // Sinalizador de timeout do Timer 1 limpo
    sysCounter++;

void intPORTDHandler(void){

    GPIO_PORTD_AHB_ICR_R |= (1<<2);
    contaBordas++;
}

uint32_t lerNivel(void){

    return nivel;

}
