#include "main.hpp"
#include "HelperFunctions.hpp"

vector<Registers> CPU_List;

void Registers::tokenize(string s){
    instruction ins;
    if(s.empty())return; // It might be the case that we have an empty line <- If we use labels, then we can safely ignore empty lines
    int i = 0;
    string fun="";
    
    // cout << s << endl;

    int j=0;
    while(j<s.length() && s[j]!=':')j++;

    if(j<s.length()){
        ins.fun_label = strip(s.substr(0,j));
        if(ins.fun_label==""){flag=true;return;}
        label[ins.fun_label] = instr.size();
        instr.pb(ins);
        // cout << s << "   <- Label\n";
        // cout << "->" << s.substr(j+1,s.size()-j-1) << "<-Label\n";
        tokenize(s.substr(j+1,s.size()-j-1));
        return;
    }

    while(i<s.length() && (s[i] == ' ' || s[i] == '\t'))i++;
    while(i<s.length() && s[i]!=' ' && s[i]!='\t'){fun+=s.at(i);i++;}
    // All labels are converted to the form
    /*
        label:
                ......       
                ......
    */  
    while(i<s.length() && (s[i] == ' ' || s[i] == '\t'))i++;

    if(fun.size() == 0)return;
    if(i == s.size()){flag=true;print_msg = "ERROR : Incorrect Instruction\n"; return;}
    ins.original = strip(s);
    s = s.substr(i,s.size()-i);
    
    vector<string> tokens;
    boost::split(tokens,s,boost::is_any_of(","));
    
    if(tokens.size()>3){flag=true;print_msg = "ERROR : Too many Arguments\n"; return;}
    for(int i=0;i<tokens.size();i++){        
        tokens[i] = strip(tokens[i]);
        if(flag)return;
    }
    
    transform(fun.begin(),fun.end(),fun.begin(),::tolower);
    ins.op = fun;
    
    if(fun == "add" || fun == "sub" || fun == "mul" || fun == "slt" || fun == "addi"){
        if(tokens.size()!=3){flag=true;print_msg = "ERROR : Incorrect number of Registers\n"; return;}
        ins.target  = tokens[0];
        ins.source1 = tokens[1];
        ins.source2 = tokens[2];    
    }
    else if(fun == "beq" || fun == "bne"){
        if(tokens.size()!=3){flag=true;print_msg = "ERROR : Incorrect number of Registers\n";return;}
        ins.source1 = tokens[0];
        ins.source2 = tokens[1];
        ins.jump = tokens[2];
    }
    else if(fun == "j"){
        if(tokens.size()!=1){flag=true;print_msg = "ERROR : Incorrect number of Registers\n";return;}
        ins.jump = tokens[0];    
    }
    else if(fun == "lw" || fun == "sw"){
        if(tokens.size()!=2){flag=true;print_msg = "ERROR : Incorrect number of Registers\n";return;}
        ins.target = tokens[0];
        i=0;
        s="";
        while(i<tokens[1].size() && tokens[1][i]!='(' && (tokens[1][i] - '0' <= 9) && (tokens[1][i] - '0' >= 0) ){s+=tokens[1][i];i++;}
        while(i<tokens[1].size() && (tokens[1][i]==' ' || tokens[1][i]=='\t'))i++;
        if(i == tokens[1].size() || tokens[1][i]!='('){flag=true;print_msg = "ERROR : Syntax Error\n";return;}
        ins.offset = s;
        ins.source1 = tokens[1].substr(i+1,tokens[1].size() - i - 2);
    }
    else{
        flag = true;
        print_msg = "ERROR : Operation not Defined\n";
        return;
    }
    
    instr.pb(ins);
}

void Registers::ResolveDependency(instruction &ins){
    
    if(ins.dependent[0].ff == -2)return; // Instruction is already executed
    for (int i=0;i<4;i++){
        if(ins.dependent[i].ff != -1){
            ResolveDependency(QueueInstruction[ins.dependent[i].ss]);
        }
    }
    if(ins.dependent[0].ff == -2)return; // Instruction is already executed
    // After the above for loop, we have successfully executed all the Dependent instructions
    // Now, we have to execute the current lw/sw instruction

    int x = stoi(ins.offset) + reg[ins.source1];    
    int RowNumber = (int)x/ColumnMemory;
    int ColumnNumber = x - (RowNumber*ColumnMemory);       
    ins.row = RowNumber;
    
    PrintCommand pr;
    pr.Start = pr.End = ins.InstructionRead;
    pr.Command = ins.original;
    pr.Execution = "DRAM: Request Issued";
    Command.push_back(pr);
    pr.Execution = "";
    
    if(RowBuffer == ins.row){
        // cout << ins.original << "  " << ins.InstructionRead << "  " << EndTime << endl;
        pr.Start = max(ins.InstructionRead + 1, EndTime+1);
        pr.End = pr.Start + ColumnDelay - 1;
        pr.Command = "DRAM: Column Access (" + to_string(ColumnNumber) + ")" + " (Ins: " + ins.original + ")";                        
        if(ins.op == "lw"){
            pr.Execution = ins.target + " = " + to_string(Dram[RowNumber][ColumnNumber]);
        }
        else{
            pr.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(reg[ins.target]);
        }
        Command.push_back(pr);
        EndTime = pr.End;
        // NumberOfCycles = pr.End;
        // StartTime = NumberOfCycles+2;
        // EndTime = NumberOfCycles + 1 + ColumnDelay;
    }

    else if(RowBuffer == -1){ // have to initiate the process
        
        RowBufferUpdates++;
        
        // StartTime = NumberOfCycles + 2;
        // EndTime = NumberOfCycles + 1 + RowDelay + ColumnDelay;
        pr.Start = ins.InstructionRead + 1;
        pr.End = pr.Start + RowDelay - 1;

        if(to_string(ColumnNumber).size() == 4)
        pr.Command = "DRAM: Activate row " + to_string(ins.row) + "       (Ins: " + ins.original + ")";
        if(to_string(ColumnNumber).size() == 3)
        pr.Command = "DRAM: Activate row " + to_string(ins.row) + "      (Ins: " + ins.original + ")";
        if(to_string(ColumnNumber).size() == 2)
        pr.Command = "DRAM: Activate row " + to_string(ins.row) + "     (Ins: " + ins.original + ")";
        if(to_string(ColumnNumber).size() == 1)
        pr.Command = "DRAM: Activate row " + to_string(ins.row) + "    (Ins: " + ins.original + ")";
        
        Command.push_back(pr);
        pr.Start = pr.End + 1;
        pr.End = pr.Start + ColumnDelay -1;
        pr.Command = "DRAM: Column Access (" + to_string(ColumnNumber) + ")" + " (Ins: " + ins.original + ")";                        
        if(ins.op == "lw"){
            pr.Execution = ins.target + " = " + to_string(Dram[RowNumber][ColumnNumber]);
        }
        else{
            pr.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(reg[ins.target]);
        }
        Command.push_back(pr);
        RowBuffer = ins.row;
        RowBufferCopy = Dram[ins.row];
        EndTime = pr.End;
        // NumberOfCycles = pr.End;
    }

    else{
        // ins.row is the New Row
        // RowBuffer is the one which is in the Buffer
        RowBufferUpdates++;
        if(RowBufferCopy == Dram[RowBuffer] && DesignDecision){
            // No need to writeback if there is no updation in the row
            
            // StartTime = NumberOfCycles + 2;
            // EndTime = NumberOfCycles + 1 + RowDelay + ColumnDelay;

            // cout << "HEYYYYYYYYYYYYYYYYYYY\n" << ins.row << " " << RowBuffer << endl; 

            pr.Start = max(ins.InstructionRead + 1, EndTime+1);
            pr.End = pr.Start + RowDelay - 1;

            if(to_string(ColumnNumber).size() == 4)
            pr.Command = "DRAM: Activate row " + to_string(ins.row) + "       (Ins: " + ins.original + ")";
            if(to_string(ColumnNumber).size() == 3)
            pr.Command = "DRAM: Activate row " + to_string(ins.row) + "      (Ins: " + ins.original + ")";
            if(to_string(ColumnNumber).size() == 2)
            pr.Command = "DRAM: Activate row " + to_string(ins.row) + "     (Ins: " + ins.original + ")";
            if(to_string(ColumnNumber).size() == 1)
            pr.Command = "DRAM: Activate row " + to_string(ins.row) + "    (Ins: " + ins.original + ")";
            
            Command.push_back(pr);
            
            pr.Start = pr.End + 1;
            pr.End = pr.Start + ColumnDelay - 1;
            pr.Command = "DRAM: Column Access (" + to_string(ColumnNumber) + ")" + " (Ins: " + ins.original + ")";                        
            
            if(ins.op == "lw")pr.Execution = ins.target + " = " + to_string(Dram[RowNumber][ColumnNumber]);
            else pr.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(reg[ins.target]);
            
            Command.push_back(pr);
            RowBuffer = ins.row;
            EndTime = pr.End;
        }
        else{
            // Some changes took place in the Row Buffer, hence, we need to writeback that row to the DRAM
            // StartTime = NumberOfCycles + 2;
            // EndTime = NumberOfCycles + 1 + RowDelay*2 + ColumnDelay;
            
            pr.Start = max(ins.InstructionRead + 1, EndTime+1);
            pr.End = pr.Start + RowDelay - 1;
            pr.Command = "DRAM: Writeback row " + to_string(RowBuffer);
            Command.push_back(pr);
            
            pr.Start = pr.End + 1;
            pr.End = pr.Start + RowDelay - 1;

            if(to_string(ColumnNumber).size() == 4)
            pr.Command = "DRAM: Activate row " + to_string(ins.row) + "       (Ins: " + ins.original + ")";
            if(to_string(ColumnNumber).size() == 3)
            pr.Command = "DRAM: Activate row " + to_string(ins.row) + "      (Ins: " + ins.original + ")";
            if(to_string(ColumnNumber).size() == 2)
            pr.Command = "DRAM: Activate row " + to_string(ins.row) + "     (Ins: " + ins.original + ")";
            if(to_string(ColumnNumber).size() == 1)
            pr.Command = "DRAM: Activate row " + to_string(ins.row) + "    (Ins: " + ins.original + ")";
            
            Command.push_back(pr);
            
            pr.Start = pr.End + 1;
            pr.End = pr.Start + ColumnDelay -1;
            pr.Command = "DRAM: Column Access (" + to_string(ColumnNumber) + ")" + " (Ins: " + ins.original + ")";                        
            
            if(ins.op == "lw")pr.Execution = ins.target + " = " + to_string(Dram[RowNumber][ColumnNumber]);
            else pr.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(reg[ins.target]);
            
            Command.push_back(pr);
            RowBuffer = ins.row;
            EndTime = pr.End;
        }
        // NumberOfCycles = pr.End;
    }

    if(ins.op == "lw"){
        RegisterUse[ins.source1] = mp(-1,-1);
        RegisterUpdation[ins.target] = mp(-1,-1);
        MemoryUse[x] = mp(-1,-1);
        reg[ins.target] = Dram[RowNumber][ColumnNumber];
    }
    else{
        RegisterUse[ins.source1] = mp(-1,-1);
        RegisterUse[ins.target] = mp(-1,-1);
        MemoryUpdation[x] = mp(-1,-1);
        // if(Dram[RowNumber][ColumnNumber]!=reg[ins.target])
        RowBufferUpdates++;
        Dram[RowNumber][ColumnNumber] = reg[ins.target];
    }
    // NumberOfCycles++;
    ins.dependent[0] = mp(-2,-2); // This implies the particular instruction in the vector is executed
    // cout << EndTime << endl;
    // for(int i=QueueInstruction.size()-1;i>=0;i--){
    //     if(QueueInstruction[i].dependent[0].ff == -2)continue;
    //     if(QueueInstruction[i].row == RowBuffer)ResolveDependency(QueueInstruction[i]);
    // }
    
    for(int i=0;i<QueueInstruction.size();i++){

        if(QueueInstruction[i].dependent[0].ff == -2)continue;
        
        for(int j=0;j<4;j++)if(QueueInstruction[i].dependent[j].ss>=0 && QueueInstruction[QueueInstruction[i].dependent[j].ss].dependent[0].ff == -2)QueueInstruction[i].dependent[j] = mp(-1,-1);
        
        if(QueueInstruction[i].row == RowBuffer){
            bool flag = true;
            for(int j=0;j<4;j++){
                if(QueueInstruction[i].dependent[0].ff != -1)flag = false;
            }
            if(flag)ResolveDependency(QueueInstruction[i]);
        }
        // if(QueueInstruction[i].row == RowBuffer && QueueInstruction[i].dependent[0].ff == -1)ResolveDependency(QueueInstruction[i]);
    }
    return;


}

void Registers::parse(){
    int i = 0;
    while(i<instr.size()){
        // cout << NumberOfCycles << endl;
        instruction ins = instr[i];
        instr[i].InstructionCount++;
        print_count++;
        ErrorLine = i;
        RegisterUpdation["$zero"] = mp(-1,-1); // Initializing the zero register's RegisterUpdation value to 0 in each loop
        RegisterUse["$zero"] = mp(-1,-1);      // Initializing the zero register's RegisterUse value to 0 in each loop

        // print_ins(ins);
        
        if(ins.op == "add" || ins.op == "sub" || ins.op == "mul" || ins.op == "slt"){

            if( reg.find(ins.target) == reg.end() || reg.find(ins.source1) == reg.end() || reg.find(ins.source2) == reg.end() ){
                print_msg = "ERROR : Unknown Register\n";
                flag = true;
                return;
            }
            if(ins.target == "$0" || ins.target == "$zero"){
                print_msg = "ERROR : Trying to change Const value\n";
                flag =true;
                return;
            }

            // To resolve the dependency
            if(RegisterUpdation[ins.target].ff >=0 || RegisterUse[ins.target].ff >=0 || RegisterUpdation[ins.source1].ff >=0 || RegisterUpdation[ins.source2].ff >=0 ){
                if(RegisterUpdation[ins.target].ff >=0)ResolveDependency(QueueInstruction[RegisterUpdation[ins.target].ss]);
                if(RegisterUse[ins.target].ff >=0)ResolveDependency(QueueInstruction[RegisterUse[ins.target].ss]);
                if(RegisterUpdation[ins.source1].ff >=0)ResolveDependency(QueueInstruction[RegisterUpdation[ins.source1].ss]);
                if(RegisterUpdation[ins.source2].ff >=0)ResolveDependency(QueueInstruction[RegisterUpdation[ins.source2].ss]);
            }
            // To resolve the dependency
            
            PrintCommand pr;
            pr.Start = NumberOfCycles + 1;
            pr.End = pr.Start;
            pr.Command = ins.original;
            ins.InstructionRead = pr.Start;
            
            if(ins.op == "add")reg[ins.target] = reg[ins.source1] + reg[ins.source2];
            if(ins.op == "sub")reg[ins.target] = reg[ins.source1] - reg[ins.source2];
            if(ins.op == "mul")reg[ins.target] = reg[ins.source1] * reg[ins.source2];
            if(ins.op == "slt"){
                if( reg[ins.source1] < reg[ins.source2] ) reg[ins.target] = 1;
                else reg[ins.target] = 0;
            }
            
            pr.Execution = ins.target + " = " + to_string(reg[ins.target]);
            Command.pb(pr);
            i++;
            NumberOfCycles++;
            
        }

        else if( ins.op == "beq" || ins.op == "bne"){
            if( reg.find(ins.source1) == reg.end() || reg.find(ins.source2) == reg.end()){
                flag = true;
                print_msg = "ERROR : Unknown Register\n";
                return;
            }
            if( label.find(ins.jump) == label.end() ){
                flag = true;
                print_msg = "ERROR : Unknown Function\n";
                return;
            }

            // To resolve the dependency
            if( RegisterUse[ins.source1].ff >=0 || RegisterUse[ins.source2].ff >=0){

                if(RegisterUpdation[ins.source1].ff >=0)ResolveDependency(QueueInstruction[RegisterUse[ins.source1].ss]);
                if(RegisterUpdation[ins.source2].ff >=0)ResolveDependency(QueueInstruction[RegisterUse[ins.source2].ss]);

            }
            // To resolve the dependency

            PrintCommand pr;
            pr.Start = NumberOfCycles+1;
            pr.End = pr.Start;
            pr.Command = ins.original;
            ins.InstructionRead = pr.Start;

            if(ins.op == "beq"){
                if( reg[ins.source1] == reg[ins.source2] ) {
                    i = label[ins.jump];
                    pr.Execution = "Values Matched : Jump to " + ins.jump;
                }
                else {i++; pr.Execution = "Values Mismatched : Move to next instruction";}
            }
            if( ins.op == "bne"){
                if( reg[ins.source1] != reg[ins.source2] ) {
                    i = label[ins.jump];
                    pr.Execution = "Values Mismatched : Jump to " + ins.jump;
                }
                else {i++; pr.Execution = "Values Matched : Move to next instruction";}
            }
            Command.pb(pr);
            NumberOfCycles++;
        }

        else if( ins.op == "j" ){
            if( label.find(ins.jump) == label.end()){
                // print_ins(ins);
                flag = true;
                print_msg = "ERROR : Unknown Function\n";
                return;
            }
            PrintCommand pr;
            pr.Start = NumberOfCycles + 1;
            pr.End = pr.Start;
            pr.Command = ins.original;
            pr.Execution = "Jump to " + ins.jump;
            Command.pb(pr);

            ins.InstructionRead = pr.Start;
            i = label[ins.jump];
            NumberOfCycles++;
        }

        else if(ins.op == "lw" || ins.op == "sw"){
            if( reg.find(ins.target) == reg.end() || reg.find(ins.source1) == reg.end() || !Registers::is_number(ins.offset)){
                flag = true;
                print_msg = "ERROR : Unknown Register\n";
                return;
            }
            int x = stoi(ins.offset) + reg[ins.source1];    
            int RowNumber = (int)x/ColumnMemory;
            int ColumnNumber = x - (RowNumber*ColumnMemory);       
            ins.row = RowNumber;
            if(x < MemoryAvailable && x%4!=0){
                flag = true;
                print_msg = "ERROR : Offset should be a multiple of 4\n";
                return;
            }
                        
            if(x >= MemoryAvailable){  // Equality because of 4 bytes after x
                flag = true;
                print_msg = "ERROR : Overflow\n";
                return;
            }
            
            for(int i=0;i<4;i++)ins.dependent.pb(mp(-1,-1)); // Initializing length to 4for(int i=0;i<4;i++)ins.dependent.pb(-1); // Initializing length to 4
            
            if(MemoryUpdation.find(x) == MemoryUpdation.end()) MemoryUpdation[x] = mp(-1,-1);
            if(MemoryUse.find(x) == MemoryUse.end()) MemoryUse[x] = mp(-1,-1);
            

            // Appropriate if-else blocks decide whether to execute the lw/sw instruction or store it in the Queue ("QueueInstruction")
            if(RowBuffer == -1 || RowBuffer != ins.row){

                // cout << ins.op << "\n";                
                if(ins.op == "lw"){

                    if(RegisterUpdation[ins.source1].ff >= 0 || MemoryUpdation[x].ff >=0 || RegisterUse[ins.target].ff >=0 || RegisterUpdation[ins.target].ff >= 0){
                        // This implies the current instruction is dependent on some instruction already present in the Queue
                        ins.dependent[0] = RegisterUpdation[ins.source1];                        
                        ins.dependent[1] = MemoryUpdation[x];
                        ins.dependent[2] = RegisterUse[ins.target];
                        ins.dependent[3] = RegisterUpdation[ins.target];
                    }

                    // RegisterUse[ins.source1] = mp(i,QueueInstruction.size());
                    RegisterUpdation[ins.target] = mp(i,QueueInstruction.size());
                    MemoryUse[x] = mp(i,QueueInstruction.size());
                }

                else{
                    
                    // For sw instruction
                    if(RegisterUpdation[ins.source1].ff >= 0 || MemoryUpdation[x].ff >=0 || MemoryUse[x].ff >=0 || RegisterUpdation[ins.target].ff >= 0){
                        ins.dependent[0] = RegisterUpdation[ins.source1];                                                
                        ins.dependent[1] = MemoryUpdation[x];                                                
                        ins.dependent[2] = MemoryUse[x];                        
                        ins.dependent[3] = RegisterUpdation[ins.target];
                    }

                    MemoryUpdation[x] = mp(i,QueueInstruction.size());
                    RegisterUse[ins.source1] = mp(i,QueueInstruction.size());
                    RegisterUse[ins.target] = mp(i,QueueInstruction.size());

                }

                ins.InstructionRead = NumberOfCycles+1;
                NumberOfCycles++;
                QueueInstruction.pb(ins);
                
                // DEBUGGING
                // print_ins(QueueInstruction[QueueInstruction.size()-1]);
                // cout << "Memory Updation  " << x << " : " ; printp(MemoryUpdation[x]); cout << endl;
                // cout << "Memory Use " << x << " : " ; printp(MemoryUse[x]); cout << endl << endl;
                
                // for(int i=QueueInstruction.size()-1;i>=QueueInstruction.size()-1;i--){
                //     // cout << (QueueInstruction.size()) << endl;
                //     // print_ins(QueueInstruction[i]);
                //     cout << i << endl;
                //     // ResolveDependency(QueueInstruction[i]);
                // }


            }  

            else{
                // This else block implies the current lw/sw instruction access the same row as in Buffer.
                if(ins.op == "lw"){

                    if(RegisterUpdation[ins.source1].ff >= 0 || MemoryUpdation[x].ff >=0 || RegisterUse[ins.target].ff >=0 || RegisterUpdation[ins.target].ff >= 0){
                        // This implies the current instruction is dependent on some instruction already present in the Queue
                        ins.dependent[0] = RegisterUpdation[ins.source1];
                        ins.dependent[1] = MemoryUpdation[x];
                        ins.dependent[2] = RegisterUse[ins.target];
                        ins.dependent[3] = RegisterUpdation[ins.target];
                        
                        RegisterUse[ins.source1] = mp(i,QueueInstruction.size());
                        RegisterUpdation[ins.target] = mp(i,QueueInstruction.size());
                        MemoryUse[x] = mp(i,QueueInstruction.size());
                        ins.InstructionRead = NumberOfCycles+1;
                        NumberOfCycles++;
                        QueueInstruction.pb(ins);
                    }
                    else{
                        // Execute the command 
                        PrintCommand pr;

                        pr.Start = pr.End = NumberOfCycles + 1;
                        pr.Command = ins.original;
                        pr.Execution = "DRAM: Request Issued";
                        Command.pb(pr);

                        pr.Execution = "";
                        pr.Start = max(NumberOfCycles+2,EndTime+1);
                        pr.End = pr.Start + ColumnDelay - 1;
                        pr.Command = "DRAM: Column Access (" + to_string(ColumnNumber) + ")" + " (Ins: " + ins.original + ")";                                        
                        pr.Execution = ins.target + " = " + to_string(Dram[RowNumber][ColumnNumber]);
                        reg[ins.target] = Dram[RowNumber][ColumnNumber];
                        Command.pb(pr);
                        // NumberOfCycles = pr.End;
                        EndTime = pr.End;
                        NumberOfCycles++;
                    }

                }

                else{
                    
                    // sw instruction
                    if(RegisterUpdation[ins.source1].ff >= 0 || MemoryUpdation[x].ff >=0 || MemoryUse[x].ff >=0 || RegisterUpdation[ins.target].ff >= 0){
                        ins.dependent[0] = RegisterUpdation[ins.source1];
                        ins.dependent[1] = MemoryUpdation[x];
                        ins.dependent[2] = MemoryUse[x];
                        ins.dependent[3] = RegisterUpdation[ins.target];

                        MemoryUpdation[x] = mp(i,QueueInstruction.size());
                        RegisterUse[ins.source1] = mp(i,QueueInstruction.size());
                        RegisterUse[ins.target] = mp(i,QueueInstruction.size());
                        ins.InstructionRead = NumberOfCycles+1;
                        NumberOfCycles++;
                        QueueInstruction.pb(ins);
                    }
                    else{
                        // Execute the command
                        PrintCommand pr;
                        pr.Start = pr.End = NumberOfCycles + 1;
                        pr.Command = ins.original;
                        pr.Execution = "DRAM: Request Issued";
                        Command.pb(pr);

                        pr.Execution = "";
                        pr.Start = max(NumberOfCycles + 2,EndTime+1);
                        pr.End = pr.Start + ColumnDelay - 1;
                        pr.Command = "DRAM: Column Access (" + to_string(ColumnNumber) + ")" + " (Ins: " + ins.original + ")";                                             
                        pr.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(reg[ins.target]);

                        // if(Dram[RowNumber][ColumnNumber] != reg[ins.target])
                        RowBufferUpdates++;
                        Dram[RowNumber][ColumnNumber] = reg[ins.target];
                        Command.pb(pr);
                        // NumberOfCycles = pr.End;
                        EndTime = pr.End;
                        NumberOfCycles++;
                    }
                }

            }      
            i++;        
        
        }

        else if(ins.op == "addi"){
            if( reg.find(ins.target) == reg.end()){
                flag = true;
                print_msg = "ERROR : Unknown Register\n";
                return;
            }
            if(ins.target == "$0" || ins.target == "$zero"){
                print_msg = "ERROR : Trying to change Const value\n";
                flag =true;
                return;
            }
            int x = 0, y = 0;

            // cout << ins.source1 << " " << ins.source2 << " " ; printp(RegisterUse[ins.source1]) ; cout <<  endl;

            if(ins.source1[0] == '$' && reg.find(ins.source1) == reg.end()){
                flag = true;
                print_msg = "ERROR : Unknown Register\n";
                return;
            }            
            else if(ins.source1[0]!='$'){
                flag = !Registers::is_number(ins.source1);
                if(flag){print_msg = "ERROR : Not a number\n"; return;}
                x = stoi(ins.source1);
            }
            else{  // First symbol is $ and is present in Register file
                x = reg[ins.source1];
                if(RegisterUpdation[ins.source1].ff >=0){
                    ResolveDependency(QueueInstruction[RegisterUpdation[ins.source1].ss]);
                    // continue;
                }
            }

            // cout << ins.source1 << " " << ins.source2 << endl;
            
            if(ins.source2[0] == '$' && reg.find(ins.source2) == reg.end()){
                flag = true;
                print_msg = "ERROR : Unknown Register\n";
                return;
            }            
            else if(ins.source2[0]!='$'){
                
                flag = !Registers::is_number(ins.source2);
                if(flag){print_msg = "ERROR : Not a number\n"; return;}                
                y = stoi(ins.source2);
                
            }
            else{  // First symbol is $ and is present in Register file
                y = reg[ins.source2];                
                if(RegisterUpdation[ins.source2].ff >=0){
                    ResolveDependency(QueueInstruction[RegisterUpdation[ins.source2].ss]);
                    // continue;
                }
            }
            // cout << ins.source1 << " " << ins.source2 << endl;

            // cout << "Hey  "  << ins.target << "  " <<  NumberOfCycles <<   "\n" ;

            
            
            // To resolve the dependency            
            
            if(RegisterUpdation[ins.target].ff >=0 || RegisterUse[ins.target].ff >=0){
                // cout << ins.source1 << " " << ins.source2 << endl;
                // cout << "Hey\n";
                if(RegisterUpdation[ins.target].ff >= 0)ResolveDependency(QueueInstruction[RegisterUpdation[ins.target].ss]);
                if(RegisterUse[ins.target].ff >= 0)ResolveDependency(QueueInstruction[RegisterUse[ins.target].ss]);
                // continue;
            }

            // To resolve the dependency

            PrintCommand pr;
            pr.Start = NumberOfCycles + 1;
            pr.End = pr.Start;
            pr.Command = ins.original;
            reg[ins.target] = x+y;
            pr.Execution = ins.target + " = " + to_string(reg[ins.target]);
            Command.pb(pr);
            
            i++;
            NumberOfCycles++;
        }
        
        else {i++;print_count--;} //  This is for the labels like "main:" or "loop:" which will not be covered in any if the cases above and nor the NumberOfCycles will be incremented on this  


        // DEBUGGING
        // if(ins.op!="")
        // {
        //     cout << "Instruction : " << ins.original << endl;
        //     print_reg();
        //     // print_ins(ins);
        //     print_count++;
        // }

    }
}

bool openFile(string path){
    ifstream infile(path);

    if(infile.is_open()){
        Registers r;
        CPU_List.pb(r);
        CPU_List[CPU_List.size()-1].std_registers();
        int ln = 1;
        for(string line;getline(infile, line);){
            CPU_List[CPU_List.size()-1].tokenize(line);
            if(flag){
                cout << CPU_List[CPU_List.size()-1].print_msg;
                cout << "FILE NAME: " << path << "\n";
                cout << "Line Number: " << ln << ": "<< line << endl;
                return false;
            }
            ln++;
        }
        if(ln==1){
            cout << "FILE NAME: " << path << "\n";
            cout << "EMPTY FILE\n";
            return 0;
        }
    }
    else{
        cout << "FILE \"" << path << "\" NOT FOUND\n";
        return false;   
    }
    bool result = CPU_List[CPU_List.size()-1].AllotMemory();
    return result;
}

bool Registers::ParsingExecution(string File){
    parse();
    if(flag){
        cout << print_msg;
        cout << "FILE NAME: " << File << "\n";
        cout << "LINE NUMBER: " << ErrorLine << ": " << instr[ErrorLine].original << endl;
        return false;
    }
    for(int i=QueueInstruction.size()-1; i>=0; i--){
        ResolveDependency(QueueInstruction[i]);
    }
    sort(Command.begin(),Command.end(),Registers::Comparator);
    if(RowBuffer != -1 && ( RowBufferCopy != Dram[RowBuffer] || !DesignDecision)){
        PrintCommand pr;
        pr.Start = Command[Command.size()-1].End + 1;
        pr.End = pr.Start + RowDelay - 1;
        pr.Command = "DRAM: Writeback row " + to_string(RowBuffer);
        Command.pb(pr);
    }
    return true;
}



int main(int argc, char * argv[]){
    if(argc >= 2 && argc <= 4){
        
        if(!Registers::is_number(argv[1])){
            cout << "INVALID INPUT: Number of Processors should be an Integer\n";
            return -1;
        }
        
        if(argc >= 3){
            if(!Registers::is_number(argv[2])){
                cout << "INVALID INPUT: ROW_ACCESS_DELAY should be an integer\n";
                return -1;
            }
            RowDelay = stoi(argv[2]);
        }

        if(argc == 4){
            if(!Registers::is_number(argv[3])){
                cout << "INVALID INPUT: COLUMN_ACCESS_DELAY should be an integer\n";
                return -1;
            }
            ColumnDelay = stoi(argv[3]);
        }        

        CPU = stoi(argv[1]);

        // This loop ensures that all files are read and tokenized correctly
        for(int i=0; i<CPU; i++){
            string filename = BaseFilename + to_string(i+1);
            bool result = openFile(filename);
            if(!result)return -1;
        }
        
        for(int i=0; i < CPU; i++){
            CPU_List[i].ParsingExecution(BaseFilename + to_string(i+1));
            CPU_List[i].Printing();
        }

    }
    else cout << "INCORRECT INPUT\n";
}
