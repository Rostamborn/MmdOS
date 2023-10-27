package main

import "cpu"
import "limine"

foreign {
    @(link_name="__$startup_runtime") _startup_runtime :: proc "odin" () ---
    @(link_name="__$cleanup_runtime") _cleanup_runtime :: proc "odin" () ---
}

@export term_req: limine.Terminal_Request = {
    id = limine.TERMINAL_REQUEST,
    revision = 0,
}

@export fb_req: limine.Framebuffer_Request = {
    id = limine.FRAMEBUFFER_REQUEST,
    revision = 1,
}

@export mem_req: limine.Memmap_Request = {
    id = limine.MEMMAP_REQUEST,
    revision = 0,
}

limine_print :: proc(req: ^limine.Terminal_Request, str: string) {
        term := req.response.terminals[0]
        req.response.write(term, cast([^]u8)raw_data(str), cast(u64)len(str))
}

@(export, link_name="m_main")
m_main :: proc "contextless" () {
    cpu.enable_sse()
    context = {}
    #force_no_inline _startup_runtime()


    // if term_req.response == nil || term_req.response.terminal_count < 1 {
    //     cpu.hcf()
    // }

//Check all framebuffers, find the one that we support
  //   framebuffer: ^limine.Framebuffer
  //   if fb_req.response != nil {
  //       for i := 0; i < cast(int)fb_req.response.framebuffer_count; i += 1 {
  //           fb := fb_req.response.framebuffers[i]
  //           if fb.memory_model != limine.FRAMEBUFFER_RGB {
  //               continue
  //           }
  //           if fb.bpp != 32 {
  //               continue
  //           }
  //           framebuffer = fb
  //           break
  //       }
  //   }
  //   if framebuffer == nil {
  //       limine_print(&term_req, "Error: unable to find framebuffer")
  //       cpu.hcf()
  //   }
  // //  Load memory map
  //   if mem_req.response == nil {
  //       limine_print(&term_req, "Error: unable to load memory map")
  //       cpu.hcf()
  //   }
    // memmap_entries := mem_req.response.entry_count
    // memmap_size := memmap_entries * size_of(kernel.Memmap_Entry)
    // memmap_storage := cast([^]kernel.Memmap_Entry)intrinsics.alloca(memmap_size, 16)
    // memmap := memmap_storage[:memmap_entries]
    // for i in 0 ..< memmap_entries {
    //     limine_entry := mem_req.response.entries[i]
    //     memmap_entry := &memmap[i]
    //     memmap_entry.base = auto_cast cast(uintptr) limine_entry.base
    //     memmap_entry.size = limine_entry.length
    //     switch limine_entry.type {
    //         case limine.MEMMAP_USABLE: {
    //             memmap_entry.type = .Usable
    //         } break
    //         case limine.MEMMAP_RESERVED: {
    //             memmap_entry.type = .Reserved
    //         } break
    //         case limine.MEMMAP_ACPI_RECLAIMABLE: {
    //             memmap_entry.type = .ACPI_Reclaimable
    //         } break
    //         case limine.MEMMAP_ACPI_NVS: {
    //             memmap_entry.type = .ACPI_NVS
    //         } break
    //         case limine.MEMMAP_BAD_MEMORY: {
    //             memmap_entry.type = .Bad_Memory
    //         } break
    //         case limine.MEMMAP_BOOTLOADER_RECLAIMABLE: {
    //             memmap_entry.type = .Boot
    //         } break
    //         case limine.MEMMAP_KERNEL_AND_MODULES: {
    //             memmap_entry.type = .Kernel
    //         } break
    //         case limine.MEMMAP_FRAMEBUFFER: {
    //             memmap_entry.type = .Framebuffer
    //         } break
    //     }
    // }
    //
    // for i in 0..=1000000 {
    //     limine_print(&term_req, "Hello, world!")
    // }
    // limine_print(&term_req, "Hello, world!")
    // cpu.hcf()


    kmain()
}
