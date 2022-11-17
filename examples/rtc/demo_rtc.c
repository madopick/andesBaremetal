#include <stdio.h>
#include "Driver_RTC.h"
#include "Driver_USART.h"
#include <nds32_intrinsic.h>

// RTC demo flag
typedef enum _DEMO_RTC_FLAG_ID
{
    DEMO_RTC_NONE,
    DEMO_RTC_ALARM_INT_DONE,
    DEMO_RTC_DAY_INT_DONE,
    DEMO_RTC_HOUR_INT_DONE,
    DEMO_RTC_MIN_INT_DONE,
    DEMO_RTC_SEC_INT_DONE,
    DEMO_RTC_HSEC_INT_DONE,
}DEMO_RTC_FLAG_ID;

// volatile is necessary for while(!(g_Demo_RTC_Flag == eRTC_DEMO_SEC_INT_DONE));
volatile uint32_t g_Demo_RTC_Flag = DEMO_RTC_NONE;
extern NDS_DRIVER_RTC Driver_RTC;
extern NDS_DRIVER_USART Driver_USART1;

#define START_STR          ("\n\r=====Start demo RTC=====")
#define SEC_INT_DONE_STR   ("\n\rSecond interrupt done.....")
#define ALARM_DELTA_3S_STR ("\n\rAfter 3 seconds elapsed, you will see alarm interrupt done msg.....")
#define ALARM_INT_DONE_STR ("\n\rAlarm interrupt done.....")
#define ALARM_DELTA_3S     (3)

// UART Print() unable to be used in RTC ISR since RTC interrupt priority level is the highest
#define Print(s)           do                                               \
                           {                                                \
                                pDrv_USART->Send(s, sizeof(s)/sizeof(char));\
                                wait_uart_complete();                       \
                           } while(0);

static volatile int uart_event_complete = 0;

static void uart_callback(uint32_t event)
{
    switch (event)
    {
    case NDS_USART_EVENT_RECEIVE_COMPLETE:
    case NDS_USART_EVENT_TRANSFER_COMPLETE:
    case NDS_USART_EVENT_SEND_COMPLETE:
    case NDS_USART_EVENT_TX_COMPLETE:
        // Success: continue task
        uart_event_complete = 1;
        break;
    default:
        while(1);
    }

}

static void wait_uart_complete(void)
{
    while (!uart_event_complete);
    uart_event_complete = 0;
}

void Alarm_Int()
{
    NDS_DRIVER_RTC* pDrv_RTC = &Driver_RTC;

    // disable alarm interrupt
    pDrv_RTC->Control(NDS_RTC_CTRL_ALARM_INT, 0);

    g_Demo_RTC_Flag = DEMO_RTC_ALARM_INT_DONE;
}

void Day_Int()
{
    NDS_DRIVER_RTC* pDrv_RTC = &Driver_RTC;

    // disable day interrupt
    pDrv_RTC->Control(NDS_RTC_CTRL_DAY_INT, 0);

    g_Demo_RTC_Flag = DEMO_RTC_DAY_INT_DONE;
}

void Hour_Int()
{
    NDS_DRIVER_RTC* pDrv_RTC = &Driver_RTC;

    // disable hour interrupt
    pDrv_RTC->Control(NDS_RTC_CTRL_HOUR_INT, 0);

    g_Demo_RTC_Flag = DEMO_RTC_HOUR_INT_DONE;
}

void Min_Int()
{
    NDS_DRIVER_RTC* pDrv_RTC = &Driver_RTC;

    // disable MIN interrupt
    pDrv_RTC->Control(NDS_RTC_CTRL_MIN_INT, 0);

    g_Demo_RTC_Flag = DEMO_RTC_MIN_INT_DONE;
}

void Sec_Int()
{
    NDS_DRIVER_RTC* pDrv_RTC = &Driver_RTC;

    // disable second interrupt
    pDrv_RTC->Control(NDS_RTC_CTRL_SEC_INT, 0);

    g_Demo_RTC_Flag = DEMO_RTC_SEC_INT_DONE;
}

void Hsec_Int()
{
    NDS_DRIVER_RTC* pDrv_RTC = &Driver_RTC;

    // disable half-second interrupt
    pDrv_RTC->Control(NDS_RTC_CTRL_HSEC_INT, 0);

    g_Demo_RTC_Flag = DEMO_RTC_HSEC_INT_DONE;
}

void My_RTC_Callback(uint32_t event)
{
    // callback function body
    if(event & NDS_RTC_EVENT_ALARM_INT)
    {
        Alarm_Int();
    }

    if(event & NDS_RTC_EVENT_DAY_INT)
    {
        Day_Int();
    }

    if(event & NDS_RTC_EVENT_HOUR_INT)
    {
        Hour_Int();
    }

    if(event & NDS_RTC_EVENT_MIN_INT)
    {
        Min_Int();
    }

    if(event & NDS_RTC_EVENT_SEC_INT)
    {
        Sec_Int();
    }

    if(event & NDS_RTC_EVENT_HSEC_INT)
    {
        Hsec_Int();
    }
}

void USART_Init(NDS_DRIVER_USART* pDrv_USART)
{
    if(!pDrv_USART)
    {
        return;
    }

    // Initialize the USART driver
    pDrv_USART->Initialize(uart_callback);

    // Power up the USART peripheral
    pDrv_USART->PowerControl(NDS_POWER_FULL);

    // Configure the USART to 38400 Bits/sec
    pDrv_USART->Control(NDS_USART_MODE_ASYNCHRONOUS |
                      NDS_USART_DATA_BITS_8 |
                      NDS_USART_PARITY_NONE |
                      NDS_USART_STOP_BITS_1 |
                      NDS_USART_FLOW_CONTROL_NONE, 38400);

    // Enable Transmitter lines
    pDrv_USART->Control (NDS_USART_CONTROL_TX, 1);
}

int main(void)
{
    NDS_DRIVER_RTC*     pDrv_RTC = &Driver_RTC;
    NDS_DRIVER_USART*   pDrv_USART = &Driver_USART1;
    NDS_RTC_TIME        rRTC_Time = {0,0,0,0};
    NDS_RTC_ALARM       rRTC_Alarm = {0,0,0};

    USART_Init(pDrv_USART);

    Print(START_STR);

    // start demo
    g_Demo_RTC_Flag = DEMO_RTC_NONE;

    if(pDrv_RTC)
    {
        // demo second interrupt
        pDrv_RTC->Initialize(My_RTC_Callback);
        pDrv_RTC->PowerControl(NDS_POWER_FULL);
        pDrv_RTC->SetTime(&rRTC_Time, 0);
        // enable second interrupt
        pDrv_RTC->Control(NDS_RTC_CTRL_SEC_INT, 1);
        // enable RTC
        pDrv_RTC->Control(NDS_RTC_CTRL_EN, 1);

        // sim delay
        while(!(g_Demo_RTC_Flag == DEMO_RTC_SEC_INT_DONE));
        Print(SEC_INT_DONE_STR);

        Print(ALARM_DELTA_3S_STR);

        // demo alarm interrupt
        pDrv_RTC->PowerControl(NDS_POWER_FULL);
        // get current RTC time
        pDrv_RTC->GetTime(&rRTC_Time, 0);
        rRTC_Alarm.hour = rRTC_Time.hour;
        rRTC_Alarm.min= rRTC_Time.min;
        rRTC_Alarm.sec= rRTC_Time.sec + ALARM_DELTA_3S;
        pDrv_RTC->SetAlarm(&rRTC_Alarm, 0);
        // enable alarm interrupt,as rRTC_Time count to rRTC_Alarm
        pDrv_RTC->Control(NDS_RTC_CTRL_ALARM_INT, 1);
        // enable RTC
        pDrv_RTC->Control(NDS_RTC_CTRL_EN, 1);

        while(!(g_Demo_RTC_Flag == DEMO_RTC_ALARM_INT_DONE));
        Print(ALARM_INT_DONE_STR);

        // exit demo
        pDrv_RTC->Uninitialize();
    }
    else
    {
        // pDrv_RTC is NULL
    }

    return 0;
}
