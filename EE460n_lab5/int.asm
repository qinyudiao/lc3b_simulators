    .ORIG x1200
    ADD R6, R6, #-2 ;push R1,R2
    STW R1, R6, #0
    ADD R6, R6, #-2
    STW R2, R6, #0
    LEA R1, A
    LDW R1, R1, #0 ;R1 = x4000
    LDW R2, R1, #0 ;R2 = MEM[x4000]
    ADD R2, R2, #1
    STW R2, R1, #0
    LDW R2, R6, #0 ;pop R2,R1
    ADD R6, R6, #2
    LDW R1, R6, #0
    ADD R6, R6, #2
    RTI
A   .FILL x4000
    .END
