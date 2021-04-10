#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<iomanip>
#include<map>
#include<queue>
#include<sstream>
#include<boost/algorithm/string.hpp>
#include<boost/format.hpp> 

#define ff first
#define pb push_back
#define ss second
#define mp make_pair
#define pii pair<int,int>
#define printp(x) cout << "(" <<  x.ff << "," << x.ss << ") "

using namespace std;
map<string,int> reg, label;
map<string,pair<int,int> > RegisterUpdation, RegisterUse;
map<int,pair<int,int> > MemoryUpdation, MemoryUse;
map<string,int> InstructionCountVector;

string print_msg = "";
vector<vector<int> > Dram; // Dynamic Random Access Memory

int RowDelay           = 10;
int ColumnDelay        = 2;
int TotalMemory_4Bytes = 1048576;
int RowMemory          = 1024;
int ColumnMemory       = 1024;
int RowBuffer          = -1;                   // Assumption - We have only 1 Row Buffer (otherwise, we could have used an array)
                                               // Also defined a vector named RowBufferCopy to store the orginal copy of row copied to row buffer

int TotalMemory        = TotalMemory_4Bytes/4;
int print_count        = 0;
int ErrorLine          = -1;
int NumberOfCycles     = 0;
int StartTime          = -1;
int EndTime            = -1;
int RowBufferUpdates   = 0;
int MemoryAvailable;

bool Part2 = false;
bool flag  = false;
bool DesignDecision = false;
vector<int> RowBufferCopy;
map<string,string> MemoryContent;


struct PrintCommand{
    int Start = -1;
    int End = -1;
    string Command = "";
    string Execution = "";
};

struct instruction{
    string op="",target="",source1="",source2="";
    string jump = "";
    string offset = "";
    string fun_label="";
    string original = "";
    int InstructionCount = 0;
    int row = -1;
    vector<pair<int,int> > dependent; // Can be of length at most 4.
    // Different index will contain different instructions 
    // Kind of instructions will also vary with lw, sw
};

vector<instruction> instr;
vector<PrintCommand> Command;
vector<instruction> QueueInstruction; // Also need to check whether a particular address is in use and whether it is accessed again


// First job is to parse over the file and store the instructions in instruction memory 
string strip(string s){
    // cout << "->" << s << "<-\n";
    int start=0;
    while(start<s.size() && s[start] == ' ' || s[start] == '\t')start++; // removing whitespace from tokens
    if(start == s.size()){flag=true;print_msg = "ERROR : Syntax Error\n";return "";}
    int end = s.size()-1;
    while(end >= 0 && s[end] == ' ' || s[end] == '\t')end--;
    string s_new = s.substr(start,end-start+1);
    return s_new;
}

bool Comparator(PrintCommand a, PrintCommand b){
    if(a.End!=b.End)return a.End < b.End;
    return a.Start < b.Start;
}

bool is_number(const string& s)
{
    string::const_iterator it = s.begin();
    if(*it=='-')it++;
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void std_registers(){

    // Standard QTSPIM registers
    reg["$zero"]   =0; reg["$at"]=0; reg["$v0"]=0; reg["$v1"]=0; 
    reg  ["$a0"]   =0; reg["$a1"]=0; reg["$a2"]=0; reg["$a3"]=0; 
    reg  ["$t0"]   =0; reg["$t1"]=0; reg["$t2"]=0; reg["$t3"]=0; reg["$t4"]=0; reg["$t5"]=0; reg["$t6"]=0; reg["$t7"]=0;
    reg  ["$s0"]   =0; reg["$s1"]=0; reg["$s2"]=0; reg["$s3"]=0; reg["$s4"]=0; reg["$s5"]=0; reg["$s6"]=0; reg["$s7"]=0;
    reg  ["$t8"]   =0; reg["$t9"]=0; reg["$k0"]=0; reg["$k1"]=0; reg["$gp"]=0; reg["$sp"]=0; reg["$fp"]=0; reg["$ra"]=0;

    // reg["$t0"]=0;reg["$t1"]=0;reg["$t2"]=0;reg["$t3"]=0;reg["$t4"]=0;reg["$t5"]=0;reg["$t6"]=0;reg["$t7"]=0;reg["$t8"]=0;reg["$t9"]=0;
    // reg["$s0"]=0;reg["$s1"]=0;reg["$s2"]=0;reg["$s3"]=0;reg["$s4"]=0;reg["$s5"]=0;reg["$s6"]=0;reg["$s7"]=0;reg["$s8"]=0;reg["$s9"]=0;
    // reg["$r0"]=0;reg["$r1"]=0;reg["$r2"]=0;reg["$r3"]=0;reg["$r4"]=0;reg["$r5"]=0;reg["$r6"]=0;reg["$r7"]=0;reg["$r8"]=0;reg["$r9"]=0;
    // reg["$v0"]=0;reg["$zero"]=0;

    // reg["$r0"]=0;reg["$r1"]=0;reg["$r2"]=0;reg["$r3"]=0;reg["$r4"]=0;reg["$r5"]=0;reg["$r6"]=0;reg["$r7"]=0;reg["$r8"]=0;reg["$r9"]=0;
    // reg["$r10"]=0;reg["$r11"]=0;reg["$r12"]=0;reg["$r13"]=0;reg["$r14"]=0;reg["$r15"]=0;reg["$r16"]=0;reg["$r17"]=0;reg["$r18"]=0;reg["$r19"]=0;
    // reg["$r20"]=0;reg["$r21"]=0;reg["$r22"]=0;reg["$r23"]=0;reg["$r24"]=0;reg["$r25"]=0;reg["$r26"]=0;reg["$r27"]=0;reg["$r28"]=0;reg["$r29"]=0;
    // reg["$r30"]=0;reg["$zero"]=0;

    for(auto i = reg.begin(); i != reg.end(); i++){RegisterUpdation[i->ff] = mp(-1,-1); RegisterUse[i->ff] = mp(-1,-1);}

    // Initializing the InstructionCountVector
    InstructionCountVector["add"] = 0;
    InstructionCountVector["addi"] = 0;
    InstructionCountVector["sub"] = 0;
    InstructionCountVector["mul"] = 0;
    InstructionCountVector["beq"] = 0;
    InstructionCountVector["bne"] = 0;
    InstructionCountVector["slt"] = 0;
    InstructionCountVector["j"] = 0;
    InstructionCountVector["lw"] = 0;
    InstructionCountVector["sw"] = 0;

}

void print_reg(){
    int cnt = 0;
    for (auto i = reg.begin(); i != reg.end(); i++){
        stringstream ss;
        ss << hex << i->second;
        string res(ss.str());
        cout << i->ff << ":" << res << ",   ";
        // cout << res << " ";
        cnt++;
        if(cnt%10==0 || cnt==32)cout<<"\n";
        // else cout<<", ";
    }
    cout << NumberOfCycles;
    cout <<"\n";
}

void print_ins(instruction ins){
    cout << "Op : ->" << ins.op << "<-\n";
    cout << "Tar : ->" << ins.target << "<-\n";
    cout << "Sou1 : ->" << ins.source1 << "<-\n";
    cout << "Sou2 : ->" << ins.source2 << "<-\n";
    cout << "Jump : ->" << ins.jump << "<-\n";
    cout << "Jump_index : ->" << label[ins.jump] << "<-\n";
    cout << "Off : ->" << ins.offset << "<-\n";
    cout << "Fun_lable : ->" << ins.fun_label << "<-\n";
    cout << "Dependency : "; printp(ins.dependent[0]) ; printp(ins.dependent[1]); printp(ins.dependent[2]) ; printp(ins.dependent[3]) ; cout << "\n";
    cout << ins.target << " (Updation): " ;printp(RegisterUpdation[ins.target]); cout << " (Use): ";printp(RegisterUse[ins.target]);cout << "\n";
    cout << ins.source1 << " (Updation): " ;printp(RegisterUpdation[ins.source1]); cout << " (Use): ";printp(RegisterUse[ins.source1]);cout << "\n";
}

void print_map(map<string,int> m){
    for(auto i=m.begin(); i!=m.end();i++){
        cout << "->" << i->ff << "<-->" << i->second <<"<-\n"; 
    }
}  

void print_command_debug(PrintCommand pc){
    cout << "Start : ->" << pc.Start << "<-\n";
    cout << "End : ->" << pc.End << "<-\n";
    cout << "Command : ->" << pc.Command << "<-\n\n";
}

void print_command(PrintCommand pc){
    string s = "Cycle ";
    if(pc.Start == pc.End){
        s += to_string(pc.Start);
        s += ":";
    }
    else if(pc.End < pc.Start){
        s += to_string(pc.End);
        s+=":";
    }
    else{
        s += to_string(pc.Start) + "-" + to_string(pc.End);
        s += ":";
    }
    
    vector<string> countvector;
    boost::split(countvector, pc.Command, boost::is_any_of(" "));
    if(InstructionCountVector.find(countvector[0]) != InstructionCountVector.end()){
        InstructionCountVector[countvector[0]]++;
    }

    cout << boost::format("%-20s %-55s %-50s\n") % s % pc.Command % pc.Execution;
    if(pc.Execution!=""){
        vector<string> temp;
        boost::split(temp,pc.Execution,boost::is_any_of(" "));
        if(temp[0] == "Address"){
            MemoryContent[temp[1]] = temp[3];
        }
    }
}

void AllotMemory(){
    if(instr.size() <= TotalMemory){

        MemoryAvailable = 4*(TotalMemory - instr.size());
        // Considered offset to be a multiple of 4
        Dram.resize(RowMemory);
        for(int i=0;i<RowMemory;i++){
            Dram[i].resize(ColumnMemory);
        }

    }
    else {
        print_msg = "ERROR : Overflow\n";
        flag = true;
        return;
    }

}

void tokenize(string s){
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

// Recursive function
void ResolveDependency(instruction &ins){
    
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
    pr.Start = pr.End = NumberOfCycles + 1;
    pr.Command = ins.original;
    pr.Execution = "DRAM: Request Issued";
    Command.push_back(pr);
    pr.Execution = "";
    
    if(RowBuffer == ins.row){
        pr.Start = NumberOfCycles + 2;
        pr.End = pr.Start + ColumnDelay - 1;
        pr.Command = "DRAM: Column Access (" + to_string(ColumnNumber) + ")" + " (Ins: " + ins.original + ")";
        if(ins.op == "lw"){
            pr.Execution = ins.target + " = " + to_string(Dram[RowNumber][ColumnNumber]);
        }
        else{
            pr.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(reg[ins.target]);
        }
        Command.push_back(pr);
        NumberOfCycles = pr.End;
        // StartTime = NumberOfCycles+2;
        // EndTime = NumberOfCycles + 1 + ColumnDelay;
    }

    else if(RowBuffer == -1){ // have to initiate the process
        
        RowBufferUpdates++;
        
        // StartTime = NumberOfCycles + 2;
        // EndTime = NumberOfCycles + 1 + RowDelay + ColumnDelay;
        pr.Start = NumberOfCycles + 2;
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
        NumberOfCycles = pr.End;
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

            pr.Start = NumberOfCycles + 2;
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
        }
        else{
            // Some changes took place in the Row Buffer, hence, we need to writeback that row to the DRAM
            // StartTime = NumberOfCycles + 2;
            // EndTime = NumberOfCycles + 1 + RowDelay*2 + ColumnDelay;
            
            pr.Start = NumberOfCycles + 2;
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
        }
        NumberOfCycles = pr.End;
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

void parse(){
    int i = 0;
    while(i<instr.size()){
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
            if(RegisterUpdation[ins.target].ff >=0 || RegisterUse[ins.target].ff >=0 || RegisterUpdation[ins.source1].ff >=0 || RegisterUpdation[ins.source2].ff >=0 ){
                // Pura Tandav 
                // COMPLEX
                if(RegisterUpdation[ins.target].ff >=0)ResolveDependency(QueueInstruction[RegisterUpdation[ins.target].ss]);
                if(RegisterUse[ins.target].ff >=0)ResolveDependency(QueueInstruction[RegisterUse[ins.target].ss]);
                if(RegisterUpdation[ins.source1].ff >=0)ResolveDependency(QueueInstruction[RegisterUpdation[ins.source1].ss]);
                if(RegisterUpdation[ins.source2].ff >=0)ResolveDependency(QueueInstruction[RegisterUpdation[ins.source2].ss]);
            }
            
            PrintCommand pr;
            pr.Start = NumberOfCycles + 1;
            pr.End = pr.Start;
            pr.Command = ins.original;
            
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
            if( RegisterUse[ins.source1].ff >=0 || RegisterUse[ins.source2].ff >=0){

                if(RegisterUpdation[ins.source1].ff >=0)ResolveDependency(QueueInstruction[RegisterUse[ins.source1].ss]);
                if(RegisterUpdation[ins.source2].ff >=0)ResolveDependency(QueueInstruction[RegisterUse[ins.source2].ss]);

            }

            PrintCommand pr;
            pr.Start = NumberOfCycles+1;
            pr.End = pr.Start;
            pr.Command = ins.original;

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
            i = label[ins.jump];
            NumberOfCycles++;
        }

        else if(ins.op == "lw" || ins.op == "sw"){
            if( reg.find(ins.target) == reg.end() || reg.find(ins.source1) == reg.end() || !is_number(ins.offset)){
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

                    RegisterUse[ins.source1] = mp(i,QueueInstruction.size());
                    RegisterUpdation[ins.target] = mp(i,QueueInstruction.size());
                    MemoryUse[x] = mp(i,QueueInstruction.size());
                }

                else{

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
                        pr.Start = NumberOfCycles + 2;
                        pr.End = pr.Start + ColumnDelay - 1;
                        pr.Command = "DRAM: Column Access (" + to_string(ColumnNumber) + ")" + " (Ins: " + ins.original + ")";                                        
                        pr.Execution = ins.target + " = " + to_string(Dram[RowNumber][ColumnNumber]);
                        reg[ins.target] = Dram[RowNumber][ColumnNumber];
                        Command.pb(pr);
                        NumberOfCycles = pr.End;
                    }

                }

                else{

                    if(RegisterUpdation[ins.source1].ff >= 0 || MemoryUpdation[x].ff >=0 || MemoryUse[x].ff >=0 || RegisterUpdation[ins.target].ff >= 0){
                        ins.dependent[0] = RegisterUpdation[ins.source1];
                        ins.dependent[1] = MemoryUpdation[x];
                        ins.dependent[2] = MemoryUse[x];
                        ins.dependent[3] = RegisterUpdation[ins.target];

                        MemoryUpdation[x] = mp(i,QueueInstruction.size());
                        RegisterUse[ins.source1] = mp(i,QueueInstruction.size());
                        RegisterUse[ins.target] = mp(i,QueueInstruction.size());
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
                        pr.Start = NumberOfCycles + 2;
                        pr.End = pr.Start + ColumnDelay - 1;
                        pr.Command = "DRAM: Column Access (" + to_string(ColumnNumber) + ")" + " (Ins: " + ins.original + ")";                                             
                        pr.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(reg[ins.target]);

                        // if(Dram[RowNumber][ColumnNumber] != reg[ins.target])
                        RowBufferUpdates++;
                        Dram[RowNumber][ColumnNumber] = reg[ins.target];
                        Command.pb(pr);
                        NumberOfCycles = pr.End;
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
                flag = !is_number(ins.source1);
                if(flag){print_msg = "ERROR : Not a number\n"; return;}
                x = stoi(ins.source1);
            }
            else{  // First symbol is $ and is present in Register file
                x = reg[ins.source1];
                if(RegisterUpdation[ins.source1].ff >=0){
                    // cout << "Hey" << "\n";
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
                
                flag = !is_number(ins.source2);
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

            if(RegisterUpdation[ins.target].ff >=0 || RegisterUse[ins.target].ff >=0){
                // cout << ins.source1 << " " << ins.source2 << endl;
                // cout << "Hey\n";
                if(RegisterUpdation[ins.target].ff >= 0)ResolveDependency(QueueInstruction[RegisterUpdation[ins.target].ss]);
                if(RegisterUse[ins.target].ff >= 0)ResolveDependency(QueueInstruction[RegisterUse[ins.target].ss]);
                // continue;
            }

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

        // if(ins.op!="")
        // {
        //     cout << "Instruction : " << ins.original << endl;
        //     print_reg();
        //     // print_ins(ins);
        //     print_count++;
        // }
    }
}

int main(int argc, char * argv[]){
    if(argc >= 2 && argc <= 4){
        ifstream infile(argv[1]);
        if(argc >= 3){
            if(!is_number(argv[2])){
                cout << "Invalid Input: ROW_ACCESS_DELAY should be an integer\n";
                return -1;
            }
            RowDelay = stoi(argv[2]);
        }
        if(argc == 4){
            if(!is_number(argv[3])){
                cout << "Invalid Input: COLUMN_ACCESS_DELAY should be an integer\n";
                return -1;
            }
            ColumnDelay = stoi(argv[3]);
        }
        vector<string> text_file;
        if(infile.is_open()){
            
            std_registers();
            int ln=1;
            
            for(string line;getline(infile,line);){
                text_file.pb(line);
                tokenize(line);
                if(flag){
                    cout << print_msg;
                    cout << "Line Number: " << ln << ": "<< line << endl;
                    return -1;
                }
                ln++;
            }
            
            
            if(ln==1){cout << "Empty File\n";return 0;}

            AllotMemory();
            
            if(flag){cout << print_msg;return -1;}

            // cout <<"Hey\n";
            parse();
            
            if(flag){
                cout << print_msg;
                cout << "Line Number: " << ErrorLine << ": "<< instr[ErrorLine].original << endl;
                // print_map(label);
                return -1;
            }
            else{
                
                // cout << printp(RegisterUpdation["$t1"] << " " << RegisterUse["$t1"] << " " << RegisterUpdation["$s0"] << " " << RegisterUse["$s0"] << "\n";
                // printp(RegisterUpdation["$t1"]);
                // printp(RegisterUse["$t1"]);
                // printp(RegisterUpdation["$s0"]);
                // printp(RegisterUse["$s0"]);

                // for(int i=QueueInstruction.size()-1;i>=0;i--){
                //     // cout << (QueueInstruction.size()) << endl;
                //     print_ins(QueueInstruction[i]);
                //     // ResolveDependency(QueueInstruction[i]);
                // }

                for(int i=QueueInstruction.size()-1;i>=0;i--){
                    // cout << (QueueInstruction.size()) << endl;
                    // print_ins(QueueInstruction[i]);
                    ResolveDependency(QueueInstruction[i]);
                }
                
                sort(Command.begin(),Command.end(),Comparator);
                if(RowBuffer != -1 && ( RowBufferCopy != Dram[RowBuffer] || !DesignDecision)){
                    PrintCommand pr;
                    pr.Start = Command[Command.size()-1].End + 1;
                    pr.End = pr.Start + RowDelay - 1;
                    pr.Command = "DRAM: Writeback row " + to_string(RowBuffer);
                    Command.pb(pr);
                }
                
                cout << "ASSIGNMENT 4 BLOCKING\n\n";
                // if(Part2)cout << "PART 2\n\n";
                // else cout << "PART 1\n\n";
                cout << "ROW_ACCESS_DELAY: " << RowDelay << "\n";
                cout << "COLUMN_ACCESS_DELAY: " << ColumnDelay << "\n";
                cout << "Clock Cycles with Last Row Writeback(if any): " << Command[Command.size()-1].End<< "\n\n";
                cout << "Cycle Wise Analysis\n\n";
                cout << boost::format("%-20s %-55s %-50s\n") % "Cycle Count" % "Instruction" % "Register/Memory/Request";
                for(int i=0;i<Command.size();++i){
                    print_command(Command[i]);
                }

                cout << "======================================\n";
                cout << "Program Execeuted Successfully\n";
                cout << "======================================\n";
                
                cout << "Program Statistics\n";
                
                cout << "Row Buffer Updates : " << RowBufferUpdates << endl;
                bool heading = false;
                for(auto i = MemoryContent.begin();i!=MemoryContent.end();i++){
                    if(!heading)cout << "\nMemory Content\n";
                    heading = true;
                    if( i->second != "0")
                    cout << i->first << ": " << i->second << "\n";
                }
                cout << endl;
                cout << boost::format("%-17s %-2s %5s") % "Operation Count" % " : " % print_count;
                cout << endl;
                for(auto i = InstructionCountVector.begin();i!=InstructionCountVector.end();i++){
                    // cout << i->first << " " << i->second << endl;
                    cout << boost::format("%-17s %-2s %5s") % i->first % " : " % i->second;
                    cout << "\n";
                }

                cout << "\nInstruction Count : " << print_count  << endl;
                cout << "Instruction Execution Count:\n";
                int j = 1;
                for(int i=0;i<instr.size();i++){
                    if(instr[i].original!=""){
                        string val1 = to_string(j) + ". ";
                        // string val2 = instr[i].original + " : ";
                        cout << boost::format("%-5s %-30s %-3s %-3s\n") % val1 % instr[i].original % "=>" % instr[i].InstructionCount;
                        // cout << j << ". " << instr[i].original << " : "  << instr[i].InstructionCount <<  "\n";
                        j++;
                    }
                }
            }
        }
        else cout << "File not found.\n";
    }
    else cout << "Incorrect Input\n";
}
