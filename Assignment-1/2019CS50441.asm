    .text
    .globl main
main:
    li $v0,4
    la $a0,msg1                 # Will ask for number of points i.e. N
    syscall           
        
    li $v0,5                    # Input (N)
    syscall

    move $t0,$v0                # t0 holds the number of points (N)
    li $t1,0                    # counter for number of points
    li.s $f1,0.0                # Net Area
    li.s $f2,0.0                # Will store current_x
    li.s $f3,0.0                # Will store current_y
    li.s $f4,0.0                # Will store prev_x
    li.s $f5,0.0                # Will store prev_y
    li.s $f6,2.0                # Constant to divide area by 2
    li.s $f7,0.0                # Constant 0 to check sign 
    li.s $f8,0.0                # To store y2-y1 (These are for area calculation if y1*y2<0)
    li.s $f9,0.0                # To store y1^2 + y2^2
    li $t3,0                    # To check sign of $f3 and $f5  t3==1 implies I am in the case of y1*y2<0

loop:   beq $t1, $t0, exit
    
        mov.s $f4,$f2           # Storing previous x in f4
        mov.s $f5,$f3           # Storing previous y in f5
        li $t3,0         
            
        li $v0,4        
        la $a0,give_x           # Asking for x
        syscall        
        
        li $v0,6                # Take float as an input
        syscall        
        
        mov.s $f2,$f0           # Storing x-coord in $f2
        li $v0,4        
        la $a0,give_y           # Asking for y
        syscall
        
        li $v0,6
        syscall        
        mov.s $f3,$f0           # Storing y-coord in $f3 
        



        # li $v0,2  # For debugging
        # mov.s $f12,$f1
        # syscall
        # li $v0,4
        # la $a0,next_line
        # syscall






        c.lt.s $f3,$f7,         # Checking the sign of current_y
        bc1t A_                 # If current_y < 0, jump to A_
        add $t3,$t3,1           # This will be skipped if f3 is negative
    
    A_: addi $t1,$t1,1          # Counter for number of points seen
        
        c.lt.s $f5,$f7          # Checking the sign of prev_y
        bc1t B_                 # If prev_y < 0, jump to B_
        add $t3,$t3,1           # This will be skipped if f3 is negative        
    B_: beq $t1,1,loop          # Will ask for input if only one point has been seen so far
        beq $t3,1,ELSE_         # if $t3==1, we are in the case of y1*y2 < 0, Will jump to ELSE_ i.e. area_mod
        jal area                # Will read this line if t3==0 or t3==2 i.e. y1*y2 > 0
        j C_                    
    ELSE_: jal area_mod         # area_mod is the function to compute area in the case of y1*y2 < 0
    C_: bne $t1, $t0, loop      # Will terminate if t1==t0
        j   exit
        
    
area_mod:
    # Area = abs((x1-x2)*(y1^2+y2^2)/((y2-y1)*2)) (y1*y2 < 0)
    # Here, results will be stored in $f4 

    sub.s $f4,$f2,$f4            # for x1-x2
    sub.s $f8,$f5,$f3            # for y2-y1
    
    c.eq.s $f8,$f7               # checking if denominator = 0
    bc1t D_                      # If yes, jumping to return statement



    # li $v0,2                   # For debugging
    # mov.s $f12,$f1
    # syscall
    # li $v0,4
    # la $a0,next_line
    # syscall


    
    mul.s $f9,$f3,$f3            # y1^2
    mul.s $f5,$f5,$f5            # y2^2
    add.s $f9,$f9,$f5            # y1^2 + y2^2
    div.s $f4,$f4,$f8            # computing (x1-x2)/(y2-y1)
    div.s $f4,$f4,$f6            # computing (x1-x2)/((y2-y1)*2)  
    mul.s $f4,$f4,$f9            # computing (x1-x2)*(y1^2+y2^2)/((y2-y1)*2)  
    abs.s $f4,$f4                # Taking mod of the area
    add.s $f1,$f1,$f4            # Updating the total area



   # li $v0,2                    # For debugging
   # mov.s $f12,$f1
   # syscall



    D_: jr $ra                   # Return statement


area:
    # Area = abs((x1 - x2)*(y1 + y2)/2)
    add.s $f5,$f5,$f3            # y1 + y2 (Sum of parallel sides)
    sub.s $f4,$f2,$f4            # x1 - x2 (Height of the trapezium)
    div.s $f4,$f4,$f6            # (x1 - x2)/2
    mul.s $f4,$f4,$f5            # (x1 - x2)*(y1 + y2)/2
    abs.s $f4,$f4                # Taking absolute value
    add.s $f1,$f1,$f4            # Updating the total area

    # li $v0,2                   # For debugging
    # mov.s $f12,$f1
    # syscall
 
    jr $ra                       # Return statement

exit:

    # li $v0,1
    # move $a0,$t1
    # syscall

    li $v0,4
    la $a0,area_                 # Will print "Area = "
    syscall

    li $v0,2 
    mov.s $f12,$f1               # Will print the value of total Area
    syscall

    li $v0,4
    la $a0,next_line
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
