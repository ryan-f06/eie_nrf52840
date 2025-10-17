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

  int b1 = 0, b2 = 0, b3 = 0, b4 = 0;
  while(1) {
    if (BTN_check_clear_pressed(BTN0)) {
      b1++;
      LED_toggle(LED0);
      if (b1 == 2) {
        b1 = 0;
        b2++;
        LED_toggle(LED1);
      }
      if (b2 == 2) {
        b2 = 0;
        b3++;
        LED_toggle(LED2);
      }
      if (b3 == 2) {
        b3 = 0;
        b4++;
        LED_toggle(LED3);
      }
      if (b4 == 2) {
        b4 = 0;
      }
      
      
      
       
      printk("Button 0 pressed!\n");
    }
    k_msleep(SLEEP_MS);
  }
	return 0;
}
