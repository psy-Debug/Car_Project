#include "usart3.h"

void USART3_Init(uint32_t baudrate) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART3, &USART_InitStructure);
    USART_Cmd(USART3, ENABLE);
}

static int usart3_init(uint32_t baudrate) {
    USART3_Init(baudrate);
    return 0;
}

static void usart3_send_string(const char *str) {
    while (*str) {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3, (uint8_t)*str++);
    }
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}

static void usart3_send_bytes(const uint8_t *buf, uint16_t len) {
    uint16_t i;
    for (i = 0; i < len; i++) {
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
        USART_SendData(USART3, buf[i]);
    }
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}

static int usart3_poll_recv(uint8_t *buf, uint16_t max_len) {
    if (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != RESET) {
        buf[0] = USART_ReceiveData(USART3);
        return 1;
    }
    return 0;
}

const serial_driver_t usart3_driver = {
    .init        = usart3_init,
    .send_string = usart3_send_string,
    .send_bytes  = usart3_send_bytes,
    .poll_recv   = usart3_poll_recv,
};
