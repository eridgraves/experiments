/**
 * Take UART input from PIO, and spit out on UART0 or UART1 depending on selection.
 *
 * Based on UART examples here: https://github.com/raspberrypi/pico-examples/tree/master/pio
 */

 #include <stdio.h>

 #include "pico/stdlib.h"
 #include "pico/multicore.h"
 #include "hardware/pio.h"
 #include "hardware/uart.h"
 #include "uart_rx.pio.h"
 
 
 #define SERIAL_BAUD PICO_DEFAULT_UART_BAUD_RATE
 #define HARD_UART_INST uart1
 
 #define PIO_UART_RX_PIN 3   // UART input from host machine
 #define UART_TX_PIN 4       // Currently-selected UART output
 
 // Kept from example:
 // Check the pin is compatible with the platform
 #if PIO_UART_RX_PIN >= NUM_BANK0_GPIOS
 #error Attempting to use a pin>=32 on a platform that does not support it
 #endif
 
 int main() {
     // Console output (also a UART, yes it's confusing)
     setup_default_uart();
     printf("Starting PIO UART RX example\n");
 
     // Set up the hard UART we're going to use to recieve from host
     uart_init(HARD_UART_INST, SERIAL_BAUD);
     gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
 
     // Set up the state machine we're going to use to receive
     PIO pio;
     uint sm;
     uint offset;
 
     // This will find a free pio and state machine for our program and load it for us
     // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
     // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
     bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&uart_rx_program, &pio, &sm, &offset, PIO_UART_RX_PIN, 1, true);
     hard_assert(success);
 
     uart_rx_program_init(pio, sm, offset, PIO_UART_RX_PIN, SERIAL_BAUD);   // in .pio file
     //uart_rx_mini_program_init(pio, sm, offset, PIO_UART_RX_PIN, SERIAL_BAUD);
 
     // Tell core 1 to print some text to uart1 as fast as it can
     multicore_launch_core1(core1_main);
     const char *text = "Hello, world from PIO! (Plus 2 UARTs and 2 cores, for complex reasons)\n";
     multicore_fifo_push_blocking((uint32_t) text);
 
     // Echo characters received from PIO to the console
     while (true) {
         char c = uart_rx_program_getc(pio, sm);
         putchar(c);
     }
 
     // This will free resources and unload our program
     pio_remove_program_and_unclaim_sm(&uart_rx_program, pio, sm, offset);
 }