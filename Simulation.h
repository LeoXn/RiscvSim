#include<iostream>
#include<stdio.h>
#include<math.h>
#include<io.h>
#include<process.h>
#include<time.h>
#include<stdlib.h>
#include<stdint.h>

//R-Type:0x33
#define OP_R 0x33
#define F3_ADD 0x0
#define F7_ADD 0x00

#define F3_MUL 0x0
#define F7_MUL 0x01

#define F3_SUB 0x0
#define F7_SUB 0x20

#define F3_SLL 0x1
#define F7_SLL 0x00

#define F3_MULH 0x1
#define F7_MULH 0x01

#define F3_SLT 0x2
#define F7_SLT 0x00

#define F3_XOR 0x4
#define F7_XOR 0x00

#define F3_DIV 0x4
#define F7_DIV 0x01

#define F3_SRL 0x5
#define F7_SRL 0x00

#define F3_SRA 0x5
#define F7_SRA 0x20

#define F3_OR 0x6
#define F7_OR 0x00

#define F3_REM 0x6
#define F7_REM 0x01

#define F3_AND 0x7
#define F7_AND 0x00

//R-Type:0x3B
#define OP_RW 0x3B
#define F3_ADDW 0x0
#define F7_ADDW 0x0

#define F3_SUBW 0x0
#define F7_SUBW 0x20

#define F3_MULW 0x0
#define F7_MULW 0x01

#define F3_DIVW 0x4
#define F7_DIVW 0x01

#define F3_REMW 0x6
#define F7_REMW 0x01

//I-Type:0x03
#define OP_LW 0x03
#define F3_LB 0x0
#define F3_LH 0x1
#define F3_LW 0x2
#define F3_LD 0x3

//I-Type:0x13
#define OP_I 0x13
#define F3_ADDI 0x0
#define F3_SLLI 0x1
#define F7_SLLI 0x00

#define F3_SLTI 0x2
#define F3_XORI 0x4
#define F3_SRLI 0x5
#define F7_SRLI 0x00

#define F3_SRAI 0x5
#define F7_SRAI 0x10

#define F3_ORI 0x6
#define F3_ANDI 0x7


//I-Type:0x1B
#define OP_IW 0x1B
#define F3_ADDIW 0x0

//I-Type:0x67
#define OP_JALR 0x67
#define F3_JALR 0x0

//I-Type:0x73
#define OP_SCALL 0x73
#define F3_SCALL 0
#define F7_SCALL 0

//S-Type:0x23
#define OP_SW 0x23
#define F3_SB 0x0
#define F3_SH 0x1
#define F3_SW 0x2
#define F3_SD 0x3

//SB-Type:0x63
#define OP_BEQ 0x63
#define F3_BEQ 0x0
#define F3_BNE 0x1
#define F3_BLT 0x4
#define F3_BGE 0x5

//U-Type:
#define OP_AUIPC 0x17
#define OP_LUI 0x37

//UJ-Type:0x6f
#define OP_JAL 0x6f


#define MAX 100000000

typedef unsigned long long REG;

//主存
uint32_t memory[MAX]={0};
//寄存器堆
REG reg[32]={0};
//PC
int PC=0;

//指令寄存器
uint32_t inst=0;

//各个指令解析段
uint32_t OP=0;
uint32_t fuc3=0,fuc7=0;
int shamt=0;
int rs1=0,rs2=0,rd=0;
uint32_t imm12=0;
uint32_t imm20=0;
uint32_t imm7=0;
uint32_t imm5=0;

//加载内存
void load_memory();

//译码
void translate_inst();

//执行
void execute_inst();

//符号扩展
int64_t ext_signed(uint32_t src,int bit);

//获取指定位
uint32_t getbit(uint32_t src,int s,int e);

uint32_t getbit(uint32_t src,int s,int e)
{
    uint32_t mask = 0;
    for(int i=s;i<=e;i++){
        mask |= (1<<i);
    }
	return (src & mask)>>s;
}

int64_t ext_signed(uint32_t src,int bit)
{
    return ((signed)src<<bit)>>bit;
}