#include "EventQueue.h"
#include "mbed.h"

Ticker ticker;
EventQueue queue(32 * EVENTS_EVENT_SIZE);

DigitalOut led(LED1);

Thread t;

void send() {
    /*
    __disable_irq();
    printf("OK\n");
    __enable_irq();
    */
    led = !led;
    queue.call(printf, "Led status: %i\n", led.read());
}

int main() {
    t.start(callback(&queue, &EventQueue::dispatch_forever));
    ticker.attach(send, 1s);
    printf("Haloo\n");
    
}
