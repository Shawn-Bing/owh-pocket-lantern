; Static excerpts from ow.unpacked.bin. Addresses use image base 0x140000000.

; RANGE 0x1400014e0..0x1400015ac (end exclusive)
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

; RANGE 0x1400021ac..0x14000231c (end exclusive)
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

; RANGE 0x1400026ec..0x140002c28 (end exclusive)
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

; RANGE 0x140002cc4..0x140002d33 (end exclusive)
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

; RANGE 0x140003770..0x140003950 (end exclusive)
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

; RANGE 0x140004271..0x1400042a4 (end exclusive)
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

; RANGE 0x1400043c6..0x1400044df (end exclusive)
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
