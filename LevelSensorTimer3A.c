#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include "inc/tm4c1294ncpdt.h"


uint32_t larguraPulsoMicro = 0;
uint32_t nivel = 0;
uint32_t sysCounter = 0;
uint32_t difClocks;
int contaBordas = 0;
int bordaSubida, bordaDescida;
int delayMicros = 10;                 // Delay em microssegundos

void confGPIO(void);
void initTimer1(void);
void initTimer3(void);
uint32_t lerNivel (void);


int main(){

    confGPIO();
    initTimer3();
    initTimer1();
    while(1){
		larguraPulsoMicro = (1000000*difClocks)/120000000;
        nivel = (larguraPulsoMicro*34300)/(2*1000000);
		if (sysCounter == 1)
        GPIO_PORTB_AHB_DATA_R &= ~(1<<4);   // Pino em nível baixo
		else if (sysCounter == 2)
			GPIO_PORTB_AHB_DATA_R |= (1<<4);    // Pino de trigger em nível alto
		else if (sysCounter == 3)
			GPIO_PORTB_AHB_DATA_R &= ~(1<<4);   // Pino em nível baixo
		else if (sysCounter >= 100)
			sysCounter = 0;
		if (contaBordas == 1){
			TIMER3_CTL_R |= (1<<0);                     // Habilita o timer 3A
			bordaSubida = TIMER3_TAR_R;                 // Registra o tempo da borda de subida
		}
		else if (contaBordas >= 2){
			bordaDescida = TIMER3_TAR_R;
			TIMER3_CTL_R &= ~(1<<0);                    // Desabilita o timer 3A
			difClocks = bordaDescida - bordaSubida;
			TIMER3_TAV_R = 0x0;
			contaBordas = 0;
		}
    }
}


void confGPIO(void){

    SYSCTL_RCGCTIMER_R |= (1<<1) | (1<<3);      // Habilita o periférico do Timer bloco 1 e 3
    SYSCTL_RCGCGPIO_R |= 0x8;                   // Habilita a porta D

    SYSCTL_RCGCGPIO_R |= 0x2;                   // Habilita o clock da porta B
    GPIO_PORTB_AHB_DIR_R |= (1<<4);             // PB4 como saída
    GPIO_PORTB_AHB_DEN_R |= (1<<4);             // PB4 como pino digital. Trigger do sensor HCSR-04

    GPIO_PORTD_AHB_DIR_R &= ~(1<<2);            // PD2 é uma entrada
    GPIO_PORTD_AHB_AFSEL_R |= (1<<2);           // PD2 é uma função alternativa
    GPIO_PORTD_AHB_DEN_R |= (1<<2);             // PD2 é um pino digital
    GPIO_PORTD_AHB_PCTL_R &= ~0xF0000;          // Limpa o PD2 no PCTL da função desejada
    GPIO_PORTD_AHB_PCTL_R |= 0x30000;           // Configura PD2 para T3CCP0 (Timer 3A)

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

    TIMER3_CTL_R &= ~(1<<0);        // Desabilita o timer 3A
    TIMER3_CFG_R = 0x0;             // 32-bit modo timer
    TIMER3_TAMR_R = 0x13;           // up-count, edge-time e capture mode
    TIMER3_CTL_R |= (0xC);          // Captura a borda de subida ou descida no pino PD4
    TIMER3_IMR_R = 0x4;             // Habilita a interrupção por modo de captura
    TIMER3_TAILR_R = 0xFFFFFFFF;    // Contagem máxima
    NVIC_EN1_R |= (1<<3);           // Interrupção do Timer 3A na NVIC

}

void intTimer1Handler(void){

    TIMER1_ICR_R |= 0x1;                    		// Sinalizador de timeout do Timer 1 limpo
    sysCounter++;
}

void intTimer3Handler(void){

    TIMER3_ICR_R |= 0x1;                            // Limpa a interrupção do Timer 3A
    contaBordas++;
}

uint32_t lerNivel(void){

    return nivel;

}
