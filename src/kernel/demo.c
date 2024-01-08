#include "lib/logger.h"
#include "demo.h"

void add_one_to_x(void* dummy) {
    int x = 0;

    while (x < 5000000) {
        x++;
    }
    klog("DEMO::", "x  will now exit");
}
void add_one_to_y(void* dummy) {
    int y = 0;

    while (y < 6000000) {
        y++;
    }
    klog("DEMO::", "y  will now exit");
}
void add_one_to_z(void* dummy) {
    int z = 0;

    while (z < 7000000) {
        z++;
    }

    klog("DEMO::", "z  will now exit");
}
