#include <fstream>
#include "pin.H"
#include <stdio.h>
#include "string.h"

using std::string;
using std::ofstream;

// gives us the results output option
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o","results.out", "specify output file"); 

FILE* outfile;




// Print a memory read record
VOID RecordMemRead(VOID* ip, VOID* addr, UINT32 size) { 
    fprintf(outfile, "MR,%p,%p,%u\n", ip, addr,size); 
}

// Print a memory write record
VOID RecordMemWrite(VOID* ip, VOID* addr, UINT32 size) { 
    fprintf(outfile, "MW,%p,%p,%u\n", ip, addr,size); 
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID* v)
{
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead, 
                    IARG_INST_PTR, IARG_MEMORYOP_EA,  memOp,
                    IARG_UINT32, INS_MemoryOperandSize(ins,memOp),IARG_END);
        }
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite, 
                    IARG_INST_PTR, IARG_MEMORYOP_EA,  memOp,
                    IARG_UINT32, INS_MemoryOperandSize(ins,memOp),IARG_END);
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    //fprintf(outfile,)
    fclose(outfile);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This Pintool prints a trace of memory addresses\n" + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return Usage();
    
    outfile= fopen(KnobOutputFile.Value().c_str(),"w");
            
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
