#ifndef MAIN_HPP
#define MAIN_HPP

#include<climits>
#include<iostream>
#include<fstream>
#include<bitset>
#include<vector>
#include<string>
#include<iomanip>
#include<map>
#include<queue>
#include<sstream>
#include<boost/algorithm/string.hpp>
#include<boost/format.hpp> 

#include "Constants.hpp"
#define CLOCK_CHECK           SIMULATION_TIME * 10 // To avoid writing at same clock cycle

#define ff              first
#define pb              push_back
#define ss              second
#define mp              make_pair
#define pii             pair<int,int>
#define printp(x)       cout << "(" <<  x.ff << "," << x.ss << ") "

#define d0(x) cout<<(x)<<" "
#define d1(x) cout<<(x)<<endl
#define d2(x,y) cout<<(x)<<" "<<(y)<<endl
#define d3(x,y,z) cout<<(x)<<" "<<(y)<<" "<<(z)<<endl
#define d4(a,b,c,d) cout<<(a)<<" "<<(b)<<" "<<(c)<<" "<<(d)<<endl
#define d5(a,b,c,d,e) cout<<(a)<<" "<<(b)<<" "<<(c)<<" "<<(d)<<" "<<(e)<<endl
#define d6(a,b,c,d,e,f) cout<<(a)<<" "<<(b)<<" "<<(c)<<" "<<(d)<<" "<<(e)<<" "<<(f)<<endl


using namespace std;

map<int,pair<int,int> > MemoryUpdation, MemoryUse;
vector<vector<int> > Dram;
int UniversalIssueTime[UNIVERSAL_ISSUE_TIME];

int MRM_Delay = 0;
int InstructionCount = 0; // Throughput

int RowDelay           = 10;
int ColumnDelay        = 2;
int TotalMemory_4Bytes = 1048576;
int RowMemory          = 1024;
int ColumnMemory       = 1024;
int TotalMemory        = TotalMemory_4Bytes/4;
int NumberOfCycles     = 0;
int StartTime          = -1;

int UniversalEndTime   = -1;
int UniversalInstructionRead = 0;

int MemoryAvailable;
int RowBuffer          = -1;
int RowBufferUpdates   = 0;



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
    string File = "";
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
    int cost = 0;
    bool issued = false;
    int FileIndex = -1;
    int Endtime = -1; //  For lw/sw instruction
    bool active = true;
    bool done = false;

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
        int i = 0;

        string print_msg = "";
        map<string,int> reg, label;
        map<string,pair<int,int> > RegisterUpdation, RegisterUse;
        map<string,int> InstructionCountVector, regEndTime;
        vector<instruction> instr;
        vector<instruction> QueueInstruction;
        vector<PrintCommand> Command;

        vector<instruction> QueueInstruction_new;
        int ClockCount[CLOCK_CHECK];

        Registers(){for(int i=0;i<CLOCK_CHECK;i++)ClockCount[i]=0;}
        ~Registers(){}

        void std_registers();
        
        void print_reg();
        static void print_ins(instruction ins);
        void print_map(map<string,int> m);
        void print_command(PrintCommand pc);
        
        void tokenize(string s, int jFileIndex);
        bool AllotMemory();
        
        void parse();
        instruction parse_new();

        bool ParsingExecution(string File);
        void ResolveDependency(instruction &ins);

        void CheckDependency_add(instruction &ins, int index, bool Update); 
        // <-  True implies it is dependent
        // <-  False implies it is independent

        void CheckDependency_beq(instruction &ins, int index, bool Update);
        void CheckDependency_j(instruction &ins, int index, bool Update);
        void CheckDependency_lw(instruction &ins, int index, bool Update);
        void CheckDependency_addi(instruction &ins, int index, bool Update);

        bool CheckOverallDependency(instruction &ins, int index, bool Update);

        void Execute_1(int j, instruction ins);

        void DramRequestIssued(instruction &ins);

        void DeleteInstruction(instruction &ins);

        string InstructionExecution(instruction &ins);

        void Printing();
        
        string strip(string s);

        static bool is_number(const string& s);

        static bool Comparator(PrintCommand a, PrintCommand b);

        static void print_command_debug(PrintCommand pc);

        static bool EqualityInstruction(instruction a, instruction b);
        
};

#endif