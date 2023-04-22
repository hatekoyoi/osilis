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
