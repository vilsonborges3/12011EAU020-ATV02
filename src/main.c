#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define LED_DELAY 75000
#define LED_PIN 13
#define BUTTON 0
/* changes to the F103 */

#define STM32_RCC_BASE 0x40021000     /* 0x40021000 - 0x400213ff: Reset and Clock control RCC */

/* AHB2 Base Addresses ******************************************************/

#define STM32_GPIOC_BASE 0x40011000     /* 0x40011000 - 0x400113ff: GPIO Port C */
#define STM32_GPIOA_BASE 0x40010800      /* 0x40010800 - 0x40010bff: GPIO Port A */

/* Register Offsets *********************************************************/
#define STM32_RCC_APB2ENR_OFFSET    0x0018  /* APB2 peripheral clock enable register */
#define STM32_GPIO_CRL_OFFSET       0x0000  /* Port configuration register low */
#define STM32_GPIO_CRH_OFFSET       0x0004  /* Port configuration register high */
#define STM32_GPIO_BSRR_OFFSET      0x0010  /* Port bit set/reset register */
#define STM32_GPIO_IDR_OFFSET       0x0008  /* Port input data register */



/* Register Addresses *******************************************************/
#define STM32_RCC_APB2ENR               (STM32_RCC_BASE+STM32_RCC_APB2ENR_OFFSET)
#define STM32_GPIOC_CRL                 (STM32_GPIOC_BASE+STM32_GPIO_CRL_OFFSET)
#define STM32_GPIOC_CRH                 (STM32_GPIOC_BASE+STM32_GPIO_CRH_OFFSET)
#define STM32_GPIOC_BSRR                (STM32_GPIOC_BASE+STM32_GPIO_BSRR_OFFSET)
#define STM32_GPIOA_CRL                 (STM32_GPIOA_BASE+STM32_GPIO_CRL_OFFSET)
#define STM32_GPIOA_CRH                 (STM32_GPIOA_BASE+STM32_GPIO_CRH_OFFSET)
#define STM32_GPIOA_IDR                 (STM32_GPIOA_BASE+STM32_GPIO_IDR_OFFSET)

/* APB2 Peripheral Clock enable register (RCC_APB2ENR) */
#define RCC_APB2ENR_IOPCEN         (1 << 4)    /* Bit 4 IOPCEN: IO port C clock enable */

#define RCC_APB2ENR_IOPAEN         (1 << 2)        /* Bit 2 IOPAEN: IO port A clock enable */




/* Port configuration register */
#define GPIO_CNF_I_ANALOG_MODE        (0) /* Input: Analog mode */
#define GPIO_CNF_I_FLOATING_INPUT     (1) /* Input: Floating input (reset state) */
#define GPIO_CNF_I_PULL_UP_DOWN       (2) /* Input: Input with pull-up / pull-down */
                                      /* Input 11 (3): Reserved */
#define GPIO_CNF_O_GPO_PUSH_PULL      (0) /* Output: General purpose output push-pull */
#define GPIO_CNF_O_GPO_OPEN_DRAIN     (1) /* Output: General purpose output Open-drain */
#define GPIO_CNF_O_AFO_PUSH_PULL      (2) /* Output: Alternate function output Push-pull */
#define GPIO_CNF_O_AFO_OPEN_DRAIN     (3) /* Output: Alternate function output Open-drain */
#define GPIO_MODE_INPUT               (0) /* Input */
#define GPIO_MODE_OUTPUT_10MHZ        (1) /* Output 10MHz */
#define GPIO_MODE_OUTPUT_2MHZ         (2) /* Output 2MHz */
#define GPIO_MODE_OUTPUT_50MHZ        (3) /* Output 50MHz */

/* Deslocamento do registrador e Máscaras para setar os registradores da memória */

#define GPIO_MODE_SHIFT(n)            ((n < 8) ? (n << 2) : ((n - 8) << 2))
#define GPIO_MODE_MASK(n)             (3 << GPIO_MODE_SHIFT(n))
#define GPIO_CNF_SHIFT(n)             ((n < 8) ? ((n << 2) + 2) : (((n - 8) << 2) + 2))
#define GPIO_CNF_MASK(n)              (3 << GPIO_CNF_SHIFT(n))
/* GPIO port bit set/reset register */
#define GPIO_BSRR_SET(n)              (1 << (n))
#define GPIO_BSRR_RST(n)              (1 << (n + 16))
/* Functions */

uint32_t setGPIO(uint32_t, int, int, int);
uint32_t piscaLed(uint32_t, int, bool,int);
bool buttonClick(uint32_t,int);

static uint32_t led_status;
static const char fw_version[] = {'V', '1', '.', '0'};

int main(int argc, char *argv[])
{
    uint32_t reg;
    /* Ponteiros para registradores */
    uint32_t *pRCC_APB2ENR  = (uint32_t *)STM32_RCC_APB2ENR;
    uint32_t *pGPIOC_CRL    = (uint32_t *)STM32_GPIOC_CRL;
    uint32_t *pGPIOC_CRH    = (uint32_t *)STM32_GPIOC_CRH;
    uint32_t *pGPIOC_BSRR   = (uint32_t *)STM32_GPIOC_BSRR;

    uint32_t *pGPIOA_CRL    = (uint32_t *)STM32_GPIOA_CRL;
    uint32_t *pGPIOA_CRH    = (uint32_t *)STM32_GPIOA_CRH;
    uint32_t *pGPIOA_IDR    = (uint32_t *)STM32_GPIOA_IDR;

    /* Habilita clock GPIOC e GPIOA */
    reg  = *pRCC_APB2ENR;
    reg |= RCC_APB2ENR_IOPCEN;
    reg |= RCC_APB2ENR_IOPAEN;
    *pRCC_APB2ENR = reg;

    /* Configurando PC13 como saida output push-pull */
    if(LED_PIN < 8) *pGPIOC_CRL = setGPIO((uint32_t)*pGPIOC_CRL,LED_PIN,GPIO_CNF_O_GPO_PUSH_PULL,GPIO_MODE_OUTPUT_10MHZ); 
    else *pGPIOC_CRH = setGPIO((uint32_t)*pGPIOC_CRH,LED_PIN,GPIO_CNF_O_GPO_PUSH_PULL,GPIO_MODE_OUTPUT_10MHZ); 
    if(BUTTON < 8) *pGPIOA_CRL = setGPIO((uint32_t)*pGPIOA_CRL,BUTTON,GPIO_CNF_I_PULL_UP_DOWN,GPIO_MODE_INPUT); 
    else *pGPIOA_CRH = setGPIO((uint32_t)*pGPIOA_CRH,BUTTON,GPIO_CNF_I_PULL_UP_DOWN,GPIO_MODE_INPUT);

    bool led_status = false;
    bool pressed;
    int delay = LED_DELAY;

    while(1){
        *pGPIOC_BSRR = piscaLed((uint32_t)*pGPIOC_BSRR, LED_PIN, led_status,delay);
        led_status = !led_status;

        pressed = buttonClick((uint32_t)*pGPIOA_IDR, BUTTON);
        if(pressed) delay = LED_DELAY; 
        else delay = 3*LED_DELAY; 
    }

    
    /* Nao deveria chegar aqui */
    return EXIT_SUCCESS;
}

uint32_t setGPIO(uint32_t reg, int led_pin, int cnf, int mode){ 
    reg &= ~GPIO_CNF_MASK(led_pin);
    reg |= (cnf << GPIO_CNF_SHIFT(led_pin));

    reg &= ~GPIO_MODE_MASK(led_pin);
    reg |= (mode << GPIO_MODE_SHIFT(led_pin));

    return reg;
}

uint32_t piscaLed(uint32_t reg, int led_pin, bool led_status, int delay){
        
    if(led_status) reg = GPIO_BSRR_SET(led_pin);
    else reg = GPIO_BSRR_RST(led_pin);
    for(int i = 0; i < delay; i++);

    return reg;
}

bool buttonClick(uint32_t reg, int b_pin){
    reg &= (1 << b_pin);
    return (reg >= 1);
}