package main

import "cpu"
import "limine"

foreign {
    @(link_name="__$startup_runtime") _startup_runtime :: proc "odin" () ---
    @(link_name="__$cleanup_runtime") _cleanup_runtime :: proc "odin" () ---
}

@export
limine_term_req := limine.Terminal_Request {
    id = limine.TERMINAL_REQUEST,
    revision = 0,
}

@export
limine_fb_req := limine.Framebuffer_Request {
    id = limine.FRAMEBUFFER_REQUEST,
    revision = 1,
}

@export
limine_mem_req := limine.Memmap_Request {
    id = limine.MEMMAP_REQUEST,
    revision = 0,
}

@(export, link_name="m_main")
m_main :: proc "contextless" () {
    cpu.enable_sse()
    context = {}
    #force_no_inline _startup_runtime()

    if limine_term_req.response == nil || limine_term_req.response.terminal_count < 1 {
        cpu.hcf()
    }

    str := string("mamad")
    ternimal_rs := limine_term_req.response
    terminal := limine_term_req.response.terminals[0]
    ternimal_rs.write(terminal, cast([^]u8)raw_data(str), cast(u64)len(str))

    // kmain()
    cpu.hcf()
}
