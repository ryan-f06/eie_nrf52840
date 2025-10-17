/*
 * main.c
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 1

int main(void) {

  if (0 > BTN_init()) {
    return 0;
  } 
  if (0 > LED_init()) {
    return 0;
  } 

  int b = 0;
  while(1) {
    if (BTN_check_clear_pressed(BTN0)) {
      b++;
      if (b == 16)
        b = 0;
      LED_toggle(LED0);
      if (b & 0b10) {
        LED_set(LED1, LED_ON);
      }
      else
        LED_set(LED1, LED_OFF);

      if (b & 0b100) {
        LED_set(LED2, LED_ON);
      }
      else
        LED_set(LED2, LED_OFF);
      if (b & 0b1000) {
        LED_set(LED3, LED_ON);
      }
      else
        LED_set(LED3, LED_OFF);
      
      printk("Button 0 pressed!\n");
    }
    k_msleep(SLEEP_MS);
  }
	return 0;
}
