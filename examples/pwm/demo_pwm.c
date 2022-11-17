#include "Driver_PWM.h"
#include "Driver_USART.h"

#include <nds32_intrinsic.h>

#define PWM_OUPUT_CHANNEL	1

#define NOTE_DO                 262  //523
#define NOTE_RE                 294  //587
#define NOTE_MI                 330  //659
#define NOTE_FA                 349  //698
#define NOTE_SO                 392  //784
#define NOTE_LA                 440  //880
#define NOTE_SE                 494  //988

/* PWM Driver */
extern NDS_DRIVER_PWM Driver_PWM0;
/* UART Driver */
extern NDS_DRIVER_USART Driver_USART1;

static const unsigned int tones[] = {
    NOTE_DO, NOTE_RE, NOTE_MI, NOTE_FA, NOTE_SO, NOTE_LA, NOTE_SE
};

static volatile char usart_event_complete = 0;

static void wait_complete(void)
{
    while (!usart_event_complete);
    usart_event_complete = 0;
}

void myUSART_callback(uint32_t event)
{
    switch (event) {
    case NDS_USART_EVENT_RECEIVE_COMPLETE:
    case NDS_USART_EVENT_TRANSFER_COMPLETE:
    case NDS_USART_EVENT_SEND_COMPLETE:
    case NDS_USART_EVENT_TX_COMPLETE:
        /* Success: continue task */
        usart_event_complete = 1;
        break;

    case NDS_USART_EVENT_RX_TIMEOUT:
        while(1);
        break;

    case NDS_USART_EVENT_RX_OVERFLOW:
    case NDS_USART_EVENT_TX_UNDERFLOW:
        while(1);
        break;
    }
}

/* Application entry function */
int main(void)
{
    static NDS_DRIVER_PWM *PWMdrv = &Driver_PWM0;
    static NDS_DRIVER_USART *USARTdrv = &Driver_USART1;
    char pwm_num = PWM_OUPUT_CHANNEL;
    char note;

#ifdef DEBUG
    NDS_DRIVER_VERSION     version;
    NDS_PWM_CAPABILITIES drv_capabilities;

    version = PWMdrv->GetVersion();
    if (version.api < 0x200)   /* requires at minimum API version 2.00 or higher */
    {                          /* error handling */
        return;
    }
    drv_capabilities = PWMdrv->GetCapabilities();
    if (drv_capabilities.channels == 0)
    {                          /* error handling */
        return;
    }
#endif

    /*Initialize the USART driver */
    USARTdrv->Initialize(myUSART_callback);
    /*Power up the USART peripheral */
    USARTdrv->PowerControl(NDS_POWER_FULL);

    /*Configure the USART to 38400 Bits/sec */
    USARTdrv->Control(NDS_USART_MODE_ASYNCHRONOUS |
                      NDS_USART_DATA_BITS_8 |
                      NDS_USART_PARITY_NONE |
                      NDS_USART_STOP_BITS_1 |
                      NDS_USART_FLOW_CONTROL_NONE, 38400);

    /* Enable Receiver and Transmitter lines */
    USARTdrv->Control (NDS_USART_CONTROL_TX, 1);
    USARTdrv->Control (NDS_USART_CONTROL_RX, 1);


    USARTdrv->Send("\n\rPWM Demo", 11);
    wait_complete();

    /*Initialize the PWM driver */
    PWMdrv->Initialize(NULL);
    /*Power up the PWM peripheral */
    PWMdrv->PowerControl(NDS_POWER_FULL);
    /*Active the PWM outout channel */
    PWMdrv->Control(NDS_PWM_ACTIVE_CONFIGURE | NDS_PWM_PARK_LOW | NDS_PWM_CLKSRC_EXTERNAL, pwm_num);

    USARTdrv->Send("\n\rLet's play piano...", 21);
    wait_complete();

    USARTdrv->Send("\n\r[1 2 3 4 5 6 7] ==> Play [Do Re Mi Fa So La Si]", 49);
    wait_complete();

    USARTdrv->Send("\n\r[Space]         ==> Mute", 26);
    wait_complete();

    USARTdrv->Send("\n\r[Enter]         ==> Quit", 26);
    wait_complete();

    while(1) {
        USARTdrv->Receive(&note, 1);         /* Get byte from UART */
   	wait_complete();

        if (note == 13) {                    /* CR, send greeting  */
            PWMdrv->PowerControl(NDS_POWER_OFF);
            USARTdrv->Send("\n\rQUIT!", 7);
            wait_complete();
	    break;
        } else if (note > 48 && note < 56) { /* 1 ~ 7, play note */
	    PWMdrv->SetFreq(pwm_num, tones[note - 49]);
            PWMdrv->Output(pwm_num, 255/2);  /* Output 50% duty cycle */
	} else if (note == 32) {             /* Space, mute */
            PWMdrv->Output(pwm_num, 0);      /* Output 0% duty cycle to disable */
	}
    }

    return 0;
}
