#include "main.hpp"
#include "HelperFunctions.hpp"


vector<PrintCommand> output;
vector<instruction> MemoryManager;

int InstructionCount = 0; // Throughput

void Registers::CheckDependency_add(instruction &Current_ins, int maxIndexTocheck)
{
    if(QueueInstruction_new.size() == 0 || maxIndexTocheck < 0)return;

    int cnt = 0; // calculation of dependency cost

    int size_ = 0;
    for(auto ins : QueueInstruction_new)
    {
        if(size_ > maxIndexTocheck)break;
        // cout << size_ << endl;
        if(ins.op == "add" || ins.op == "sub" || ins.op == "mul" || ins.op == "slt")
        {
            if(Current_ins.source1 == ins.target || Current_ins.source2 == ins.target)cnt += ins.cost;
            else if(Current_ins.target == ins.target || Current_ins.target == ins.source1 || Current_ins.target == ins.source2) cnt += ins.cost;
        }
        else if( ins.op == "beq" || ins.op == "bne")
        {
            if(Current_ins.target == ins.source1 || Current_ins.target == ins.source2)cnt += ins.cost;
        }
        else if(ins.op == "lw")
        {
            if(Current_ins.target == ins.target || Current_ins.target == ins.source1 || Current_ins.target == ins.source2) cnt += ins.cost;
        }
        else if(ins.op == "sw")
        {
            if(Current_ins.target == ins.target)cnt += ins.cost;   
        }
        else if(ins.op == "addi")
        {
            if(Current_ins.source1 == ins.target || Current_ins.source2 == ins.target) cnt += ins.cost;
            else if(Current_ins.target == ins.target || Current_ins.target == ins.source1 || Current_ins.target == ins.source2) cnt += ins.cost;            
        }
        size_++;
    }

    Current_ins.cost = cnt;
    
    // cout << cnt << endl;
}

void Registers::CheckDependency_beq(instruction &Current_ins, int maxIndexTocheck)
{
    if(QueueInstruction_new.size() == 0 || maxIndexTocheck < 0)return;

    int cnt = 0; // calculation of dependency cost

    int size_ = 0;
    for(auto ins : QueueInstruction_new)
    {
        if(size_ > maxIndexTocheck)break;
        if(ins.op == "add" || ins.op == "sub" || ins.op == "mul" || ins.op == "slt")
        {
            if(Current_ins.source1 == ins.target || Current_ins.source2 == ins.target)cnt += ins.cost;
        }
        else if(ins.op == "lw")
        {
            if(Current_ins.target == ins.source1 || Current_ins.target == ins.source2) cnt += ins.cost;
        }
        else if(ins.op == "addi")
        {
            if(Current_ins.source1 == ins.target || Current_ins.source2 == ins.target) cnt += ins.cost;
        }
    }
    Current_ins.cost = cnt;
}

void Registers::CheckDependency_lw(instruction &Current_ins, int maxIndexTocheck)
{
    if(QueueInstruction_new.size() == 0 || maxIndexTocheck < 0)return;

    int cnt = 0; // calculation of dependency cost

    int size_ = 0;
    for(auto ins : QueueInstruction_new)
    {
        if(size_ > maxIndexTocheck)break;
        if(ins.op == "add" || ins.op == "sub" || ins.op == "mul" || ins.op == "slt")
        {
            if(Current_ins.target == ins.target || Current_ins.target == ins.source1 || Current_ins.target == ins.source2) cnt += ins.cost;
        }
        else if( ins.op == "beq" || ins.op == "bne")
        {
            if(Current_ins.target == ins.source1 || Current_ins.target == ins.source2)cnt += ins.cost;
        }
        else if(ins.op == "lw")
        {
            if(Current_ins.target == ins.target || Current_ins.target == ins.source1 || Current_ins.target == ins.source2) cnt += ins.cost;
        }
        else if(ins.op == "sw")
        {
            if(Current_ins.target == ins.target)cnt += ins.cost;   
        }
        else if(ins.op == "addi")
        {
            if(Current_ins.target == ins.target || Current_ins.target == ins.source1 || Current_ins.target == ins.source2) cnt += ins.cost;            
        }
    }
    Current_ins.cost = cnt;
}

bool Registers::CheckOverallDependency(instruction &ins, int index)
{
    ins.cost = 0;

    if(ins.op == "add" || ins.op == "sub" || ins.op == "mul" || ins.op == "slt" || ins.op == "addi")
    {CheckDependency_add(ins, index);ins.cost+=1;return true;}
    
    if(ins.op == "beq" || ins.op == "bne")
    {CheckDependency_beq(ins, index);ins.cost+=1; return true;}
    
    if(ins.op == "lw" || ins.op == "sw")
    {CheckDependency_lw(ins, index);ins.cost += (RowDelay + ColumnDelay + 1); return true;}

    // cout << "Cost:" << ins.cost << "\n";
    return false;
}

instruction Registers::parse_new()
{
    // cout << "H1  " << instr.size() << "  " << i << endl;
    instruction instemp;

    if(i >= instr.size())return instemp;

    // cout << "H2  " << instr.size() << "  " << i << endl;
    // return instemp;

    instruction ins = instr[i];
    instr[i].InstructionCount++;
    print_count++;
    ErrorLine = i;

    // return instemp;
    if(ins.op == "j")ins.cost+=1;
    else if( !CheckOverallDependency(ins, QueueInstruction_new.size()-1) )
    {
        i++;
        print_count--;
        return parse_new();
    }

    i++;

    QueueInstruction_new.pb(ins);

    return ins;

}

void Registers::Execute_1(int j, instruction ins)
{
    int ind = max(EndTime,1);
    while(ClockCount[ind] == 1)EndTime++;
    ClockCount[ind] = 1;
    EndTime = ind+1;

    // cout << EndTime << endl;

    PrintCommand pc;
    pc.Start = pc.End = ind;
    pc.Command = ins.original;
    pc.Execution = InstructionExecution(ins);
    pc.File = BaseFilename + to_string(j);
    output.pb(pc);


    vector<instruction>::iterator it;
    int k=0;
    for(it = QueueInstruction_new.begin(); it!=QueueInstruction_new.end(); it++)
    {
        if(EqualityInstruction(*it, ins)){QueueInstruction_new.erase(it);break;}        
        k++;
    }
    k--;
    while(it!=QueueInstruction_new.end())
    {
        CheckOverallDependency(*it,k);
        it++;k++;
    }

}

void Registers::DramRequestIssued(instruction &ins)
{
    PrintCommand pc;
    
    int ind = max(EndTime,1);
    while(ClockCount[ind] == 1)EndTime++;
    ClockCount[ind] = 1;
    EndTime = ind+1;

    pc.Start = pc.End = ind;
    pc.Command = ins.original;
    pc.Execution = "DRAM Request Issued";
    pc.File = BaseFilename + to_string(ins.FileIndex);
    
    ins.InstructionRead = ind;
    ins.issued = true;

    int j    = ins.FileIndex;    
    int x = stoi(ins.offset) +  CPU_List[j-1].reg[ins.source1];
    ins.row = int(x / ColumnMemory);

    output.pb(pc);
}

void Registers::DeleteInstruction(instruction &ins)
{
    vector<instruction>::iterator it;
    int k=0;
    for(it = QueueInstruction_new.begin(); it!=QueueInstruction_new.end(); it++)
    {
        if(EqualityInstruction(*it, ins)){QueueInstruction_new.erase(it);break;}        
        k++;
    }
    k--;
    while(it!=QueueInstruction_new.end())
    {
        CheckOverallDependency(*it,k);
        it++;k++;
    }

}

void Execute_lw_sw(instruction ins)
{
    // cout << "Hey";
    // return;
    

    PrintCommand pc;
    pc.Start = max( ins.InstructionRead + 1, UniversalEndTime + 1);
    int j    = ins.FileIndex;    
    int x = stoi(ins.offset) +  CPU_List[j-1].reg[ins.source1];
    
    int rownumber    = ins.row;    
    int columnnumber = (x)%ColumnMemory;

    

    // return;

    if(ins.row != RowBuffer && RowBuffer != -1)
    {
        pc.End = pc.Start + RowDelay - 1;
        pc.Command = "DRAM: Writeback row " + to_string(RowBuffer) + "     (Ins: " + ins.original + ")";
        output.pb(pc);
    }
    else pc.End = pc.Start-1; // So that next if-else block can be implemented easily

    if(ins.row != RowBuffer)
    {
        pc.Start = pc.End+1;
        pc.End = pc.Start + RowDelay - 1;
        pc.Command = "DRAM: Activate row " + to_string(ins.row) + "      (Ins: " + ins.original + ")";
        pc.File = BaseFilename + to_string(ins.FileIndex);
        output.pb(pc);
    }
    else pc.End = pc.Start-1;

    pc.Start = pc.End+1;
    
    pc.End = pc.Start + ColumnDelay - 1;
    pc.Command = "DRAM: Column Access " + to_string( columnnumber ) + "  (Ins: " + ins.original + ")";
    pc.File = BaseFilename + to_string(ins.FileIndex);

    if(ins.op == "lw")
    {
        CPU_List[j-1].reg[ins.target] = Dram[rownumber][columnnumber];
        pc.Execution = ins.target + " = " + to_string(Dram[rownumber][columnnumber]);
    }
    else 
    {
        Dram[rownumber][columnnumber] = CPU_List[j-1].reg[ins.target];
        pc.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(CPU_List[j-1].reg[ins.target]);
    }

    output.pb(pc);
    

    UniversalEndTime = pc.End;
    RowBuffer = ins.row;

    CPU_List[j-1].DeleteInstruction(ins); // Here j is FileIndex i.e. Actual_j + 1
    CPU_List[j-1].ClockCount[pc.End] = 1;

}

void MemoryManager_Implementation()
{
    if(MemoryManager.size() != 0)
    {
        if(UniversalEndTime == -1)
        {
            instruction ins = MemoryManager[rand() % MemoryManager.size()];
            Execute_lw_sw(ins);
            return;
        }
        else{
            bool wait = false;
            
            for(int j=0; j < CPU; j++)
            {
                if(CPU_List[j].i < CPU_List[j].instr.size() && ( CPU_List[j].EndTime - 1 ) < UniversalEndTime )wait = true;
            }
            // if(!wait)cout << "Hey\n";
            if(!wait)
            {
                // Ready to execute an instruction in Memory Manager.
                bool SameRow = false;
                for(int j=0; j < MemoryManager.size(); j++)
                {
                    if(MemoryManager[j].row == RowBuffer)
                    {
                        Execute_lw_sw(MemoryManager[j]);
                        SameRow = true;
                        break;
                    }
                }
                if(!SameRow)
                {
                    instruction ins = MemoryManager[rand() % MemoryManager.size()];
                    Execute_lw_sw(ins);
                }
            }
        }
    }
}

int main(int argc, char * argv[]){

    int program = input(argc, argv);
    if(program == -1)return -1;

    // This loop ensures that all files are tokenized and parsed correctly
    for(int i=0; i<CPU; i++){

        string filename = BaseFilename + to_string(i+1);
        bool result = openFile(filename);
        if(!result)return -1;
        CPU_List[i].parse();

        if(flag){
            cout << CPU_List[i].print_msg;
            cout << "FILE NAME: " << BaseFilename << i+1 << "\n";
            cout << "LINE: " << CPU_List[i].instr[CPU_List[i].ErrorLine].original << endl;
            return -1;
        }
    }

    // int stop = 11;

    while(true)
    {
        bool check = false;        
        MemoryManager.clear();
        
        for(int j = 0; j < CPU; j++)
        {        
            instruction ins = CPU_List[j].parse_new();
            if(ins.cost == 0)continue;
            check = true;

            for(int k = 0; k < CPU_List[j].QueueInstruction_new.size(); k++)
            {
                if(CPU_List[j].QueueInstruction_new[k].cost == 1)CPU_List[j].Execute_1(j+1, CPU_List[j].QueueInstruction_new[k]);
                
                if( (CPU_List[j].QueueInstruction_new[k].op == "lw" || CPU_List[j].QueueInstruction_new[k].op == "sw") && 
                    !CPU_List[j].QueueInstruction_new[k].issued)
                {
                    // cout << CPU_List[j].QueueInstruction_new[k].original << " " << CPU_List[j].QueueInstruction_new[k].InstructionRead << endl;
                    CPU_List[j].QueueInstruction_new[k].FileIndex = j+1;
                    CPU_List[j].DramRequestIssued(CPU_List[j].QueueInstruction_new[k]);
                    // cout << CPU_List[j].QueueInstruction_new[k].original << " " << CPU_List[j].QueueInstruction_new[k].InstructionRead << endl;
                }
            }
            int cst = INT_MAX;
            instruction temp;            
            for(int k = 0; k < CPU_List[j].QueueInstruction_new.size(); k++)
            {
                if(CPU_List[j].QueueInstruction_new[k].cost < cst)
                {
                    temp = CPU_List[j].QueueInstruction_new[k];
                    cst = CPU_List[j].QueueInstruction_new[k].cost;
                }
                CPU_List[j].QueueInstruction_new[k].FileIndex = j+1;
            }
            
            if(cst != INT_MAX)MemoryManager.pb(temp);

        }

        // cout << MemoryManager.size() << endl;

        MemoryManager_Implementation();

        // stop--;
        // if(stop==0)break;
        if(!check)break;
    }
    while(true)
    {

        MemoryManager.clear();
        
        for(int j=0;j<CPU;j++)
        {   
            // DEBUGGING
            // VectorOutput(CPU_List[j].QueueInstruction_new, "Queue", j+1);

            if(CPU_List[j].QueueInstruction_new.size() == 0)continue;

            // cout << j << endl;
            
            for(int k = 0; k < CPU_List[j].QueueInstruction_new.size(); k++)            
            {
                if(CPU_List[j].QueueInstruction_new[k].cost == 1)CPU_List[j].Execute_1(j+1, CPU_List[j].QueueInstruction_new[k]);                                
            }

            int cst = INT_MAX;
            instruction temp;            

            for(int k = 0; k < CPU_List[j].QueueInstruction_new.size(); k++)
            {
                if(CPU_List[j].QueueInstruction_new[k].cost < cst)
                {
                    temp = CPU_List[j].QueueInstruction_new[k];
                    cst = CPU_List[j].QueueInstruction_new[k].cost;
                }
                CPU_List[j].QueueInstruction_new[k].FileIndex = j+1;
            }

            for(int k = 0; k < CPU_List[j].QueueInstruction_new.size(); k++)
            {
                if(CPU_List[j].QueueInstruction_new[k].cost == cst)
                {
                    MemoryManager.pb(CPU_List[j].QueueInstruction_new[k]);
                }
            }
            
            // if(cst != INT_MAX)MemoryManager.pb(temp);
            
        }
        if(MemoryManager.size() == 0) break;

        // DEBUGGING
        // VectorOutput(MemoryManager, "MemoryManager", -1);

        MemoryManager_Implementation();

    }
    FinalPrint(output);

    return 0;
}
