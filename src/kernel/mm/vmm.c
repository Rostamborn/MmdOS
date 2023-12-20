#include "../lib/util.h"
#include "../limine.h"
#include <stdint.h>

static volatile struct limine_kernel_address_request kaddr_req = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
};
