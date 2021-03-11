.globl main
.text


.data
	stack: .space 400
	print_exp_msg: .asciiz "enter expression: "
	evaluation_msg: .asciiz "\nvalue: "
	input_error_msg: .asciiz "\nInputError: not a postfix expression"
	test_msg: .asciiz "\nxxx"
	input: .space 100
	


.text
	

main:
	li $v0, 4
	la $a0, print_exp_msg
	syscall

	li $v0, 8
	la $a0, input
	li $a1, 100
	syscall
	
	j evaluate

evaluate:
	li $t0, 0	#input counter
	li $t1, 0	#stack counter
	addi $t4, $t1, -1
	j while1

while1:

	lb $s0, input($t0)	#character input
	beq $s0, 10, EOwhile1
	beq $s0, $0, EOwhile1	#end while loop if input[i] == '\0'

	li $t2, 48		#t2 is 1 if s0<'0'
	slt $t2, $s0, $t2
	
	li $t3, 57		#t3 is 1 if s0>'9'
	sgt $t3, $s0, $t3

	or $t2, $t2, $t3


	beq $t2, $0, isOperandTrue



	li $t2, 4		#if $t1 is 0 or 4 input_error
	beq $t1, $t2, input_error
	beqz $t1, input_error

	addi $t2, $t1, -4
	lw $t8, stack($t2)	#t8 stores stack.top() (= n1)
	
	sw $0, stack($t2)	#stack.pop()
	addi $t1, $t1, -4
	addi $t2, $t1, -4

	lw $t9, stack($t2)	#t9 stores stack.top() (= n2)
	
	sw $0, stack($t2)	#stack.pop()
	addi $t1, $t1, -4

	
	li $t2, 43
	beq $s0, $t2, add_op	#checks if char in $s0 is '+'

	li $t2, 45
	beq $s0, $t2, sub_op	#checks if char in $s0 is '-'

	li $t2, 42
	beq $s0, $t2, mult_op	#checks if char in $s0 is '*'

	li $t2, 47
	beq $s0, $t2, div_op	#checks if char in $s0 is '/'

	li $t2, 37
	beq $s0, $t2, mod_op	#checks if char in $s0 is '%'

	j input_error

add_op:

	add $t7, $t8, $t9	#adds n1, n2 and stores in $t7
	sw $t7, stack($t1)	#stores result in stack
	addi $t1, $t1, 4		#increments stack counter
	j while1_update		#continue loop

sub_op:
	sub $t7, $t9, $t8	#subtracts n2, n1
	sw $t7, stack($t1)	
	addi $t1, $t1, 4
	j while1_update
	
mult_op:
	mul $t7, $t8, $t9	#product of n2, n1
	sw $t7, stack($t1)
	addi $t1, $t1, 4
	j while1_update

div_op:
	div $t9, $t8		#quotient --> n2/n1
	mflo $t7
	sw $t7, stack($t1)
	addi $t1, $t1, 4
	j while1_update

mod_op:
	div $t9, $t8		#remainder --> n2%n1
	mfhi $t7
	sw $t7, stack($t1)
	addi $t1, $t1, 4
	j while1_update
	
	


#case when input character is a digit
isOperandTrue:
	move $t2, $s0		
	addi $t2, $t2, -48
	sw $t2, stack($t1)	#stores input character to stack
	addi $t1, $t1, 4		#increments stack counter
	j while1_update



while1_update:
	addi $t0, $t0, 1		#condition for updating loop
	j while1

input_error:
	li $v0, 4		
	la $a0, input_error_msg	#error message if string is not a valid postfix expression
	syscall
	j end


##if stack contains zero or more than one integers when while-loop is done, raise error
EOwhile1:
	li $t2, 4
	beqz $t1, input_error
	bne $t1, $t2, input_error

	li $v0, 4
	la $a0, evaluation_msg
	syscall			#print message "value: "
	
	li $v0, 1
	lw $a0, stack($0)	#print last integer in stack
	syscall

	lw $0, stack($0)
	li $t1, 0		#empty stack
	j end			#jump to end

end:
	li $v0, 10		
	syscall			#exit
	
	

	