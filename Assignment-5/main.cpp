#include "main.hpp"
#include "HelperFunctions.hpp"


vector<PrintCommand> output;
vector<instruction> MemoryManager;

int InstructionCount = 0; // Throughput

void Registers::CheckDependency_add(instruction &Current_ins, int maxIndexTocheck, bool Update)
{
    if(QueueInstruction_new.size() == 0 || maxIndexTocheck < 0)return;

    int cnt = 0; // calculation of dependency cost

    int size_ = 0;
    for(auto ins : QueueInstruction_new)
    {
        int temp = cnt;

        if(size_ > maxIndexTocheck)break;
        // cout << size_ << endl;
        if(ins.op == "add" || ins.op == "sub" || ins.op == "mul" || ins.op == "slt" || ins.op == "addi")
        {
            if(Current_ins.source1 == ins.target || Current_ins.source2 == ins.target) cnt += ins.cost;
            else if(Current_ins.target == ins.target || Current_ins.target == ins.source1 || Current_ins.target == ins.source2) cnt += ins.cost;
        }
        else if( ins.op == "beq" || ins.op == "bne")
        {
            if(Current_ins.target == ins.source1 || Current_ins.target == ins.source2)cnt += ins.cost;
        }
        else if(ins.op == "lw")
        {
            if(Current_ins.target == ins.target || Current_ins.source1 == ins.target || Current_ins.source2 == ins.target) cnt += ins.cost;
        }
        else if(ins.op == "sw")
        {
            if(Current_ins.target == ins.target)cnt += ins.cost;
        }

        if(temp != cnt && !Update)
        {
            // This implies the passed instruction depends upon the current instruction in loop
            Current_ins.Endtime = max(Current_ins.Endtime, ins.Endtime + 1);
        }

        size_++;
    }

    if(Update)Current_ins.cost = cnt;
    
    // cout << cnt << endl;
}

void Registers::CheckDependency_beq(instruction &Current_ins, int maxIndexTocheck, bool Update)
{
    if(QueueInstruction_new.size() == 0 || maxIndexTocheck < 0)return;

    int cnt = 0; // calculation of dependency cost

    int size_ = 0;
    for(auto ins : QueueInstruction_new)
    {
        int temp = cnt;
        
        if(size_ > maxIndexTocheck)break;        
        if(ins.op == "add" || ins.op == "sub" || ins.op == "mul" || ins.op == "slt" || ins.op == "lw" || ins.op == "addi")
        {
            if(Current_ins.source1 == ins.target || Current_ins.source2 == ins.target)cnt += ins.cost;
        }

        if(temp != cnt && !Update)
        {
            // This implies the passed instruction depends upon the current instruction in loop
            Current_ins.Endtime = max(Current_ins.Endtime, ins.Endtime + 1);
        }

        size_++;
    }
    if(Update)Current_ins.cost = cnt;
}

void Registers::CheckDependency_lw(instruction &Current_ins, int maxIndexTocheck, bool Update)
{
    if(QueueInstruction_new.size() == 0 || maxIndexTocheck < 0)return;

    int cnt = 0; // calculation of dependency cost

    int size_ = 0;
    for(auto ins : QueueInstruction_new)
    {
        int temp = cnt; // To check if any instruction is updated or not

        if(size_ > maxIndexTocheck)break;
        if(ins.op == "add" || ins.op == "sub" || ins.op == "mul" || ins.op == "slt")
        {
            if(Current_ins.target == ins.target || Current_ins.source1 == ins.target 
                || Current_ins.target == ins.source1 || Current_ins.target == ins.source2) cnt += ins.cost;
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

        if(temp != cnt && !Update)
        {
            // This implies the passed instruction depends upon the current instruction in loop
            Current_ins.Endtime = max(Current_ins.Endtime, ins.Endtime + 1);
        }

        size_++;
    }
    if(Update)Current_ins.cost = cnt;
}

bool Registers::CheckOverallDependency(instruction &ins, int index, bool Update = true)
{
    if(Update)ins.cost = 0;

    if(ins.op == "add" || ins.op == "sub" || ins.op == "mul" || ins.op == "slt" || ins.op == "addi")
    {CheckDependency_add(ins, index, Update);if(Update)ins.cost+=1;return true;}
    
    if(ins.op == "beq" || ins.op == "bne")
    {CheckDependency_beq(ins, index, Update);if(Update)ins.cost+=1; return true;}
    
    if(ins.op == "lw" || ins.op == "sw")
    {CheckDependency_lw(ins, index, Update); if(Update)ins.cost += (RowDelay + ColumnDelay + 1); return true;}

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
    if(ins.op == "j"){
        ins.cost+=1;
    }
    else if( !CheckOverallDependency(ins, QueueInstruction_new.size()-1) )
    {
        i++;
        print_count--;
        return parse_new();
    }

    if(  ins.op == "j" || 
        (ins.op == "beq" && reg[ins.source1] == reg[ins.source2]) ||
        (ins.op == "bne" && reg[ins.source1] != reg[ins.source2])
    ) i = label[ins.jump];

    else i++;

    ins.InstructionRead = UniversalInstructionRead;
    QueueInstruction_new.pb(ins);

    return ins;

}

void Registers::Execute_1(int j, instruction ins)
{
    int ind = max(EndTime, max(1, ins.Endtime) );
    while(ClockCount[ind] == 1)ind++;
    ClockCount[ind] = 1;
    EndTime = ind;

    // cout << EndTime << endl;

    if(ins.op == "add")reg[ins.target] = reg[ins.source1] + reg[ins.source2];
    if(ins.op == "sub")reg[ins.target] = reg[ins.source1] - reg[ins.source2];
    if(ins.op == "mul")reg[ins.target] = reg[ins.source1] * reg[ins.source2];
    if(ins.op == "slt"){
        if( reg[ins.source1] < reg[ins.source2] ) reg[ins.target] = 1;
        else reg[ins.target] = 0;
    }



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
    
    int ind = max(EndTime,max(1,UniversalInstructionRead));
    while(ClockCount[ind] == 1)ind++;
    ClockCount[ind] = 1;
    EndTime = ind+1;

    pc.Start = pc.End = ind;
    pc.Command = ins.original;
    pc.Execution = "DRAM Request Issued";
    pc.File = BaseFilename + to_string(ins.FileIndex);
    
    // ins.InstructionRead = ind;
    ins.issued = true;
    ins.Endtime = pc.End;

    int j    = ins.FileIndex;    
    int x = stoi(ins.offset) +  CPU_List[j-1].reg[ins.source1];
    ins.row = int(x / ColumnMemory);

    output.pb(pc);

    //---------------------------------
    vector<instruction>::iterator it;
    for(it = QueueInstruction_new.begin(); it!=QueueInstruction_new.end(); it++)
    {
        if(EqualityInstruction(*it, ins)){                        
            it->issued = true;
            it->Endtime = ins.Endtime;
            it->row = ins.row;
            break;
        }                
    }
    //---------------------------------

}

void Registers::DeleteInstruction(instruction &ins)
{
    
    vector<instruction>::iterator it, it_temp;
    int k=0, k_temp = 0;
    it_temp = QueueInstruction_new.begin();
    // print_ins(ins);
    
    for(it = QueueInstruction_new.begin(); it!=QueueInstruction_new.end(); it++)
    {
        if(EqualityInstruction(*it, ins)){

            it_temp->Endtime = ins.Endtime;
            while( it_temp != QueueInstruction_new.end() )
            {
                
                // print_ins(*it_temp);

                CheckOverallDependency(*it_temp, k_temp-1, false);

                // print_ins(*it_temp);
                it_temp++;
                k_temp++;
            }
            
            // cout << k << endl;

            QueueInstruction_new.erase(it);
            break;
        }
        
        k++;
        it_temp++;
        k_temp++;
    }
    k--;
    while(it!=QueueInstruction_new.end())
    {
        CheckOverallDependency(*it,k);
        it++;k++;
    }

}

void Execute_lw_sw(instruction &ins)
{
    
    PrintCommand pc;
    pc.Start = max( ins.InstructionRead + 1, UniversalEndTime + 1);
    int j    = ins.FileIndex;    

    // int x;
    
    // try
    // {
    //     x = stoi(ins.offset) +  CPU_List[j-1].reg[ins.source1];
    //     if(x<0)throw 1;
    // }
    // catch (int x)
    // {
    //     cout << "Exception\n";
    // }

    int x = stoi(ins.offset) +  CPU_List[j-1].reg[ins.source1];
    
    int rownumber    = ins.row;    
    int columnnumber = (x)%ColumnMemory;

    if(ins.row != RowBuffer && RowBuffer != -1)
    {
        pc.End = pc.Start + RowDelay - 1;
        pc.Command = "DRAM: Writeback row " + to_string(RowBuffer) + "     ( Ins: " + ins.original + " )";
        output.pb(pc);
    }
    else pc.End = pc.Start-1; // So that next if-else block can be implemented easily

    if(ins.row != RowBuffer)
    {
        pc.Start = pc.End+1;
        pc.End = pc.Start + RowDelay - 1;
        pc.Command = "DRAM: Activate row " + to_string(ins.row) + "      ( Ins: " + ins.original + " )";
        pc.File = BaseFilename + to_string(ins.FileIndex);
        output.pb(pc);
    }
    else pc.End = pc.Start-1;

    pc.Start = pc.End+1;
    
    pc.End = pc.Start + ColumnDelay - 1;

    if(to_string( columnnumber ).size() == 4)
    pc.Command = "DRAM: Column Access " + to_string( columnnumber ) + "  ( Ins: " + ins.original + " )";
    if(to_string( columnnumber ).size() == 3)
    pc.Command = "DRAM: Column Access " + to_string( columnnumber ) + "   ( Ins: " + ins.original + " )";
    if(to_string( columnnumber ).size() == 2)
    pc.Command = "DRAM: Column Access " + to_string( columnnumber ) + "    ( Ins: " + ins.original + " )";
    if(to_string( columnnumber ).size() == 1)
    pc.Command = "DRAM: Column Access " + to_string( columnnumber ) + "     ( Ins: " + ins.original + " )";

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
    ins.Endtime = pc.End;

    // d1("++++++++++++");
    // Registers::print_ins(ins);
    // d1("++++++++++++");

    CPU_List[j-1].ClockCount[pc.End] = 1;
    CPU_List[j-1].DeleteInstruction(ins); // Here j is FileIndex i.e. Actual_j + 1
    

}

void MemoryManager_Implementation()
{

    vector<instruction> MemoryManager_NotIssued;

    for(int i=0;i<MemoryManager.size();i++)
    {
        if(!MemoryManager[i].issued)
        {
            MemoryManager_NotIssued.pb(MemoryManager[i]);
        }
    }

    int MinCost = INT_MAX;
    vector<instruction>Notissued;

    for(int i=0;i<MemoryManager_NotIssued.size();i++)
    {
        MinCost = min(MinCost, MemoryManager_NotIssued[i].cost);
    }

    for(int i=0;i<MemoryManager_NotIssued.size();i++)
    {
        if(MemoryManager_NotIssued[i].cost == MinCost)
        {
            Notissued.pb(MemoryManager_NotIssued[i]);
        }
    }

    MemoryManager_NotIssued = Notissued;

    if( MemoryManager_NotIssued.size() > 0)
    {
        if(UniversalEndTime == -1)
        {
            instruction ins = MemoryManager_NotIssued[rand() % MemoryManager_NotIssued.size()];
            CPU_List[ins.FileIndex-1].DramRequestIssued(ins);

        }
        else
        {
            bool issued = false;
            for(int i=0; i < MemoryManager_NotIssued.size(); i++)
            {
                instruction ins = MemoryManager_NotIssued[i];
                int x = stoi(ins.offset) +  CPU_List[ins.FileIndex-1].reg[ins.source1];

                MemoryManager_NotIssued[i].row = int(x / ColumnMemory);                
                if(RowBuffer == MemoryManager_NotIssued[i].row)
                {
                    CPU_List[ins.FileIndex-1].DramRequestIssued(MemoryManager_NotIssued[i]);
                    issued = true;
                    break;
                }
            }
            if(!issued)
            {
                instruction ins = MemoryManager_NotIssued[rand() % MemoryManager_NotIssued.size()];
                CPU_List[ins.FileIndex-1].DramRequestIssued(ins);
            }
        }
    }

    vector<instruction> MemoryManager_Issued;


    MinCost = INT_MAX;
    vector<instruction>Issued_Instr;

    for(int i = 0; i < MemoryManager.size(); i++)
    {
        if(MemoryManager[i].issued) MemoryManager_Issued.pb(MemoryManager[i]);
    }

    for(int i=0;i<MemoryManager_Issued.size();i++)
    {
        MinCost = min(MinCost, MemoryManager_Issued[i].cost);
    }

    for(int i=0;i<MemoryManager_Issued.size();i++)
    {
        if(MemoryManager_Issued[i].cost == MinCost)
        {
            Issued_Instr.pb(MemoryManager_Issued[i]);
        }
    }

    MemoryManager_Issued = Issued_Instr;

    if(MemoryManager_Issued.size() > 0)
    {
        if(UniversalEndTime == -1)
        {
            instruction ins = MemoryManager_Issued[rand() % MemoryManager_Issued.size()];
            Execute_lw_sw(ins);
            return;
        }
        else
        {
            bool wait = false;
            
            for(int j=0; j < CPU; j++)
            {
                if(CPU_List[j].EndTime < UniversalEndTime && CPU_List[j].i >= CPU_List[j].instr.size() ) CPU_List[j].EndTime = UniversalEndTime;  
                else if(CPU_List[j].i < CPU_List[j].instr.size() && CPU_List[j].EndTime  < UniversalEndTime ){
                    wait = true;
                }
            }

            if(!wait)
            {
                // Ready to execute an instruction in Memory Manager.
                bool SameRow = false;
                for(int j=0; j < MemoryManager_Issued.size(); j++)
                {
                    if(MemoryManager_Issued[j].row == RowBuffer)
                    {
                        Execute_lw_sw(MemoryManager_Issued[j]);
                        SameRow = true;
                        break;
                    }
                }
                if(!SameRow)
                {
                    instruction ins = MemoryManager_Issued[rand() % MemoryManager_Issued.size()];
                    Execute_lw_sw(ins);
                }
            }

        }
    }

    /*

    We have MemoryManager Vector
    This will contain all lw/sw instructions ( Not just minimum)

    Make a vector containing non-issued instructions and select one out of them and issue Dram request
    ( Implement Random selection ... Optimize it later )

    Iterate over MemoryManager and store issued instructions in some other vector
    Select one out of them using some algorithm and execute it (if possible)



    */
}

int main(int argc, char * argv[]){

    int program = input(argc, argv);
    if(program == -1)return -1;

    for(int i=0;i<UNIVERSAL_ISSUE_TIME;i++)UniversalIssueTime[i]=0;

    // This loop ensures that all files are tokenized and parsed correctly
    for(int i=0; i<CPU; i++){

        string filename = BaseFilename + to_string(i+1);
        bool result = openFile(filename, i+1);
        if(!result)return -1;
        CPU_List[i].parse();

        if(flag){
            cout << CPU_List[i].print_msg;
            cout << "FILE NAME: " << BaseFilename << i+1 << "\n";
            cout << "LINE: " << CPU_List[i].instr[CPU_List[i].ErrorLine].original << endl;
            return -1;
        }
    }

    // d1("Hello");
    // return 0;

    // int stop = 11;

    while(true)
    {
        UniversalInstructionRead++;
        bool check = false;        
        MemoryManager.clear();
        
        // d1(UniversalInstructionRead);
        // d2(CPU_List[0].EndTime, CPU_List[1].EndTime);

        vector<instruction> DramRequestIssued_Vector;

        for(int j = 0; j < CPU; j++)
        {                    
            // cout << "Hey0\n";
            instruction ins = CPU_List[j].parse_new();
            
            if(ins.cost == 0){
                // CPU_List[j].EndTime++;
                continue; // Means file has been read completely
            }
            check = true;            

            for(int k = 0; k < CPU_List[j].QueueInstruction_new.size(); k++)
            {
                if(k >= CPU_List[j].QueueInstruction_new.size())break;

                CPU_List[j].QueueInstruction_new[k].FileIndex = j+1;
                if(CPU_List[j].QueueInstruction_new[k].cost == 1)
                {
                    CPU_List[j].Execute_1(j+1, CPU_List[j].QueueInstruction_new[k]);
                    k--;
                    k = max(0,k);
                }                
                // else if( (CPU_List[j].QueueInstruction_new[k].op == "lw" || CPU_List[j].QueueInstruction_new[k].op == "sw") && 
                //     !CPU_List[j].QueueInstruction_new[k].issued)
                // {
                //     if(CPU_List[j].ClockCount[UniversalInstructionRead] == 0)
                //     {
                //         DramRequestIssued_Vector.pb(CPU_List[j].QueueInstruction_new[k]);
                //     }
                //     else
                //     {
                //         CPU_List[j].i--;
                //         CPU_List[j].print_count--;
                //         CPU_List[j].QueueInstruction_new.erase(CPU_List[j].QueueInstruction_new.end()-1);
                //         CPU_List[j].EndTime++;
                //     }
                //     // CPU_List[j].DramRequestIssued(CPU_List[j].QueueInstruction_new[k]);                    
                // }
            }            

            int k_temp = 0;
            while( k_temp < CPU_List[j].QueueInstruction_new.size() )
            {
                if(CPU_List[j].QueueInstruction_new[k_temp].cost == 1){
                    // d1("ERROR");
                    CPU_List[j].Execute_1(j+1, CPU_List[j].QueueInstruction_new[k_temp]);
                }
                else{
                    k_temp++;
                }
            }

            for(int k = 0; k < CPU_List[j].QueueInstruction_new.size(); k++)
            {
                if(CPU_List[j].QueueInstruction_new[k].op == "sw" || CPU_List[j].QueueInstruction_new[k].op == "lw")
                {
                    MemoryManager.pb(CPU_List[j].QueueInstruction_new[k]);
                }
            }

            int cst = INT_MAX;
            instruction temp;            

            // for(int k = 0; k < CPU_List[j].QueueInstruction_new.size(); k++)
            // {
            //     if(CPU_List[j].QueueInstruction_new[k].cost < cst)
            //     {
            //         temp = CPU_List[j].QueueInstruction_new[k];
            //         cst = CPU_List[j].QueueInstruction_new[k].cost;
            //     }
            //     CPU_List[j].QueueInstruction_new[k].FileIndex = j+1;
            // }

            // for(int k = 0; k < CPU_List[j].QueueInstruction_new.size(); k++)
            // {
            //     try{
            //         if(CPU_List[j].QueueInstruction_new[k].cost == cst && (CPU_List[j].QueueInstruction_new[k].op[1] == 'w'))
            //         {
            //             MemoryManager.pb(CPU_List[j].QueueInstruction_new[k]);
            //         }
            //     }
            //     catch (...)
            //     {
            //         cout << CPU_List[j].QueueInstruction_new[k].op << endl;
            //         return -1;
            //     }
                
            // }

            // VectorOutput(CPU_List[j].QueueInstruction_new, "Queue", j+1);
            // cout << "Hey0\n";
            
            // if(cst != INT_MAX)MemoryManager.pb(temp);

        }

        // vector<instruction> DramRequestIssued_Vector_Available;

        // d1(DramRequestIssued_Vector.size());
        // d1(rand() % DramRequestIssued_Vector.size());

        bool MemoryManagerActive = true;
        
        if(DramRequestIssued_Vector.size() > 0)
        {
            auto lw_sw_instr = DramRequestIssued_Vector[ rand() % DramRequestIssued_Vector.size() ];
            int jFileIndex = lw_sw_instr.FileIndex-1;
            CPU_List[jFileIndex].DramRequestIssued( lw_sw_instr );

            for(int z = 0; z < DramRequestIssued_Vector.size(); z++)
            {
                if(!DramRequestIssued_Vector[z].issued)
                {
                    CPU_List[jFileIndex].i--;
                    CPU_List[jFileIndex].print_count--;
                    CPU_List[jFileIndex].QueueInstruction_new.erase(CPU_List[jFileIndex].QueueInstruction_new.end()-1);
                    CPU_List[jFileIndex].EndTime++;
                }
            }

            MemoryManagerActive = false;
        }
        // cout << MemoryManager.size() << endl;

        // cout << "Hey1\n";
        // return 0;

        // VectorOutput(MemoryManager, "MemoryManager", -1);        

        if(MemoryManagerActive)
        {
            MemoryManager_Implementation();
        }

        // cout << "Hey2\n";

        // stop--;
        // if(stop==0)break;
        if(!check)break;
    }

    // d1(CPU_List[0].EndTime);
    // VectorOutput(CPU_List[0].QueueInstruction_new, "Queue", 1);
    // VectorOutput(MemoryManager, "MemoryManager", -1);

    // d1("Hello");
    // return 0;

    // cout << "H";
    while(true)
    {
        // cout << "Hey\n";

        MemoryManager.clear();
        
        for(int j=0;j<CPU;j++)
        {   
            // DEBUGGING
            // VectorOutput(CPU_List[j].QueueInstruction_new, "Queue", j+1);

            if(CPU_List[j].QueueInstruction_new.size() == 0)continue;

            // cout << j << endl;

            // VectorOutput(CPU_List[j].QueueInstruction_new, "Queue", j+1);
            
            int k_temp = 0;

            while( k_temp < CPU_List[j].QueueInstruction_new.size() )
            {
                if(CPU_List[j].QueueInstruction_new[k_temp].cost == 1){
                    CPU_List[j].Execute_1(j+1, CPU_List[j].QueueInstruction_new[k_temp]);
                }
                else{
                    k_temp++;
                }
            }

            // for(int k = 0; k < CPU_List[j].QueueInstruction_new.size(); ++k)
            // {
            //     if(CPU_List[j].QueueInstruction_new[k].cost == 1){
            //         CPU_List[j].Execute_1(j+1, CPU_List[j].QueueInstruction_new[k]);
            //         k--;
            //         k = max(0,k);
            //     }
            // }

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
                try{
                    if(CPU_List[j].QueueInstruction_new[k].cost == cst && (CPU_List[j].QueueInstruction_new[k].op[1] == 'w'))
                    {
                        MemoryManager.pb(CPU_List[j].QueueInstruction_new[k]);
                    }
                }
                catch (...)
                {
                    cout << CPU_List[j].QueueInstruction_new[k].op << endl;
                    return -1;
                }
            }
            
            // if(cst != INT_MAX)MemoryManager.pb(temp);

            // VectorOutput(CPU_List[j].QueueInstruction_new, "Queue", j+1);
            
        }
        if(MemoryManager.size() == 0) break;

        // DEBUGGING
        // VectorOutput(MemoryManager, "MemoryManager", -1);
        // d1("Hello");

        // return 0;

        MemoryManager_Implementation();

    }

    // d1("Hello");
    // return 0;

    FinalPrint(output);

    return 0;
}
