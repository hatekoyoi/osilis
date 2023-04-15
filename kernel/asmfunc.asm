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
