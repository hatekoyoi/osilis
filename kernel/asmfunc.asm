bits 64
section .text

; void IoOut32(uint16_t addr, uint32_t data);
global IoOut32
IoOut32:
    mov dx, di          ; dx = addr
    mov eax, esi        ; eax = data
    out dx, eax
    ret

; uint32_t IoIn32(uint16_t addr);
global IoIn32
IoIn32:
    mov dx, di          ; dx = addr
    in eax, dx
    ret

; uint16_t GetCS(void);
global GetCS
GetCS:
    xor eax, eax        ; also clears upper 32 bits of rax
    mov ax, cs
    ret

; void LoadIDT(uint16_t limit, uint64_t offset);
global LoadIDT
LoadIDT:
    push rbp
    mov rbp, rsp
    sub rsp, 10
    mov [rsp], di       ; limit
    mov [rsp + 2], rsi  ; offset
    lidt [rsp]
    mov rsp, rbp
    pop rbp
    ret

; void LoadGDT(uint16_t limit, uint64_t offset);
global LoadGDT
LoadGDT:
    push rbp
    mov rbp, rsp
    sub rsp, 10
    mov [rsp], di       ; limit
    mov [rsp + 2], rsi  ; offset
    lgdt [rsp]
    mov rsp, rbp
    pop rbp
    ret

; void SetCSSS(uint16_t cs, uint16_t ss);
global SetCSSS
SetCSSS:
    push rbp
    mov rbp, rsp
    mov ss, si
    mov rax, .next
    push rdi            ; CS
    push rax            ; RIP
    o64 retf
.next:
    mov rsp, rbp
    pop rbp
    ret

; void SetDSAll(uint16_t value);
global SetDSAll
SetDSAll:
    mov ds, di
    mov es, di
    mov fs, di
    mov gs, di
    ret

; void SetCR3(uint64_t value);
global SetCR3
SetCR3:
    mov cr3, rdi
    ret

extern kernel_main_stack
extern KernelMainNewStack

global KernelMain
KernelMain:
    mov rsp, kernel_main_stack + 1024 * 1024
    call KernelMainNewStack
.fin:
    hlt
    jmp .fin
