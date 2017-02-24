#include "driverlib.h"
#include "mymsp430.h"
#include "string.h"

volatile uint8_t receive_data[20],receive_index=0;
volatile bool  transmit_done = false;
uint8_t transmit_data[]="Hello World. TeT Holiday\r\n";
volatile uint32_t mclk=0,smclk=0,aclk=0;
void Uart_Init(void);
void delay(uint32_t time);


#pragma vector=USCI_A1_VECTOR
__interrupt void UART_Isr(void)
{
	if(USCI_A_UART_getInterruptStatus(USCI_A1_BASE,
			USCI_A_UART_RECEIVE_INTERRUPT_FLAG))
	{
		receive_data[receive_index]=USCI_A_UART_receiveData(USCI_A1_BASE);
		USCI_A_UART_clearInterrupt(USCI_A1_BASE,USCI_A_UART_RECEIVE_INTERRUPT_FLAG);
		if(receive_data[receive_index]=='\n')
		{
			__no_operation();
			for(uint8_t i=0;i<20;i++)
			{
				receive_data[i]=0;
			}
			receive_index=0;
		}
		else
		{
			receive_index++;
		}
	}
	if(USCI_A_UART_getInterruptStatus(USCI_A1_BASE,
			USCI_A_UART_TRANSMIT_INTERRUPT_FLAG))
	{
		transmit_done = true;
		USCI_A_UART_clearInterrupt(USCI_A1_BASE,USCI_A_UART_TRANSMIT_INTERRUPT_FLAG);
	}
}



void main( void )
{
	/* Stop watchdog timer */
	WDT_A_hold(WDT_A_BASE);

	Clk_Using_DCO_Init(20000,4000,SMCLK_CLOCK_DIVIDER_1);

	mclk=UCS_getMCLK();
	smclk=UCS_getSMCLK();
	aclk=UCS_getACLK();


	Uart_Init();

	__enable_interrupt();

	while(1)
	{
		for(uint8_t i=0;i<strlen((char const*)transmit_data);i++)
		{
			transmit_done = false;
			USCI_A_UART_transmitData(USCI_A1_BASE,*(transmit_data+i));
			while(!transmit_done);
		}
		delay(1000000);
	}
}

void Uart_Init(void)
{
	USCI_A_UART_initParam para;

	GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN5);
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P4,GPIO_PIN4);


	/* UART clock frewuency = SMCLK = 12E6
	 * Baudrate = 115200 => prescaler = UCBRx = 6
	 * UCBRFx = 8, UCBRSx = 0, UCOS16 =1.
	 *
	 * From: Table 36-5. Commonly Used Baud Rates, Settings, and Errors, UCOS16 = 1 at Page 953/1189 User guide.
	 */
	para.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK; // 12E6 Hz
	para.clockPrescalar = 4; // UCBRx = 4
	para.firstModReg = 3; //UCBRFx = 3
	para.secondModReg = 5; // UCBRSx = 5
	para.parity = USCI_A_UART_NO_PARITY;
	para.msborLsbFirst = USCI_A_UART_LSB_FIRST;
	para.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
	para.uartMode = USCI_A_UART_MODE;
	para.overSampling = USCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;//UCOS16=1

	USCI_A_UART_init(USCI_A1_BASE,&para);
	USCI_A_UART_enable(USCI_A1_BASE);
	USCI_A_UART_clearInterrupt(USCI_A1_BASE,USCI_A_UART_RECEIVE_INTERRUPT_FLAG|USCI_A_UART_TRANSMIT_INTERRUPT_FLAG);
	USCI_A_UART_enableInterrupt(USCI_A1_BASE,USCI_A_UART_RECEIVE_INTERRUPT|USCI_A_UART_TRANSMIT_INTERRUPT);

}

void delay(uint32_t time)
{
	while(time--);
}
