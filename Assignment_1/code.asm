    .text
    .globl main
main:
    li $v0,4
    la $a0,msg1
    syscall

    li $v0,5
    syscall

    move $t0,$v0 # t0 holds the number of points
    li $t1,0  # counter for number of points
    li.s $f1,0.0 # Net Area
    li.s $f2,0.0 # Will store current_x
    li.s $f3,0.0 # Will store current_y
    li.s $f4,0.0 # Will store prev_x
    li.s $f5,0.0 # Will store prev_y
    li.s $f6,2.0 # Constant to divide area by 2
    li.s $f7,0.0 # Constant 0
    li.s $f8,0.0 # To store y2-y1
    li.s $f9,0.0 # To store y1^2 + y2^2
    li $t3,0 # To check sign of $f3 and $f5

loop:   beq $t1, $t0, exit
    
        mov.s $f4,$f2
        mov.s $f5,$f3
        li $t3,0
    
        li $v0,4
        la $a0,give_x
        syscall

        li $v0,6 # Take float as an input
        syscall

        mov.s $f2,$f0 
        li $v0,4
        la $a0,give_y
        syscall
        
        li $v0,6
        syscall        
        mov.s $f3,$f0  # Storing y-coord in $f3 
        



        # li $v0,2  # For debugging
        # mov.s $f12,$f1
        # syscall
        # li $v0,4
        # la $a0,next_line
        # syscall






        c.lt.s $f3,$f7,
        bc1t A_          # Current_y < 0
        add $t3,$t3,1 # This will be skipped if f3 is negative
    A_: addi $t1,$t1,1
        c.lt.s $f5,$f7
        bc1t B_          # prev_y < 0
        add $t3,$t3,1 # This will be skipped if f3 is negative        
    B_:  beq $t1,1,loop
        beq $t3,1,ELSE_ # if $t3==1, 
        jal area
        j C_
    ELSE_: jal area_mod         
    C_: bne $t1, $t0, loop
        j   exit
        
    
area_mod:
    # area will be (x1-x2)*(y1^2+y2^2)/((y2-y1)*2)
    # Here, results will be stored in $f4 (step by step)
    sub.s $f4,$f2,$f4  # for x1-x2
    sub.s $f8,$f5,$f3  # for y2-y1
    
    c.eq.s $f8,$f7  # checking if denom=0
    bc1t D_


    # li $v0,2  # For debugging
    # mov.s $f12,$f1
    # syscall
    # li $v0,4
    # la $a0,next_line
    # syscall


    
    mul.s $f9,$f3,$f3  # y1^2
    mul.s $f5,$f5,$f5  # y2^2
    add.s $f9,$f9,$f5  # y1^2 + y2^2
    mul.s $f4,$f4,$f9  # computing (x1-x2)*(y1^2+y2^2)
    div.s $f4,$f4,$f8  # computing (x1-x2)*(y1^2+y2^2)/((y2-y1))  
    div.s $f4,$f4,$f6  # computing (x1-x2)*(y1^2+y2^2)/((y2-y1)*2)  
    abs.s $f4,$f4
    add.s $f1,$f1,$f4
    
   # li $v0,2
   # mov.s $f12,$f1
   # syscall

    D_: jr $ra


area:
    add.s $f5,$f5,$f3 # summing 2 parallel sides
    sub.s $f4,$f2,$f4 # height of my trapezium
    mul.s $f4,$f4,$f5
    div.s $f4,$f4,$f6
    abs.s $f4,$f4
    add.s $f1,$f1,$f4    

    # li $v0,2
    # mov.s $f12,$f1
    # syscall

    jr $ra    

exit:

    # li $v0,1
    # move $a0,$t1
    # syscall

    li $v0,4
    la $a0,area_
    syscall

    li $v0,2
    mov.s $f12,$f1
    syscall

    li $v0,4
    la $a0,next_line
    syscall

    li $v0,10
    syscall

    .data
msg1: .asciiz "Enter the Number of points(N): "
next_line: .asciiz "\n"
give_x: .asciiz "x = "
give_y: .asciiz "y = "
area_: .asciiz "Area = "