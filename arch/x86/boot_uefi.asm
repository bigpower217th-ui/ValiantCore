bits 64
default rel

section .text
global efi_main
extern kmain

%define EFI_SUCCESS             0
%define EFI_BUFFER_TOO_SMALL    5

%define EFI_GOP_GUID_1  0x9042a9de
%define EFI_GOP_GUID_2  0x23dc4a38
%define EFI_GOP_GUID_3  0xb0d24b28
%define EFI_GOP_GUID_4  0x9a40972a

efi_main:
    push rbp
    mov  rbp, rsp
    sub  rsp, 64

    mov [rel system_table], rdx
    mov [rel image_handle], rcx

    mov rax, [rdx + 0x60]
    mov [rel boot_services], rax

    lea rcx, [rel gop_guid]
    xor rdx, rdx
    lea r8,  [rel gop_ptr]

    mov rax, [rel boot_services]
    call [rax + 0x140]

    cmp rax, EFI_SUCCESS
    jne .no_gop

    mov rax, [rel gop_ptr]
    mov rax, [rax + 0x18]
    mov rbx, [rax + 0x18]
    mov [rel fb_base], rbx

    mov ecx, [rax + 0x20]
    mov [rel fb_size], ecx

    mov rax, [rax + 0x10]
    mov ecx, [rax + 0x04]
    mov [rel fb_width], ecx
    mov ecx, [rax + 0x08]
    mov [rel fb_height], ecx
    mov ecx, [rax + 0x0C]
    mov [rel fb_pitch], ecx

.no_gop:
    lea rcx, [rel mmap_size]
    lea rdx, [rel mmap_buf]
    lea r8,  [rel mmap_key]
    lea r9,  [rel mmap_desc_size]
    push 0
    lea  rax, [rel mmap_desc_ver]
    push rax

    mov rax, [rel boot_services]
    call [rax + 0x58]
    add rsp, 16

    mov rcx, [rel image_handle]
    mov rdx, [rel mmap_key]
    mov rax, [rel boot_services]
    call [rax + 0xE8]

    mov rcx, [rel fb_base]
    mov edx, [rel fb_width]
    mov r8d,  [rel fb_height]
    mov r9d,  [rel fb_pitch]
    call kmain

    cli
    hlt

section .data
align 8

gop_guid:
    dd EFI_GOP_GUID_1
    dd EFI_GOP_GUID_2
    dd EFI_GOP_GUID_3
    dd EFI_GOP_GUID_4

system_table: dq 0
image_handle: dq 0
boot_services: dq 0
gop_ptr:      dq 0
fb_base:      dq 0
fb_size:      dd 0
fb_width:     dd 0
fb_height:    dd 0
fb_pitch:     dd 0

mmap_size:     dq 4096
mmap_key:      dq 0
mmap_desc_size: dq 0
mmap_desc_ver:  dd 0

section .bss
align 4096
mmap_buf: resb 4096
