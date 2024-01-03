#include "src/kernel/lib/logger.h"

void add_one_to_x() {
    int x = 0;
    int i = 0;
    while (1) {
        if (i == 10000) {
            i = 0;
            klog("DEMO::", "x is now %d", x);
        }
        x++;
        i++;
    }
}
void add_one_to_y() {
    int y = 0;
    int i = 0;
    while (1) {
        if (i == 10000) {
            i = 0;
            klog("DEMO::", "y is now %d", y);
        }
        y++;
        i++;
    }
}
void add_one_to_z() {
    int z = 0;
    int i = 0;
    while (1) {
        if (i == 10000) {
            i = 0;
            klog("DEMO::", "z is now %d", z);
        }
        z++;
        i++;
    }
}