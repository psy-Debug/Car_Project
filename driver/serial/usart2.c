#include "usart2.h"
#include "protocol.h"
#include <string.h>

static uint8_t rx_fifo[64];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;
static volatile uint8_t rx_count = 0;

void USART2_Config(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART2, ENABLE);
}

void USART2_IRQHandler(void) {
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        uint8_t data = USART_ReceiveData(USART2);
        protocol_feed_byte(data);
        if (rx_count < 64) {
            rx_fifo[rx_head] = data;
            rx_head = (rx_head + 1) % 64;
            rx_count++;
        }
    }
}

static int usart2_init(uint32_t baudrate) {
    USART2_Config(baudrate);
    return 0;
}

static void usart2_send_string(const char *str) {
    while (*str) {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        USART_SendData(USART2, (uint8_t)*str++);
    }
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
}

static void usart2_send_bytes(const uint8_t *buf, uint16_t len) {
    uint16_t i;
    for (i = 0; i < len; i++) {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
        USART_SendData(USART2, buf[i]);
    }
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
}

static int usart2_poll_recv(uint8_t *buf, uint16_t max_len) {
    int copied = 0;
    __disable_irq();
    while (rx_count > 0 && copied < (int)max_len) {
        buf[copied++] = rx_fifo[rx_tail];
        rx_tail = (rx_tail + 1) % 64;
        rx_count--;
    }
    __enable_irq();
    return copied;
}

const serial_driver_t usart2_driver = {
    .init        = usart2_init,
    .send_string = usart2_send_string,
    .send_bytes  = usart2_send_bytes,
    .poll_recv   = usart2_poll_recv,
};
