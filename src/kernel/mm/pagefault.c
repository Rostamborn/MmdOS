#include "../interrupts/idt.h"
#include "../lib/logger.h"

execution_context* pagefault_handler(execution_context* context) {
    klog("PAGEFAULT", "Installed handler for pagefaults");
}

void pagefault_init() { isr_install_handler(14, &pagefault_handler); }
