#pragma once

#include "main.hpp"

using namespace std;

string Registers::strip(string s){
    // cout << "->" << s << "<-\n";
    int start=0;
    while(start<s.size() && s[start] == ' ' || s[start] == '\t')start++; // removing whitespace from tokens
    if(start == s.size()){flag=true;print_msg = "ERROR : Syntax Error\n";return "";}
    int end = s.size()-1;
    while(end >= 0 && s[end] == ' ' || s[end] == '\t')end--;
    string s_new = s.substr(start,end-start+1);
    return s_new;
}

void   Registers::print_reg(){
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

void   Registers::print_ins(instruction ins){
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

void   Registers::print_map(map<string,int> m){
    for(auto i=m.begin(); i!=m.end();i++){
        cout << "->" << i->ff << "<-->" << i->second <<"<-\n"; 
    }
}  

void   Registers::std_registers(){

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

bool   Registers::Comparator(PrintCommand a, PrintCommand b){
    if(a.End!=b.End)return a.End < b.End;
    return a.Start < b.Start;
}

bool   Registers::is_number(const string& s)
{
    string::const_iterator it = s.begin();
    if(*it=='-')it++;
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool   Registers::AllotMemory(){
    if(instr.size() <= TotalMemory){
        
        MemoryAvailable = 4*(TotalMemory - instr.size());
        
        TotalMemory = TotalMemory - instr.size(); // To account for multiple files        
        
        // Considered offset to be a multiple of 4
        Dram.resize(RowMemory);
        for(int i=0;i<RowMemory;i++)Dram[i].resize(ColumnMemory);
    }
    else {
        cout << "ERROR : Insufficient Memory\n";
        flag = true;
        return false;
    }
    return true;
}

void   Registers::print_command_debug(PrintCommand pc){
    cout << "Start : ->" << pc.Start << "<-\n";
    cout << "End : ->" << pc.End << "<-\n";
    cout << "Command : ->" << pc.Command << "<-\n\n";
}

void   Registers::print_command(PrintCommand pc){
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

void   Registers::Printing(){

    cout << "ASSIGNMENT 5\n\n";
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
            cout << boost::format("%-5s %-30s %-3s %-3s\n") % val1 % instr[i].original % "=>" % instr[i].InstructionCount;        
            j++;
        }
    }
    cout << "\n";
}
