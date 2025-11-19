#include "hal.h"
#include "simpleserial.h"
#include <stdint.h>

/* RSA parameters:
 * p = 251, q = 239
 * N = p*q = 59989
 * d = 41
 */

static uint16_t rsa_N = 59989;
static uint16_t rsa_d = 41;


static uint16_t modmul(uint16_t a, uint16_t b)
{
    uint32_t prod = (uint32_t)a * (uint32_t)b;
    prod %= rsa_N;
    return (uint16_t)prod;
}

static void delay_cycles(volatile uint32_t cycles) {
    while(cycles--) {
        __asm__("nop");
    }
}

static uint16_t modexp(uint16_t base)
{
    uint16_t r = 1;
    for (int i = 0; i <= 7; i++) {
        /* always square */
        r = modmul(r, r);
        /* conditional multiply when bit of d is 1 */
        if ((rsa_d >> i) & 1) {
            r = modmul(r, base);
        }
        // help seeing different iterations of the loop in the trace visually
        // unnecessary in practice
        delay_cycles(30);
    }

    return r;
}

/* STM32 Firmware stuff
 */
uint8_t handle_rsa(uint8_t *pt, uint8_t len)
{
    if (len < 2)
        return 0;

    uint16_t m = ((uint16_t)pt[0] << 8) | pt[1];

    trigger_high();
    uint16_t c = modexp(m);
    trigger_low();

    uint8_t out[2];
    out[0] = (uint8_t)(c >> 8);
    out[1] = (uint8_t)(c & 0xFF);

    simpleserial_put('r', 2, out);
    return 0;
}

int main(void)
{
    platform_init();
    init_uart();
    trigger_setup();

    simpleserial_init();
    simpleserial_addcmd('p', 2, handle_rsa);

    while (1) {
        simpleserial_get();
    }

    return 0;
}
