#include <LowPower.h>
 
//int led 13;
int minutos=1;

void setup()
{
// No setup is required for this library
}

void loop() 
{
    ///el tiempo maximo que la libreria permite poner el Arduino en modo sleep es
    //de 8seg, por lo que se hacen varios ciclos para lograr los minutos requeridos
    //(por ej: para 10 minutos se hacen 10*60/8=75 ciclos) 
    int cont=(minutos*60)/8;
    for (int i = 0; i < cont; i++) { 
    //idle(period_t period, adc_t adc, timer2_t timer2,timer1_t timer1, timer0_t timer0,spi_t spi, usart0_t usart0,twi_t twi
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
    }
}
