section .data
    newline db 10
    infile_descriptor dd 0  ; default stdin = 0
    outfile_descriptor dd 1 ; default stdout = 1
section .bss
    input_buffer resb 1
section .text
    global main
    extern strlen

prepare_input:
    pushad
    mov eax, 5     ; sys_open              
    mov ebx, ecx   ; -i{file}        
    inc ebx
    inc ebx        
    xor ecx, ecx   ; ecx = 0 ===> READ_ONLY                 
    int 0x80                        
    mov dword [infile_descriptor], eax         
    popad
    jmp check_command

prepare_output:
    pushad
    mov eax, 5    ; sys_open                 
    mov ebx, ecx  ; -o{file}
    inc ebx
    inc ebx
    mov ecx, 1 ; O_WRONLY
    or ecx, 64   ; (O_WRONLY | O_CREAT)
    mov edx, 777o ; Permission (write)
    int 0x80  
    mov dword [outfile_descriptor], eax        
    popad
    jmp check_command

main:
    mov edi, dword [esp+8]  ; argv
    mov esi, dword [esp+4]  ; argc
    xor edx, edx ; counter = 0
printing_loop:
    loop_body:
    mov ecx, dword [edi+edx*4] ; what to print
    ; print argv[i], get length then print
    print_string:
    pushad
    mov ebx, 1 ; file descriptor = stdout = 1
    mov ecx, dword [edi+edx*4] ; load string to print
    push ecx
    call strlen ; get length of string
    pop ecx
    mov edx, eax ;; length
    mov eax, 4 ;; write syscall
    int 0x80

    ; print newline
    mov eax, 4
    mov ebx, 1
    mov ecx, newline
    mov edx, 1
    int 0x80
    popad

    check_input:
        cmp word [ecx], "-i"            
        je prepare_input   
    check_output:              
        cmp word [ecx], "-o"          
        je prepare_output

check_command:
    inc edx ;; counter += 1
    cmp edx, esi
    jne loop_body

encoder_loop:
    read_char:
    mov eax, 3                ; sys_read
    mov ebx, dword [infile_descriptor]                ; stdin
    mov ecx, input_buffer            ; where to store char
    mov edx, 1                ; number of bytes to read
    int 0x80                  ; syscall

    check_eof:   ; eof entered ==> exit
    sub eax, 0
    jle exit_program

    check_range:
    ; check if character is between 'A' - 'z'
    movzx eax, byte [input_buffer]
    cmp eax, 'A'
    jl not_in_range
    cmp eax, 'z'
    jg not_in_range

    inc byte [input_buffer]

    print_character:
    mov eax, 4
    mov ebx, dword [outfile_descriptor] 
    mov ecx, input_buffer
    mov edx, 1
    int 0x80
    jmp encoder_loop

    not_in_range:
    jmp print_character              ; Jump to print the character

exit_program:
    mov eax, 1   ; exit syscall              
    mov ebx, 0   ; successful exit
    int 0x80