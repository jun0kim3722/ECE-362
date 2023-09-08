/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An
  * @version V1.0
  * @date    Oct 17, 2022
  * @brief   ECE 362 Lab 6 Student template
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include <stdint.h>

void initc();
void initb();
void togglexn(GPIO_TypeDef *port, int n);
void init_exti();
void set_col(int col);
void SysTick_Handler();
void init_systick();
void adjust_priorities();

extern void nano_wait(int);

volatile int current_col = 1;

int main(void) {
    // Uncomment when most things are working
    //autotest();
    
    initb();
    initc();
    init_exti();
    init_systick();
    adjust_priorities();

    // Slowly blinking
    for(;;) {
        togglexn(GPIOC, 9);
        nano_wait(500000000);
    }
}

/**
 * @brief Init GPIO port C
 *        PC0-PC3 as input pins with the pull down resistor enabled
 *        PC4-PC9 as output pins
 * 
 */
void initc() {
//    0-PC3 as input pins with the pull down resistor enabled
//    PC4-PC9 as output pins
    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC -> MODER &= 0xfff000ff;
    GPIOC -> MODER |= 0x00055500;
    GPIOC -> MODER &= 0xffffff00;

    GPIOC -> PUPDR &= 0xffffff00;
    GPIOC -> PUPDR |= 0x000000AA;
}

/**
 * @brief Init GPIO port B
 *        PB0, PB2, PB3, PB4 as input pins
 *          enable pull down resistor on PB2 and PB3
 *        PB8-PB11 as output pins
 * 
 */
void initb() {
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB -> MODER &= 0xff00ffff;
    GPIOB -> MODER |= 0x00550000;
    GPIOB -> MODER &= 0xfffffC0C;

    GPIOB -> PUPDR &= 0xffffff0f;
    GPIOB -> PUPDR |= 0x000000A0;
}

/**
 * @brief Change the ODR value from 0 to 1 or 1 to 0 for a specified 
 *        pin of a port.
 * 
 * @param port : The passed in GPIO Port
 * @param n    : The pin number
 */
void togglexn(GPIO_TypeDef *port, int n) {

    port -> ODR ^= (0x0000001 << n); // check port val

}

//==========================================================
// Write the EXTI interrupt handler for pins 0 and 1 below.
// Copy the name from startup/startup_stm32.s, create a label
// of that name below, declare it to be global, and declare
// it to be a function.
// It acknowledge the pending bit for pin 0, and it should
// call togglexn(GPIOB, 8).
void EXTI0_1_IRQHandler() {
    EXTI -> PR = EXTI_PR_PR0;
    togglexn(GPIOB, 8);
}



//EXTI0_1_IRQHandler                              /* EXTI Line[1:0] interrupts                                         */
//  .word EXTI2_3_IRQHandler                              /* EXTI Line[3:2] interrupts                                         */
//  .word EXTI4_15_IRQHandler


//==========================================================
// Write the EXTI interrupt handler for pins 2-3 below.
// It should acknowledge the pending bit for pin2, and it
// should call togglexn(GPIOB, 9).
void EXTI2_3_IRQHandler() {
    EXTI -> PR = EXTI_PR_PR2;
    togglexn(GPIOB, 9);
}


//==========================================================
// Write the EXTI interrupt handler for pins 4-15 below.
// It should acknowledge the pending bit for pin4, and it
// should call togglxn(GPIOB, 10).
void EXTI4_15_IRQHandler() {
    EXTI -> PR = EXTI_PR_PR4;
    togglexn(GPIOB, 10);
}


/**
 * @brief Follow lab manual section 4.4 to initialize EXTI
 *        (1-2) Enable the SYSCFG subsystem, and select Port B for
 *            pins 0, 2, 3, and 4.
 *        (3) Configure the EXTI_RTSR register so that an EXTI
 *            interrupt is generated on the rising edge of
 *            pins 0, 2, 3, and 4.
 *        (4) Configure the EXTI_IMR register so that the EXTI
 *            interrupts are unmasked for pins 2, 3, and 4.
 *        (5) Enable the three interupts for EXTI pins 0-1, 2-3 and
 *            4-15. Don't enable any other interrupts.
 */
void init_exti() {
// (1-2) Enable the SYSCFG subsystem, and select Port B for pins 0, 2, 3, and 4.
    RCC -> APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
    SYSCFG-> EXTICR[0] |= 0x1101;
    SYSCFG -> EXTICR[1] |= 0x0001;
// 3) Configure the EXTI_RTSR register so that an EXTI interrupt is generated on the rising edge of
//    pins 0, 2, 3, and 4.
    EXTI -> RTSR |= 0x0000001D;
//    (4) Configure the EXTI_IMR register so that the EXTI
//     *            interrupts are unmasked for pins 0, 2, 3, and 4.
    EXTI -> IMR |= 0x0000001D;
//    5) Enable the three interupts for EXTI pins 0-1, 2-3 and
//     *            4-15. Don't enable any other interrupts.
    NVIC -> ISER[0] |= 1 << 5;
    NVIC -> ISER[0] |= 1 << 6;
    NVIC -> ISER[0] |= 1 << 7;

}

/**
 * @brief For the keypad pins, 
 *        Set the specified column level to logic "high.
 *        Set the other three three columns to logic "low".
 * 
 * @param col 
 */
void set_col(int col) {
    // Set PC4-7 (i.e. all columns) output to be 0
    GPIOC -> ODR &= 0xffffff0f;
    // Set the column `col` output to be 1
    //  if col = 1, PC7 will be set to 1 as
    if (col == 1) {
        GPIOC -> ODR |= 0x00000080;
    }
    else if (col == 2) {
        GPIOC -> ODR |= 0x00000040;
    }
    else if (col == 3) {
        GPIOC -> ODR |= 0x00000020;
    }
    //  it is connected to column 1 of the keypad
    //  Likewise, if col = 4, PC4 will be set to 1
    if (col == 4) {
        GPIOC -> ODR |= 0x00000010;
    }

}

/**
 * @brief The ISR for the SysTick interrupt.
 * 
 */
//volatile int current_col = 1;
void SysTick_Handler() {
    // 1. Read the row pins using GPIOC->IDR
    //    You can check the pins used for rows
    //    of keypad in lab 5 manual
    // 2. If the var `current_col` corresponds to
    //    the row value, toggle one of the leds connected
    //    to PB8-11.
    //    Basically the same we have done in lab 5
    // 3. Increment the `current_col` and wrap around
    //    to 1 if `current_col` > 4. So that next time
    //    we scan the next column
    // 4. Set the changed column pin designated by `current_col`
    //    to 1 and rest of the column pins to 0 to energized that
    //    particular column for next read of keypad.

    // 1. Read the row pins using GPIOC->IDR
    //    You can check the pins used for rows
    //    of keypad in lab 5 manual
    int rows = GPIOC -> IDR & 0xf;
    // 2.If the var `current_col` corresponds to
    //    the row value, toggle one of the leds connected
    //    to PB8-11.
    if ((rows >> (4 - current_col) & 1) == 1) {
        togglexn(GPIOB, current_col + 7);
    }
    // 3.Increment the `current_col` and wrap around
    //    to 1 if `current_col` > 4. So that next time
    //    we scan the next column
    current_col++;
    if (current_col > 4) {
        current_col = 1;
    }
    // 4. Set the changed column pin designated by `current_col`
    //    to 1 and rest of the column pins to 0 to energized that
    //    particular column for next read of keypad.
    set_col(current_col);
}

/**
 * @brief Enable the SysTick interrupt to occur every 1/16 seconds.
 * 
 */
void init_systick() {
//    SysTick_Handler
    SysTick -> LOAD = 375000 - 1; // ClockRate / (RVR + 1)
    SysTick -> CTRL |= 0x00000003; // CLKSOURCE 0, TICKINT 1, ENABLE 1, == 3

}

/**
 * @brief Bonus question
 *        Set the priority for EXTI pins 2-3 interrupt to 192.
 *        Set the priority for EXTI pins 4-15 interrupt to 128.
 *        Do not adjust the priority for any other interrupts.
 * 
 */
void adjust_priorities() {
    // Most significant bits of 0x80 = 10, 0xc0 = 11
//    NVIC_SetPriority(priority, two bits)
    NVIC_SetPriority(6, 0b11);
    NVIC_SetPriority(7, 0b10);


}
