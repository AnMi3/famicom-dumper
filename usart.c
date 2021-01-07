#include "defines.h"
#include <avr/io.h>
#include <util/delay.h>
#include "fifo.h"
#include <avr/interrupt.h>

extern volatile FIFO(1024) uart_tx_fifo;
extern volatile FIFO(1024) uart_rx_fifo;

void USART_init(void)
{
  unsigned int bd = (F_CPU / (16UL * UART_BAUD)) - 1;
  UBRR0L = bd & 0xFF;
  UBRR0H = bd >> 8;

  UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0); /* tx/rx enable */
//  UCSRC = 1<<URSEL|1<<UCSZ0|1<<UCSZ1;
  UCSR0C |= /*_BV(UMSEL0) |*/ _BV(UCSZ01) | _BV(UCSZ00);
//  UCSR0A = _BV(U2X);
}

int USART_TransmitByte( unsigned char data )
{
//	/* Wait for empty transmit buffer */
//	while ( !( UCSR0A & (1<<UDRE0)) );
//	/* Put data into buffer, sends the data */
//	UDR0 = data;


  int ret;
  cli(); //запрещаем прерывания
  if( !FIFO_IS_FULL( uart_tx_fifo ) ) {
    //если в буфере есть место, то добавляем туда байт
    FIFO_PUSH( uart_tx_fifo, data );
    //и разрешаем прерывание по освобождению передатчика
    UCSR0B |= ( 1 << UDRIE0 );
    ret = 0;
  }
  else {
    ret = -1; //буфер переполнен
  }
  sei(); //разрешаем прерывания
  return ret;

}

void USART_TransmitHex( unsigned char data )
{
	unsigned char h = data>>4;
	char ho = (h < 10) ? (h+'0') : (h+'A'-10);
	unsigned char l = data & 0xF;
	char lo = (l < 10) ? (l+'0') : (l+'A'-10);
	USART_TransmitByte(ho);
	USART_TransmitByte(lo);
//	while ( !( UCSR0A & (1<<UDRE0)) );
//	UDR0 = ho;
//	while ( !( UCSR0A & (1<<UDRE0)) );
//	UDR0 = lo;
}

void USART_TransmitText(char* data)
{
	while (*data != 0)
	{
		/* Wait for empty transmit buffer */
		while ( !( UCSR0A & (1<<UDRE0)) );
		/* Put data into buffer, sends the data */
		USART_TransmitByte(*data);
		//UDR0 = *data;
		data++;
	}
}

void USART_Transmit(void* p, unsigned long int len)
{
	unsigned char* buff = (unsigned char*)p;
	unsigned long int b;
	for (b = 0; b < len; b++) USART_TransmitByte(buff[b]);
}
