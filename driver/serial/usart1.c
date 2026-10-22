#include "usart1.h"

int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (uint8_t) ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    return ch;
}

void USART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

static void usart1_send_string(const char *str)
{
    while (*str) {
        USART_SendData(USART1, (uint8_t)*str++);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }
}

static void usart1_send_bytes(const uint8_t *buf, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++) {
        USART_SendData(USART1, (uint8_t)buf[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }
}

static int usart1_poll_recv(uint8_t *buf, uint16_t max_len)
{
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
        buf[0] = USART_ReceiveData(USART1);
        return 1;
    }
    return 0;
}

static int usart1_init(uint32_t baudrate) {
    (void)baudrate;
    USART1_Config();
    return 0;
}

const serial_driver_t usart1_driver = {
    .init        = usart1_init,
    .send_string = usart1_send_string,
    .send_bytes  = usart1_send_bytes,
    .poll_recv   = usart1_poll_recv,
};
