#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<iomanip>
#include<map>
#include<sstream>
#include<boost/algorithm/string.hpp>
#include<boost/format.hpp> 

using namespace std;
map<string,int> reg, label, RegisterUpdation, RegisterUse;

string print_msg = "";
vector<vector<int>> Dram; // Dynamic Random Access Memory

int RowDelay           = 10;
int ColumnDelay        = 2;
int TotalMemory_4Bytes = 1048576;
int RowMemory          = 1024;
int ColumnMemory       = 1024;
int RowBuffer          = -1;                   // Assumption - We have only 1 Row Buffer (otherwise, we could have used an array)
                                               // Also defined a vector named RowBufferCopy to store the orginal copy of row copied to row buffer

int TotalMemory        = TotalMemory_4Bytes/4;
int print_count        = 1;
int ErrorLine          = -1;
int NumberOfCycles     = 0;
int StartTime          = -1;
int EndTime            = -1;
int RowBufferUpdates   = 0;
int MemoryAvailable;

bool Part2 = false;
bool flag  = false;
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
};

vector<instruction> instr;
vector<PrintCommand> Command;

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
    reg["$t0"]=0;reg["$t1"]=0;reg["$t2"]=0;reg["$t3"]=0;reg["$t4"]=0;reg["$t5"]=0;reg["$t6"]=0;reg["$t7"]=0;reg["$t8"]=0;reg["$t9"]=0;
    reg["$s0"]=0;reg["$s1"]=0;reg["$s2"]=0;reg["$s3"]=0;reg["$s4"]=0;reg["$s5"]=0;reg["$s6"]=0;reg["$s7"]=0;reg["$s8"]=0;reg["$s9"]=0;
    reg["$r0"]=0;reg["$r1"]=0;reg["$r2"]=0;reg["$r3"]=0;reg["$r4"]=0;reg["$r5"]=0;reg["$r6"]=0;reg["$r7"]=0;reg["$r8"]=0;reg["$r9"]=0;
    reg["$v0"]=0;reg["$zero"]=0;

    // reg["$r0"]=0;reg["$r1"]=0;reg["$r2"]=0;reg["$r3"]=0;reg["$r4"]=0;reg["$r5"]=0;reg["$r6"]=0;reg["$r7"]=0;reg["$r8"]=0;reg["$r9"]=0;
    // reg["$r10"]=0;reg["$r11"]=0;reg["$r12"]=0;reg["$r13"]=0;reg["$r14"]=0;reg["$r15"]=0;reg["$r16"]=0;reg["$r17"]=0;reg["$r18"]=0;reg["$r19"]=0;
    // reg["$r20"]=0;reg["$r21"]=0;reg["$r22"]=0;reg["$r23"]=0;reg["$r24"]=0;reg["$r25"]=0;reg["$r26"]=0;reg["$r27"]=0;reg["$r28"]=0;reg["$r29"]=0;
    // reg["$r30"]=0;reg["$zero"]=0;

    for(auto i = reg.begin(); i != reg.end(); i++){RegisterUpdation[i->first] = 0; RegisterUse[i->first] = 0;}

}

void print_reg(){
    int cnt = 0;
    for (auto i = reg.begin(); i != reg.end(); i++){
        stringstream ss;
        ss << hex << i->second;
        string res(ss.str());
        cout << i->first << ":" << res << ",   ";
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
    cout << "Fun_lable : ->" << ins.fun_label << "<-\n\n";
}

void print_map(map<string,int> m){
    for(auto i=m.begin(); i!=m.end();i++){
        cout << "->" << i->first << "<-->" << i->second <<"<-\n"; 
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
    cout << boost::format("%-20s %-35s %-50s\n") % s % pc.Command % pc.Execution;
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
        instr.push_back(ins);
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
        if(i == tokens[1].size() || tokens[1][i]!='('){flag=true;print_msg = "ERROR : Syntax Error\n";return;}
        ins.offset = s;
        ins.source1 = tokens[1].substr(i+1,tokens[1].size() - i - 2);
    }
    else{
        flag = true;
        print_msg = "ERROR : Operation not Defined\n";
        return;
    }
    
    instr.push_back(ins);
}

void parse(){
    int i = 0;
    while(i<instr.size()){
        instruction ins = instr[i];
        instr[i].InstructionCount++;
        ErrorLine = i;
        RegisterUpdation["$zero"] = 0; // Initializing the zero register's RegisterUpdation value to 0 in each loop
        RegisterUse["$zero"] = 0;      // Initializing the zero register's RegisterUse value to 0 in each loop

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
            if(RegisterUpdation[ins.target] > NumberOfCycles || RegisterUpdation[ins.source1] > NumberOfCycles || RegisterUpdation[ins.source2] > NumberOfCycles || RegisterUse[ins.target] > NumberOfCycles){
                NumberOfCycles = EndTime;
                continue;
            }
            else{
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
                Command.push_back(pr);
                i++;
                NumberOfCycles++;
                // ClockCounter.push_back(make_pair(NumberOfCycles+1, NumberOfCycles+1));
                // CommandCounter.push_back(ins.original);
            }
            
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
            if( RegisterUpdation[ins.source1] > NumberOfCycles || RegisterUpdation[ins.source2] > NumberOfCycles){
                NumberOfCycles = EndTime;
                continue;
            }
            else{
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
                Command.push_back(pr);
                NumberOfCycles++;
            }
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
            Command.push_back(pr);
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

            if(x < MemoryAvailable){
               
                
                if(EndTime <= NumberOfCycles){                    
                    
                    PrintCommand pr;
                    pr.Start = pr.End = NumberOfCycles + 1;
                    pr.Command = ins.original;
                    pr.Execution = "DRAM: Request Issued";
                    Command.push_back(pr);
                    pr.Execution = "";

                    if(RowBuffer == ins.row){
                        pr.Start = NumberOfCycles + 2;
                        pr.End = pr.Start + ColumnDelay - 1;
                        pr.Command = "DRAM: Column Access";                        
                        if(ins.op == "lw"){
                            pr.Execution = to_string(reg[ins.target]) + " = " + to_string(Dram[RowNumber][ColumnNumber]);
                        }
                        else{
                            pr.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(reg[ins.target]);
                        }
                        Command.push_back(pr);
                        // ClockCounter.push_back(make_pair(NumberOfCycles + 1,NumberOfCycles + ColumnDelay ));
                        // CommandCounter.push_back("DRAM : Column Access");
                        StartTime = NumberOfCycles+2;
                        EndTime = NumberOfCycles + 1 + ColumnDelay;
                    }

                    else if(RowBuffer == -1){ // have to initiate the process
                        
                        RowBufferUpdates++;
                        
                        StartTime = NumberOfCycles + 2;
                        EndTime = NumberOfCycles + 1 + RowDelay + ColumnDelay;
                        pr.Start = NumberOfCycles + 2;
                        pr.End = pr.Start + RowDelay - 1;
                        pr.Command = "DRAM: Activate row " + to_string(ins.row);
                        Command.push_back(pr);
                        pr.Start = pr.End + 1;
                        pr.End = pr.Start + ColumnDelay -1;
                        pr.Command = "DRAM: Column Access";
                        if(ins.op == "lw"){
                            pr.Execution = to_string(reg[ins.target]) + " = " + to_string(Dram[RowNumber][ColumnNumber]);
                        }
                        else{
                            pr.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(reg[ins.target]);
                        }
                        Command.push_back(pr);
                        RowBuffer = ins.row;
                        RowBufferCopy = Dram[ins.row];
                    }

                    else{
                        // ins.row is the New Row
                        // RowBuffer is the one which is in the Buffer
                        RowBufferUpdates++;
                        if(RowBufferCopy == Dram[ins.row]){
                            // No need to writeback if there is no updation in the row
                            
                            StartTime = NumberOfCycles + 2;
                            EndTime = NumberOfCycles + 1 + RowDelay + ColumnDelay;

                            pr.Start = NumberOfCycles + 2;
                            pr.End = pr.Start + RowDelay - 1;
                            pr.Command = pr.Command = "DRAM: Activate row " + to_string(ins.row);
                            Command.push_back(pr);
                            
                            pr.Start = pr.End + 1;
                            pr.End = pr.Start + ColumnDelay - 1;
                            pr.Command = "DRAM: Column Access";
                            
                            if(ins.op == "lw")pr.Execution = to_string(reg[ins.target]) + " = " + to_string(Dram[RowNumber][ColumnNumber]);
                            else pr.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(reg[ins.target]);
                            
                            Command.push_back(pr);
                            RowBuffer = ins.row;
                        }
                        else{
                            // Some changes took place in the Row Buffer, hence, we need to writeback that row to the DRAM
                            StartTime = NumberOfCycles + 2;
                            EndTime = NumberOfCycles + 1 + RowDelay*2 + ColumnDelay;
                            
                            pr.Start = NumberOfCycles + 2;
                            pr.End = pr.Start + RowDelay - 1;
                            pr.Command = "DRAM: Writeback row " + to_string(RowBuffer);
                            Command.push_back(pr);
                            
                            pr.Start = pr.End + 1;
                            pr.End = pr.Start + RowDelay - 1;
                            pr.Command = "DRAM: Activate row " + to_string(ins.row);
                            Command.push_back(pr);
                            
                            pr.Start = pr.End + 1;
                            pr.End = pr.Start + ColumnDelay -1;
                            pr.Command = "DRAM: Column Access";
                            
                            if(ins.op == "lw")pr.Execution = to_string(reg[ins.target]) + " = " + to_string(Dram[RowNumber][ColumnNumber]);
                            else pr.Execution = "Address " + to_string(x) +  "-" + to_string(x+3) + " = " + to_string(reg[ins.target]);
                            
                            Command.push_back(pr);
                            RowBuffer = ins.row;
                        }
                    }

                    // RegisterUpdation[ins.target] = RegisterUpdation[ins.source1] = EndTime;
                    if(ins.op == "lw"){
                        RegisterUpdation[ins.target] = EndTime;
                        RegisterUse[ins.source1] = EndTime;
                    }
                    else{
                        RegisterUse[ins.target] = EndTime;
                        RegisterUse[ins.source1] = EndTime;
                    }
                    
                    
                    if(ins.op == "lw")reg[ins.target] = Dram[RowNumber][ColumnNumber];
                    else {Dram[RowNumber][ColumnNumber] = reg[ins.target]; RowBufferUpdates++;}
                    
                    NumberOfCycles++; // Will take 1 cycle to read instruciton.
                }

                else {NumberOfCycles = EndTime; continue;}                            
            
            }
            else {
                flag = true;
                print_msg = "ERROR : Overflow\n";
                return;
            }
            if(!Part2)NumberOfCycles = EndTime;   // For Part1, No instruction can be skipped.
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

            // cout << ins.source1 << " " << ins.source2 << endl;

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
                if(RegisterUpdation[ins.source1] > NumberOfCycles){
                    NumberOfCycles = EndTime;
                    continue;
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
                if(RegisterUpdation[ins.source1] > NumberOfCycles){
                    NumberOfCycles = EndTime;
                    continue;
                }
            }

            // cout << "Hey  "  << ins.target << "  " <<  NumberOfCycles <<   "\n" ;

            if(RegisterUpdation[ins.target] > NumberOfCycles || RegisterUse[ins.target] > NumberOfCycles){
                NumberOfCycles = EndTime;
                continue;
            }

            PrintCommand pr;
            pr.Start = NumberOfCycles + 1;
            pr.End = pr.Start;
            pr.Command = ins.original;
            reg[ins.target] = x+y;
            pr.Execution = ins.target + " = " + to_string(reg[ins.target]);
            Command.push_back(pr);
            
            i++;
            NumberOfCycles++;
        }
        else i++; //  This is for the labels like "main:" which will not be covered in any if the cases above.  

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
    if(argc >= 3 && argc <= 5){
        if(!is_number(argv[2])){
            cout << "Invalid Input: Part can only be 1 or 2\n";
            return -1;
        }
        int PartNumber = stoi(argv[2]);
        if(PartNumber > 2 || PartNumber < 1){
            cout << "Invalid Input: Part can only be 1 or 2\n";
            return -1;
        }
        if(PartNumber == 2)Part2 = true;
        ifstream infile(argv[1]);
        if(argc >= 4){
            if(!is_number(argv[3])){
                cout << "Invalid Input: ROW_ACCESS_DELAY should be an integer\n";
                return -1;
            }
            RowDelay = stoi(argv[3]);            
        }
        if(argc == 5){
            if(!is_number(argv[4])){
                cout << "Invalid Input: COLUMN_ACCESS_DELAY should be an integer\n";
                return -1;
            }
            ColumnDelay = stoi(argv[4]);
        }
        vector<string> text_file;
        if(infile.is_open()){
            
            std_registers();
            int ln=1;
            
            for(string line;getline(infile,line);){
                text_file.push_back(line);
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
                
                
                if(RowBuffer != -1 && RowBufferCopy != Dram[RowBuffer]){
                    PrintCommand pr;
                    pr.Start = Command[Command.size()-1].End + 1;
                    pr.End = pr.Start + RowDelay - 1;
                    pr.Command = "DRAM: Writeback row " + to_string(RowBuffer);
                    Command.push_back(pr);
                }
                sort(Command.begin(),Command.end(),Comparator);
                if(Part2)cout << "PART 2\n\n";
                else cout << "PART 1\n\n";
                cout << "ROW_ACCESS_DELAY: " << RowDelay << "\n";
                cout << "COLUMN_ACCESS_DELAY: " << ColumnDelay << "\n";
                cout << "Clock Cycles with Last Row Writeback(if any): " << Command[Command.size()-1].End<< "\n\n";
                cout << "Cycle Wise Analysis\n\n";
                cout << boost::format("%-20s %-35s %-50s\n") % "Cycle Count" % "Instruction" % "Register/Memory/Request";
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
                    // if( i->second != "0")
                    cout << i->first << ": " << i->second << "\n";
                }
                // cout << "Instruction Count : " << print_count - 1 << endl;
                // cout << "Instruction Execution Count:\n";
                // int j = 1;
                // for(int i=0;i<instr.size();i++){
                //     if(instr[i].original!=""){
                //         cout << j << ". " << instr[i].original << " : "  << instr[i].InstructionCount <<  "\n";
                //         j++;
                //     }
                // }
            }
        }
        else cout << "File not found.\n";
    }
    else cout << "Incorrect Input\n";
}
