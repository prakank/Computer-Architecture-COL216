#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<map>
#include <boost/algorithm/string.hpp>

using namespace std;
#define ll long long
typedef vector<ll> vi; 
vector<int> instr_count;
map<string,int> reg,label;
bool flag = false;



struct instruction{
    string op="",target="",source1="",source2="";
    string jump = "";
    string offset = "";
    string fun_label="";
};

vector<instruction> instr;

// First job is to parse over the file and store the instructions in instruction memory 

string strip(string s){
    // cout << "->" << s << "<-\n";
    int start=0;
    while(start<s.size() && s[start] == ' ' || s[start] == '\t')start++; // removing whitespace from tokens
    if(start == s.size()){flag=true;return "";}
    int end = s.size()-1;
    while(end >= 0 && s[end] == ' ' || s[end] == '\t')end--;
    string s_new = s.substr(start,end-start+1);
    return s_new;
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

    // This is some function whose label needs to be stored
    // Assuming LO and lo represent 2 different functions
    
    // All labels are converted to the form
    /*
        label:
                ......       
                ......
    */  
    while(i<s.length() && (s[i] == ' ' || s[i] == '\t'))i++;

    if(fun.size() == 0)return;
    if(i == s.size()){flag=true;return;}
    s = s.substr(i,s.size()-i);
    
    vector<string> tokens;
    boost::split(tokens,s,boost::is_any_of(","));
    
    if(tokens.size()>3){flag=true;return;}
    for(int i=0;i<tokens.size();i++){        
        tokens[i] = strip(tokens[i]);
        if(flag)return;
    }
    
    transform(fun.begin(),fun.end(),fun.begin(),::tolower);
    ins.op = fun;
    
    if(fun == "add" || fun == "sub" || fun == "mul" || fun == "slt" || fun == "addi"){
        if(tokens.size()!=3){flag=true;return;}
        ins.target  = tokens[0];
        ins.source1 = tokens[1];
        ins.source2 = tokens[2];    
    }
    else if(fun == "beq" || fun == "bne"){
        if(tokens.size()!=3){flag=true;return;}
        ins.source1 = tokens[0];
        ins.source2 = tokens[1];
        ins.jump = tokens[2];
    }
    else if(fun == "j"){
        if(tokens.size()!=1){flag=true;return;}
        ins.jump = tokens[0];    
    }
    else if(fun == "lw" || fun == "sw"){
        if(tokens.size()!=2){flag=true;return;}
        ins.target = tokens[0];
        i=0;
        s="";
        while(i<tokens[1].size() && tokens[1][i]!='(' && (tokens[1][i] - '0' <= 9) && (tokens[1][i] - '0' >= 0) ){s+=tokens[1][i];i++;}
        if(i == tokens[1].size() || tokens[1][i]!='('){flag=true;return;}
        ins.offset = s;
        ins.source1 = tokens[1].substr(i+1,tokens[1].size() - i - 2);
    }
    else{
        flag = true;
        return;
    }
    instr.push_back(ins);
}

bool is_number(const string& s)
{
    string::const_iterator it = s.begin();
    if(*it=='-')it++;
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void print_reg(){
    int cnt = 0;
    for (auto i = reg.begin(); i != reg.end(); i++){
        cout << i->first << ":" << i->second;
        cnt++;
        if(cnt%10==0 || cnt==32)cout<<"\n";
        else cout<<", ";
    }
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

int print_count = 1;

void parse(){
    int i = 0;
    while(i<instr.size()){
        instruction ins = instr[i];
        
        // print_ins(ins);

        if(ins.op == "add"){
            if( reg.find(ins.target) == reg.end() || reg.find(ins.source1) == reg.end() || reg.find(ins.source2) == reg.end() ){
                flag = true;
                return;
            }
            reg[ins.target] = reg[ins.source1] + reg[ins.source2];
            i++;
        }

        else if(ins.op == "sub"){
            if( reg.find(ins.target) == reg.end() || reg.find(ins.source1) == reg.end() || reg.find(ins.source2) == reg.end() ){
                flag = true;
                return;
            }
            reg[ins.target] = reg[ins.source1] - reg[ins.source2];
            i++;
        }

        else if(ins.op == "mul"){
            if( reg.find(ins.target) == reg.end() || reg.find(ins.source1) == reg.end() || reg.find(ins.source2) == reg.end() ){
                flag = true;
                return;
            }
            reg[ins.target] = reg[ins.source1] * reg[ins.source2];
            i++;
        }

        else if(ins.op == "slt"){
            if( reg.find(ins.target) == reg.end() || reg.find(ins.source1) == reg.end() || reg.find(ins.source2) == reg.end() ){
                flag = true;
                return;
            }
            reg[ins.target] = reg[ins.source1] + reg[ins.source2];
            if(reg[ins.source1] < reg[ins.source2])reg[ins.target] = 1;
            else reg[ins.target] = 0;
            i++;
        }

        else if(ins.op == "beq"){
            if( reg.find(ins.source1) == reg.end() || reg.find(ins.source2) == reg.end() || label.find(ins.jump) == label.end()){
                flag = true;
                return;
            }
            if(reg[ins.source1] == reg[ins.source2]){
                i = label[ins.jump];                
            }
            else i++;
        }

        else if(ins.op == "bne"){
            if( reg.find(ins.source1) == reg.end() || reg.find(ins.source2) == reg.end() || label.find(ins.jump) == label.end()){
                flag = true;
                return;
            }
            if(reg[ins.source1] != reg[ins.source2]){
                i = label[ins.jump];                
            }
            else i++;
        }

        else if(ins.op == "j"){
            if( label.find(ins.jump) == reg.end()){
                // print_ins(ins);
                flag = true;
                return;
            }
            i = label[ins.jump];
        }

        else if(ins.op == "lw"){

        }

        else if(ins.op == "sw"){

        }

        else if(ins.op == "addi"){
            if( reg.find(ins.target) == reg.end()){
                flag = true;
                return;
            }
            int x = 0, y = 0;
            // cout << ins.source1 << " " << ins.source2 << endl;
            if(ins.source1[0] == '$' && reg.find(ins.source1) == reg.end()){
                flag = true;
                return;
            }            
            else if(ins.source1[0]!='$'){
                flag = !is_number(ins.source1);
                if(flag)return;
                x = stoi(ins.source1);
            }
            else{  // First symbol is $ and is present in Register file
                x = reg[ins.source1];
            }
            // cout << ins.source1 << " " << ins.source2 << endl;
            if(ins.source2[0] == '$' && reg.find(ins.source2) == reg.end()){
                flag = true;
                return;
            }            
            else if(ins.source2[0]!='$'){
                flag = !is_number(ins.source2);
                if(flag)return;
                y = stoi(ins.source2);
            }
            else{  // First symbol is $ and is present in Register file
                y = reg[ins.source2];
            }
            reg[ins.target] = x+y;
            i++;
        }
        else{
            i++;
        }
        
        if(ins.op!="")
        {
            cout << "Instruction Number : " << print_count << endl;
            print_reg();
            print_count++;
        }


    }
}

void std_registers(){
    reg["$t0"]=0;reg["$t1"]=0;reg["$t2"]=0;reg["$t3"]=0;reg["$t4"]=0;reg["$t5"]=0;reg["$t6"]=0;reg["$t7"]=0;reg["$t8"]=0;reg["$t9"]=0;
    reg["$s0"]=0;reg["$s1"]=0;reg["$s2"]=0;reg["$s3"]=0;reg["$s4"]=0;reg["$s5"]=0;reg["$s6"]=0;reg["$s7"]=0;reg["$s8"]=0;reg["$s9"]=0;
    reg["$r0"]=0;reg["$r1"]=0;reg["$r2"]=0;reg["$r3"]=0;reg["$r4"]=0;reg["$r5"]=0;reg["$r6"]=0;reg["$r7"]=0;reg["$r8"]=0;reg["$r9"]=0;
    reg["$v0"]=0;reg["$v1"]=0;
}

void InstructionCount(){
    instr_count.assign(instr.size(),0);
}

int main(int argc, char * argv[]){
    if(argc  ==  2){
        ifstream infile(argv[1]);
        if(infile.is_open()){
            std_registers();
            int ln=1;
            for(string line;getline(infile,line);){
                tokenize(line);
                if(flag){
                    cout << "ERROR in Line Number: " << ln << ": "<< line << endl;
                    return -1;
                }
                ln++;
            }
            parse();

            if(flag){
                cout << "Unknown Register or Function name\n";

                // for(auto i=label.begin(); i!=label.end();i++){
                //     cout << "->" << i->first << "<-->" << i->second <<"<-\n"; 
                // }

                return -1;
            }
        }
        else cout << "File not found.\n";
    }
    else cout << "Incorrect Input\n";
}
