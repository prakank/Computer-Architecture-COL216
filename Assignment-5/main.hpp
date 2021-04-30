#pragma once

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

#define ff              first
#define pb              push_back
#define ss              second
#define mp              make_pair
#define pii             pair<int,int>
#define printp(x)       cout << "(" <<  x.ff << "," << x.ss << ") "
#define QUEUE_CAPACITY  32

using namespace std;

map<int,pair<int,int> > MemoryUpdation, MemoryUse;
vector<vector<int> > Dram;

int RowDelay           = 10;
int ColumnDelay        = 2;
int TotalMemory_4Bytes = 1048576;
int RowMemory          = 1024;
int ColumnMemory       = 1024;
int TotalMemory        = TotalMemory_4Bytes/4;
int NumberOfCycles     = 0;
int StartTime          = -1;
int EndTime            = -1;
int MemoryAvailable;
/* int RowBuffer          = -1;*/                                 
/* int RowBufferUpdates   = 0; */

// Assignment-5
int CPU = 2;
string BaseFilename = "t";
// Assignment-5

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
    int InstructionRead = -1;
    int row = -1;
    vector<pair<int,int> > dependent; // Can be of length at most 4.
    // Different index will contain different instructions 
    // Kind of instructions will also vary with lw, sw
};

class Registers{
    public:
        int print_count        = 0;
        int ErrorLine          = -1;
        int RowBuffer          = -1;
        int NumberOfCycles     =  0;
        int StartTime          = -1;
        int EndTime            = -1;
        int RowBufferUpdates   =  0;

        string print_msg = "";
        map<string,int> reg, label;
        map<string,pair<int,int> > RegisterUpdation, RegisterUse;
        map<string,int> InstructionCountVector;
        vector<instruction> instr;
        vector<instruction> QueueInstruction;
        vector<PrintCommand> Command;

        void std_registers();
        
        void print_reg();
        void print_ins(instruction ins);
        void print_map(map<string,int> m);
        void print_command(PrintCommand pc);
        
        void tokenize(string s);
        bool AllotMemory();
        void parse();
        bool ParsingExecution(string File);
        void ResolveDependency(instruction &ins);
        void Printing();
        
        string strip(string s);

        static bool is_number(const string& s);

        static bool Comparator(PrintCommand a, PrintCommand b);

        static void print_command_debug(PrintCommand pc);
        
};

