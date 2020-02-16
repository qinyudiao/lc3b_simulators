	    .ORIG x3000
        LEA R2, dest;     R2 = address of x3050
        LDW R2, R2, #0; R2 = x3050, the destination location to store the result
        STB R0, R2, #0; x3050 <- R0[7:0]
        RSHFL R0, R0, #8; 
        STB R0, R2, #1; x3051 <- R0[15:8]
        LDB R0, R2, #0;
        LDB R1, R2, #1;
        ADD R0, R0, R1;
        STW R0, R2, #0;
 dest   .FILL x3050
        .END
        
 