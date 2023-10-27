package boot

import kernel "kernel:."

@(export, link_name="k_start")
k_start :: proc "sysv" () {

    kernel.m_main()
}
