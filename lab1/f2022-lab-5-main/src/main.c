/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An
  * @version V1.0
  * @date    Oct 10, 2022
  * @brief   ECE 362 Lab 5 template
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include <stdint.h>

void initb();
void initc();
void setn(int32_t pin_num, int32_t val);
int32_t readpin(int32_t pin_num);
void buttons(void);
void keypad(void);

void mysleep(void) {
    for(int n = 0; n < 1000; n++);
}

int main(void) {
    // Uncomment when most things are working
    autotest();
//    keypad();
    
    initb();
    initc();

    // uncomment one of the loops, below, when ready
    // while(1) {
    //   buttons();
    // }

    // while(1) {
    //   keypad();
    // }

    for(;;);
}

/**
 * @brief Init GPIO port B
 *        Pin 0: input
 *        Pin 4: input
 *        Pin 8-11: output
 *
 */
void initb() {
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB -> MODER &= 0xff00ffff;
    GPIOB -> MODER |= 0x00550000; // set output
    GPIOB -> MODER &= ~0x00000303; // set input
}

/**
 * @brief Init GPIO port C
 *        Pin 0-3: inputs with internal pull down resistors
 *        Pin 4-7: outputs
 *
 */
void initc() {
    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC -> MODER &= 0xffff00ff;
    GPIOC -> MODER |= 0x00005500; // output
    GPIOC -> MODER &= ~0x000000ff;

    GPIOC -> PUPDR &= 0xffffff00;
    GPIOC -> PUPDR |= 0x000000aa; // pull down
}

/**
 * @brief Set GPIO port B pin to some value
 *
 * @param pin_num: Pin number in GPIO B
 * @param val    : Pin value, if 0 then the
 *                 pin is set low, else set high
 */
void setn(int32_t pin_num, int32_t val) {
    if (val == 0) {
        // turn on
        GPIOB -> BRR |= 1 << pin_num;
    }
    else {
        // turn off
        GPIOB -> BSRR |= 1 << pin_num;
    }

}

/**
 * @brief Read GPIO port B pin values
 *
 * @param pin_num   : Pin number in GPIO B to be read
 * @return int32_t  : 1: the pin is high; 0: the pin is low
 */
int32_t readpin(int32_t pin_num) {
    if ((GPIOB -> IDR & 1 << pin_num) >> pin_num == 1) {
        return 0x1;
    }
    else {
        return 0x0;
    }
}

/**
 * @brief Control LEDs with buttons
 *        Use PB0 value for PB8
 *        Use PB4 value for PB9
 *
 */
void buttons(void) {
// Use the implemented subroutines

    // Read button input at PB0
    int32_t bt0 = readpin(0);

    // Put the PB0 value as output for PB8
    setn(8, bt0);

    // Read button input at PB4
    int32_t bt4 = readpin(4);

    // Put the PB4 value as output for PB9
    setn(9, bt4);
}

/**
 * @brief Control LEDs with keypad
 *
 */
void keypad(void) {
    for (int i = 1; i < 5; i++) {

        GPIOC -> ODR |= 1 << (8-i);
        mysleep();

        int val = GPIOC -> IDR & 0xF;
        if (i == 1 && val == 0b1000) {
            setn(8, 1);
        }
        else if (i == 2 && val == 0b0100) {
            setn(9, 1);
        }
        else if (i == 3 && val == 0b0010) {
            setn(10, 1);
        }
        else if (i == 4 && val == 0b0001) {
            setn(11, 1);
        }
        else {
            setn(8,0);
            setn(9,0);
            setn(10,0);
            setn(11,0);
        }
    }
}
