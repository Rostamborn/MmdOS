#include "../fs/vfs.h"
#include "../lib/logger.h"

void add_one_to_x() {
    int x = 0;

    while (x < 5000000) {
        x++;
        // klog("DEMO ::", "x: %d\n", x);
    }
    klog("DEMO::", "x  will now exit");
}
void add_one_to_y() {
    int y = 0;

    while (y < 6000000) {
        y++;
        // klog("DEMO ::", "y: %d\n", y);
    }
    klog("DEMO::", "y  will now exit");
}
void add_one_to_z() {
    int z = 0;

    while (z < 7000000) {
        z++;
        // klog("DEMO ::", "z: %d\n", z);
    }

    klog("DEMO::", "z  will now exit");
}

void read_file_x() {
    int  id = vfs_open("/x.txt", 0);
    char buffer[50];
    kprintf("\n");
    while (1) {
        int read = vfs_read(id, buffer, 50);
        if (read > 0) {
            for (int k = 0; k < read; k++) {
                kprintf("%c", buffer[k]);
            }
        } else {
            break;
        }
    }
    kprintf("\n");
    vfs_close(id);
}

void read_file_y() {
    int  id = vfs_open("/y.txt", 0);
    char buffer[50];
    kprintf("\n");
    while (1) {
        int read = vfs_read(id, buffer, 50);
        if (read > 0) {
            for (int k = 0; k < read; k++) {
                kprintf("%c", buffer[k]);
            }
        } else {
            break;
        }
    }
    kprintf("\n");
    vfs_close(id);
}