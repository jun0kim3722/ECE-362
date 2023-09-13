/**
  ******************************************************************************
  * @file    main.c
  * @author  Weili An
  * @version V1.0
  * @date    Oct 24, 2022
  * @brief   ECE 362 Lab 7 template
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include <stdint.h>

// Global data structure
char* login          = "xyz"; // Replace with your login.
char disp[9]         = "Hello...";
uint8_t col          = 0;
uint8_t mode         = 'A';
uint8_t thrust       = 0;
int16_t fuel         = 800;
int16_t alt          = 4500;
int16_t velo         = 0;

// Make them visible to autotest.o
extern char* login;
// Keymap is in `font.S` to match up what autotester expected
extern char keymap;
extern char disp[9];
extern uint8_t col;
extern uint8_t mode;
extern uint8_t thrust;
extern int16_t fuel;
extern int16_t alt;
extern int16_t velo;

char* keymap_arr = &keymap;

// Font array in assembly file
// as I am too lazy to convert it into C array
extern uint8_t font[];

// The functions we should implement
void enable_ports();
void setup_tim6();
void show_char(int n, char c);
void drive_column(int c);
int read_rows();
char rows_to_key(int rows);
void handle_key(char key);
void setup_tim7();
void write_display();
void update_variables();
void setup_tim14();

// Auotest functions
extern void check_wiring();
extern void autotest();
extern void fill_alpha();

int main(void) {
    // check_wiring();
//    autotest();
//    fill_alpha();
    enable_ports();
    setup_tim6();
    setup_tim7();
    setup_tim14();

    for(;;) {
        asm("wfi");
    }
}

/**
 * @brief Enable the ports and configure pins as described
 *        in lab handout
 * 
 */
void enable_ports(){
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB -> MODER &= 0xffc00000;
    GPIOB -> MODER |= 0x00155555;

    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC -> MODER &= 0xfffc00ff;
    GPIOC -> MODER |= 0x00015500; //output
    GPIOC -> MODER &= 0xffffff00;

    GPIOC -> PUPDR &= 0xffffff00;
    GPIOC -> PUPDR |= 0x000000aa;

}

//-------------------------------
// Timer 6 ISR goes here
//-------------------------------
// TODO
void TIM6_DAC_IRQHandler() {
    TIM6 -> SR &= ~TIM_SR_UIF;
    if ((GPIOC -> ODR >> 8 & 1) == 1) {
        GPIOC -> BRR = 0x0100;
    }
    else {
        GPIOC -> BSRR = 0x0100;
    }
}


/**
 * @brief Set up timer 6 as described in handout
 * 
 */
void setup_tim6() {
    RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6 -> PSC = 48000 - 1;
    TIM6 -> ARR = 500 - 1;
    TIM6 -> DIER |= TIM_DIER_UIE;
    TIM6 -> CR1 |= TIM_CR1_CEN;
    NVIC -> ISER[0] = 1 << TIM6_DAC_IRQn;
}

/**
 * @brief Show a character `c` on column `n`
 *        of the segment LED display
 * 
 * @param n 
 * @param c 
 */
void show_char(int n, char c) {
//    make sure 0 <= n <= 7, i.e. lies in the valid range
//    assert(0 <= n || n <= 7);
    if (!(0 <= n && n <= 7)) {
        return;
    }
   // font[c] contains the bit setting for PB0-7 (the sink driver)
//    GPIOB -> ODR |= hex_val;
//        write content of font[c] to the corresponding bits in `GPIOB->ODR` (i.e. those pins controlling the decoder and the sink driver)
    int sum = (n * 16) << 4 | font[c];
    GPIOB -> ODR &= 0xfffff000;
    GPIOB -> ODR |= sum;


}

/**
 * @brief Drive the column pins of the keypad
 *        First clear the keypad column output
 *        Then drive the column represented by `c`
 * 
 * @param c 
 */
void drive_column(int c) {
//    c = least 2 bits of c
    int two_bit = c & 0b11;
//    first clear the bits 4-7 of GPIOC
    GPIOC -> BRR = 0x00f0;
//    then set the bits corresponding to the column `c`
    GPIOC -> BSRR = 1 << 4 + two_bit;
}

/**
 * @brief Read the rows value of the keypad
 * 
 * @return int 
 */
int read_rows() {
    return GPIOC -> IDR & 0xf;
}

/**
 * @brief Convert the pressed key to character
 *        Use the rows value and the current `col`
 *        being scanning to compute an offset into
 *        the character map array
 * 
 * @param rows 
 * @return char 
 */
char rows_to_key(int rows) {
//    compute the offset of the button being pressed right now from `rows` and `col`
//    Note `rows` will be a 4bit value from reading the IDR register of the row pins of the keypad
    int off_set = col * 4;
    if (rows == 1){
        return keymap_arr[off_set];
    }
    else if (rows == 2) {
        return keymap_arr[off_set + 1];
    }
    else if (rows == 4) {
        return keymap_arr[off_set + 2];
    }
    else if (rows == 8) {
        return keymap_arr[off_set + 3];
    }
}

/**
 * @brief Handle key pressed in the game
 * 
 * @param key 
 */
void handle_key(char key) {
    if (key == 'A' || key == 'B' || key == 'D') {
//        set mode to key
        mode = key;
    }
    else { // if key is a digit
//        set thrust to the represented value of key, i.e. if key == '1', thrust = 1, not '1'
        thrust = key - 48;
    }
}

//-------------------------------
// Timer 7 ISR goes here
//-------------------------------
// TODO
void TIM7_IRQHandler() {
    TIM7 -> SR &= ~TIM_SR_UIF;
    int rows = read_rows();

    if (rows != 0) {
        int key = rows_to_key(rows);
        handle_key(key);
    }

    show_char(col, disp[col]);
    col += 1;
    if (col > 7) {
        col = 0;
    }
    drive_column(col);
}

/**
 * @brief Setup timer 7 as described in lab handout
 * 
 */
void setup_tim7() {
    RCC -> APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7 -> PSC = 24000 - 1;
    TIM7 -> ARR = 2 - 1;
    TIM7 -> DIER |= TIM_DIER_UIE;
    TIM7 -> CR1 |= TIM_CR1_CEN;
    NVIC -> ISER[0] = 1 << TIM7_IRQn;

}

/**
 * @brief Write the display based on game's mode
 * 
 */
void write_display() {
    if (mode == 'C') {
        snprintf(disp, 9, "Crashed");
    }
    else if (mode == 'L') {
        snprintf(disp, 9, "Landed ");
    }
    else if (mode == 'A') {
        snprintf(disp, 9, "ALt%5d", alt);
    }
    else if (mode == 'B') {
        snprintf(disp, 9, "FUEL %3d", fuel);
    }
    else if (mode == 'D') {
        snprintf(disp, 9, "Spd %4d", velo);
    }
}

/**
 * @brief Game logic
 * 
 */
void update_variables() {
    fuel -= thrust;
    if (fuel <= 0) {
        thrust = 0;
        fuel = 0;
    }
    alt += velo;
    if (alt <= 0) {
        if (-velo < 10) {
            mode = 'L';
        }
        else {
            mode = 'C';
        }
        return;
    }
    velo += thrust - 5;
}

//-------------------------------
// Timer 14 ISR goes here
//-------------------------------
void TIM14_IRQHandler() {
    TIM14 -> SR &= ~TIM_SR_UIF;
    update_variables();
    write_display();
}

/**
 * @brief Setup timer 14 as described in lab
 *        handout
 * 
 */
void setup_tim14() {
    RCC -> APB1ENR |= RCC_APB1ENR_TIM14EN;
    TIM14 -> PSC = 48000 - 1;
    TIM14 -> ARR = 500 - 1;
    TIM14 -> DIER |= TIM_DIER_UIE;
    TIM14 -> CR1 |= TIM_CR1_CEN;
    NVIC -> ISER[0] = 1 << TIM14_IRQn;
}
