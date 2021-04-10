all: code_Block.cpp
	g++ code_Block.cpp -o code_Block
	g++ code_NonBlock.cpp -o code_NonBlock

exe: 
	./code_Block input > out_Block
	./code_NonBlock input > out_NonBlock

clean:
	rm -rf code_Block
	rm -rf code_NonBlock
	rm -rf out_Block
	rm -rf out_NonBlock