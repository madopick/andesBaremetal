#include "Driver_WDT.h"
#include "Driver_USART.h"

#include <nds32_intrinsic.h>
#include "timer.h"

#include <stdio.h>
#include <string.h>

extern NDS_DRIVER_WDT Driver_WDT;
extern NDS_DRIVER_USART Driver_USART1;

volatile unsigned int counter = 0;
NDS_WDT_CAPABILITIES drv_capabilities;

/* Timer functions by Timer firmware lib */

#define TIMER1 0
#define TIMER2 1
#define TIMER3 2
#define TIMER4 3

static void set_timer_irq_period(unsigned int timer, int msec) {
    unsigned int period = msec_to_tick(msec);
    timer_set_period(timer, period);
    timer_irq_enable(timer);
    timer_start(timer);
}

/* USART functions by AMSI USART driver */

static volatile char usart_event_complete = 0;

static void usart_wait_complete(void) {
    while (!usart_event_complete);
    usart_event_complete = 0;
}

void myUSART_callback(uint32_t event) {
    switch (event) {
    case NDS_USART_EVENT_RECEIVE_COMPLETE:
    case NDS_USART_EVENT_TRANSFER_COMPLETE:
    case NDS_USART_EVENT_SEND_COMPLETE:
    case NDS_USART_EVENT_TX_COMPLETE:
        /* Success: continue task */
        usart_event_complete = 1;
        break;

    case NDS_USART_EVENT_RX_TIMEOUT:
        while (1);
        break;

    case NDS_USART_EVENT_RX_OVERFLOW:
    case NDS_USART_EVENT_TX_UNDERFLOW:
        while (1);
        break;
    }
}

void usart_init() {
    static NDS_DRIVER_USART *USARTdrv = &Driver_USART1;

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
}

// blocking output API by AMSI USART driver.

// static void usart_putc(char c) {
//     static NDS_DRIVER_USART *USARTdrv = &Driver_USART1;
//     if (USARTdrv->Send(&c, 1) == NDS_DRIVER_OK) {
//         usart_wait_complete();
//     }
// }

static void usart_puts(const char* str) {
    static NDS_DRIVER_USART *USARTdrv = &Driver_USART1;

    int len = strlen(str);
    if (USARTdrv->Send(str, len) == NDS_DRIVER_OK) {
        usart_wait_complete();
    }
}

void usart_print_int(unsigned int num) {
    char buf[8 * sizeof(unsigned int) + 1];
    char *str = &buf[sizeof(buf) - 1];

    *str = '\0';

    do {
        unsigned int digit = num % 10;
        num /= 10;
        *--str = digit + '0';
    } while (num);

    usart_puts(str);
}

/* End of USART functions */

/* Interrupt handlers */
void wdt_handler (uint32_t event) {
    usart_puts("WDT activated\r\n");
}

void timer_irq_handler(void) {
    timer_irq_clear(TIMER1);

    static NDS_DRIVER_WDT *WDTdrv = &Driver_WDT;
    WDTdrv->RestartTimer();

    counter++;

    return;
}

/* Application entry function */
int main(void) {
    static NDS_DRIVER_WDT *WDTdrv = &Driver_WDT;

#ifdef DEBUG
    NDS_DRIVER_VERSION version;

    version = WDTdrv->GetVersion();
    if (version.api < 0x200)   /* requires at minimum API version 2.00 or higher */
    {                          /* error handling */
        return;
    }
#endif

    drv_capabilities = WDTdrv->GetCapabilities();

    usart_init();
    timer_init();

    usart_puts("\r\nIt's an AMSI WDT demo\r\n\r\n");

    if (drv_capabilities.external_timer) {
        usart_puts("WDT will reset whole system if WDT doesn't restarted in 1 secs.\r\n");
        usart_puts("Timer restart WDT every 0.5 secs, so WDT doesn't reset system.\r\n\r\n");
    } else {
        usart_puts("WDT will reset whole system if WDT doesn't restarted in 1.5 msecs.\r\n");
        usart_puts("Timer restart WDT every 1 msec, so WDT doesn't reset system.\r\n\r\n");
    }


    if (drv_capabilities.external_timer) {
        set_timer_irq_period(TIMER1, 500); /* 500 ms = 0.5 sec */
    } else {
        set_timer_irq_period(TIMER1, 1);   /*   1 ms           */
    }

    if (drv_capabilities.irq_stage) {
        WDTdrv->Initialize(wdt_handler);
    } else {
        WDTdrv->Initialize(NULL);
    }

    if (drv_capabilities.external_timer) {
        /* Under external clock rate, pow(2, 15) ticks are 1 sec. */
        WDTdrv->Control(NDS_WDT_CLKSRC_EXTERNAL, NDS_WDT_TIME_POW_2_15);
    } else {
        /* Under APB clock rate in AE100, pow(2, 15) ticks are nearly 1.5 msecs. */
        WDTdrv->Control(NDS_WDT_CLKSRC_APB, NDS_WDT_TIME_POW_2_15);
    }

    WDTdrv->Enable();
    WDTdrv->RestartTimer();

    int factor, limit = 10;
    if (drv_capabilities.external_timer) {
        factor = 1;
    } else {
        factor = 1000;
    }
    int print_times = 0;
    int disable = 0;

    while (1) {
        if ((counter/factor) > print_times) {
            unsigned int local_counter = counter;
            usart_puts("Timer restart WDT (");
            usart_print_int(local_counter);
            usart_puts(" times), system still alive.\r\n");

            print_times++;
        }

        if (print_times >= limit && !disable) {
            usart_puts("\r\nThen, We disable Timer, so the whole system will be reset by WDT.\r\n\r\n");
            timer_irq_disable(TIMER1);

            disable = 1;
        }
    }

    return 0;
}
