section .text
    global _start
    global system_call
    global infection
    global infector
    extern main
    extern strlen
section .data
    newline db 10
    message db "Hello, Infected File", 0xA,0  ; message with newline
    
_start:
    pop dword ecx    ; ecx = argc
    mov esi, esp     ; esi = argv
    mov eax, ecx     ; put the number of arguments into eax
    shl eax, 2       ; compute the size of argv in bytes
    add eax, esi     ; add the size to the address of argv 
    add eax, 4       ; skip NULL at the end of argv
    push dword eax   ; char *envp[]
    push dword esi   ; char* argv[]
    push dword ecx   ; int argc

    call main        ; int main( int argc, char *argv[], char *envp[] )

    mov ebx, eax
    mov eax, 1
    int 0x80
    nop

system_call:
    push ebp             ; Save caller state
    mov ebp, esp
    sub esp, 4           ; Leave space for local var on stack
    pushad               ; Save some more caller state

    mov eax, [ebp+8]     ; Copy function args to registers: leftmost...        
    mov ebx, [ebp+12]    ; Next argument...
    mov ecx, [ebp+16]    ; Next argument...
    mov edx, [ebp+20]    ; Next argument...
    int 0x80             ; Transfer control to operating system
    mov [ebp-4], eax     ; Save returned value...
    popad                ; Restore caller state (registers)
    mov eax, [ebp-4]     ; place returned value where caller can see it
    add esp, 4           ; Restore caller state
    pop ebp              ; Restore caller state
    ret                  ; Back to caller

infection:
    ; Print "Hello, Infected File\n"
    ;mov eax, 4           ; syscall number (sys_write)
    pushad
    mov ebx, 1           ; file descriptor (stdout)
    mov ecx, message     ; message to write
    push ecx
    call strlen
    add esp, 4
    mov ecx, message
    mov edx, eax        ; message length
    mov eax, 4           ; syscall number (sys_write)
    ;mov edx, messge_len ; message length
    int 0x80             ; make syscall
    popad
    ret


infector:
    ; Print the file name
    mov ebx, 1           ; file descriptor (stdout)
    mov ecx, [esp + 4]   ; address of the file name
    push ecx
    call strlen
    pop ecx
    mov edx, eax         ; length of the file name
    mov eax, 4           ; syscall number (sys_write)
    int 0x80             ; make syscall

    mov eax, 4
    mov ebx, 1
    mov ecx, newline
    mov edx, 1
    int 0x80

    ; Open the file in append mode
    mov eax, 5           ; syscall number (sys_open)
    mov ebx, [esp + 4]   ; file name
    mov ecx, 0x102       ; flags: O_WRONLY | O_APPEND
    mov edx, 0x1B6       ; mode: 0666
    int 0x80             ; make syscall

    ; Save file descriptor
    mov esi, eax

    ; Write the virus code
    mov eax, 4           ; syscall number (sys_write)
    mov ebx, esi         ; file descriptor
    mov ecx, code_start  ; address of the code
    mov edx, code_end - code_start ; length of the code
    int 0x80             ; make syscall

    ; Close the file
    mov eax, 6           ; syscall number (sys_close)
    mov ebx, esi         ; file descriptor
    int 0x80             ; make syscall

    ret

section .data
code_start:
    ; The virus code to be appended
    db 'V', 'I', 'R', 'U', 'S', '_', 'C', 'O', 'D', 'E', '_', 'H', 'E', 'R', 'E'
code_end: