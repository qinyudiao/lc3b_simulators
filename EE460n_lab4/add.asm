    .ORIG x3000

    LEA R0, A
    LDW R0, R0, #0 ;R0 = x4000
    AND R1, R1, #0 ;R1 = 0
    ADD R1, R1, #1 ;R1 = 1
    STW R1, R0, #0 ;initialize x4000 to 1
    LEA R0, C
    LDW R0, R0, #0 ;R0 = xC000
    AND R1, R1, #0 ;R1 = 0 for sum
    LEA R2, TT
    LDW R2, R2, #0 ;R2 = 20
L   LDB R3, R0, #0 ;R3 is temp
    ADD R1, R1, R3 
    ADD R0, R0, #1 ;increment memory address
    ADD R2, R2, #-1 
    BRp L

    LEA R3, C
    ADD R3, R3, #1
    LDW R4, R3, #0
    

    STW R1, R0, #0 ; Done, store 52 at xC014
    HALT
A   .FILL x4000
C   .FILL xC000
TT  .FILL #20
    .END
