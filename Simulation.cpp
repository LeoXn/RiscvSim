#include "Simulation.h"
using namespace std;

extern void read_elf(char *elf_path);
extern uint32_t cadr;
extern uint32_t csize;
extern uint32_t vadr;
extern uint64_t gp;
extern uint32_t madr;
extern uint32_t endPC;
extern uint32_t entry;
extern FILE *file;


//指令运行数
int64_t inst_num=0;

//系统调用退出指示
int exit_flag=0;

//单步调试指示
int step_flag=0;

int main(int argc,char *argv[])
{
	if(argc<2){
		printf("Not enough args!\n");
		return 1;
	} 
    if(argc>=3){
        printf("argc=%d\n",argc);
        if(strcmp(argv[2],"-s")==0){
            step_flag=1;
        }
        else
        {
            printf("Invalid Instruction!:%s\n",argv[2]);
		    return 1;
        }
        
    }
    ///test
    printf("Start...\n");


	//解析elf文件
	read_elf(argv[1]);
	
	//加载内存
	load_memory();

	//设置入口地址
	// PC=entry>>2;
    // PC=madr>>2;
    PC=madr;
	
	//设置全局数据段地址寄存器
	reg[3]=gp;
	reg[2]=MAX/2;//栈基址 （sp寄存器）
	
	//结束PC的设置
	int end=(int)endPC/4-1;

    //查看调试信息
    printf("entry=%d\nPC=%d\nendPC=%d\nend=%d\n",entry,PC,endPC,end);

	while((PC>>2)!=end)
    // while(PC<end)
	{
	    translate_inst();
	    
        execute_inst();

        //单步调试
        if(step_flag==1) {
			char c;
			while(cin >> c){
				if (c=='n')
					break;
				else if (c=='e'){
					exit_flag = 1;
					break;
				}
				else if (c=='r'){
					for (int i = 0;i < 32;i++){
						printf("reg[%02d] = 0x%08x\n", i, reg[i] );
					}
				}
				else if (c=='m'){
					printf("Please Enter Address and Number of Word:\n 0x");
					long long adr;
                    int NumOfWord;
					cin >>hex>> adr;
                    scanf("%d",&NumOfWord);
                    while(NumOfWord--)
                    {
                        printf("0x%08x ",(adr >> 2)<<2 );
                        printf("%02x ", getbit(memory[adr >> 2],0,8));
                        printf("%02x ", getbit(memory[adr >> 2],8,14));
                        printf("%02x ", getbit(memory[adr >> 2],15,23));
                        printf("%02x\n", getbit(memory[adr >> 2],24,31));
                        adr+=4;
                    }

				}
    			else {
    				printf("Invalid Instruction! Please Retry:\n");
    			}
				
			}
		}

        if(exit_flag==1)
            break;

        reg[0]=0;//一直为零
	}
    printf("Instruct Num: %ld\n",inst_num);

    ///test
    printf("End..\n");
    
    //查看结果
    printf("Result:");
    for(uint64_t adr=reg[2]-80;adr<reg[2];adr+=4){
        printf(" %d",memory[adr>>2]);
    } 
    printf("\n");

	return 0;
}

//加载代码段
//初始化PC
void load_memory()
{
	fseek(file,cadr,SEEK_SET);
	fread(&memory[vadr>>2],1,csize,file);

	vadr=vadr>>2;
	csize=csize>>2;
	fclose(file);
}


void translate_inst()
{
	inst=memory[PC>>2];
	
    OP=getbit(inst,0,6);
    fuc3=getbit(inst,12,14);
    fuc7=getbit(inst,25,31);
	rd=getbit(inst,7,11);
    rs1=getbit(inst,15,19);
    rs2=getbit(inst,20,24);

    imm12=getbit(inst,20,31);
    imm20=getbit(inst,12,31);
    imm7=getbit(inst,25,31);
    imm5=getbit(inst,7,11);

	// printf("0x%08x: OP:0x%02x fuc3:%x fuc7:0x%2x\n",PC,OP,fuc3,fuc7);
	inst_num++;
	PC+=4;
}


void execute_inst()
{
	if(OP==OP_R)
	{
		if(fuc3==F3_ADD&&fuc7==F7_ADD)
		{
            reg[rd]=reg[rs1]+reg[rs2];

            printf("add x%d,x%d,x%d    #reg[%d]=%x\n",rd,rs1,rs2,rd,reg[rd]);
		}
		else if(fuc3==F3_MUL&&fuc7==F7_MUL)
        {
            reg[rd]=reg[rs1]*reg[rs2];
        }
		else if(fuc3==F3_SUB&&fuc7==F7_SUB)
		{
            reg[rd]=reg[rs1]-reg[rs2];
		}
		else if(fuc3==F3_SLL&&fuc7==F7_SLL)
		{
            reg[rd]=reg[rs1]<<reg[rs2];
		}
		else if(fuc3==F3_MULH&&fuc7==F7_MULH)
		{
            uint64_t AH=reg[rs1]>>32;
            uint64_t AL=(uint32_t)reg[rs1];
            uint64_t BH=reg[rs2]>>32;
            uint64_t BL=(uint32_t)reg[rs2];

            uint64_t AHBH=AH*BH;
            uint64_t AHBL=AH*BL;
            uint64_t ALBH=AL*BH;
            uint64_t ALBL=AL*BL;

            uint64_t carry=((uint64_t)(uint32_t)AHBL + (uint64_t)(uint32_t)ALBH + (ALBL>>32)) >> 32;
            reg[rd]=AHBH + (AHBL>>32) + (ALBH>>32) + carry;
		}
		else if(fuc3==F3_SLT&&fuc7==F7_SLT)
		{
            reg[rd]=reg[rs1]<reg[rs2] ? 1 : 0;
		}
		else if(fuc3==F3_XOR&&fuc7==F7_XOR)
		{
            reg[rd]=reg[rs1]^reg[rs2];
		}
		else if(fuc3==F3_DIV&&fuc7==F7_DIV)
		{
            reg[rd]=reg[rs1]/reg[rs2];
		}
		else if(fuc3==F3_SRL&&fuc7==F7_SRL)
		{
            reg[rd]=(unsigned)reg[rs1]>>reg[rs2];
		}
		else if(fuc3==F3_SRA&&fuc7==F7_SRA)
		{
            reg[rd]=reg[rs1]>>reg[rs2];
		}
		else if(fuc3==F3_OR&&fuc7==F7_OR)
		{
            reg[rd]=reg[rs1]|reg[rs2];
		}
		else if(fuc3==F3_REM&&fuc7==F7_REM)
		{
            reg[rd]=reg[rs1]%reg[rs2];
		}
		else if(fuc3==F3_AND&&fuc7==F7_AND)
		{
            reg[rd]=reg[rs1]&reg[rs2];
		}
	}
    else if(OP==OP_RW)
    {
        if(fuc3==F3_ADDW&&fuc7==F7_ADDW)
        {
			reg[rd]=(int64_t)((int)reg[rs1] + (int)reg[rs2]);

            printf("addw x%d,x%d,x%d    #reg[%d]=%x\n",rd,rs1,rs2,rd,reg[rd]);
        }
        else if(fuc3==F3_SUBW&&fuc7==F7_SUBW)
        {
			reg[rd]=(int64_t)((int)reg[rs1] - (int)reg[rs2]);
        }
        else if(fuc3==F3_MULW&&fuc7==F7_MULW)
        {
			reg[rd]=(int64_t)((int)reg[rs1] * (int)reg[rs2]);

            printf("mulw x%d,x%d,x%d    #reg[%d]=%x\n",rd,rs1,rs2,rd,reg[rd]);
        }
        else if(fuc3==F3_DIVW&&fuc7==F7_DIVW)
        {
			reg[rd]=(int64_t)((int)reg[rs1] / (int)reg[rs2]);
        }
        else if(fuc3==F3_REMW&&fuc7==F7_REMW)
        {
			reg[rd]=(int64_t)((int)reg[rs1] % (int)reg[rs2]);
        }
    }
	else if(OP==OP_LW)
    {
        int64_t addr=reg[rs1]+ext_signed(imm12,20);
        if(addr<0)//MemAddrErr
        {
            printf("MemAddr Error!\n");
            return;
        }
        uint8_t pos;
        if(fuc3==F3_LB)
        {
            reg[rd]=*(char*)((uint8_t*)memory + addr);
        }
        else if(fuc3==F3_LH)
        {
            reg[rd]=*(short*)((uint8_t*)memory + addr);
        }
        else if(fuc3==F3_LW)
        {
            reg[rd]=*(int*)((uint8_t*)memory + addr);

            printf("lw  x%d,%d(x%d)      #reg[%d]=%x\n",rd,ext_signed(imm12,20),rs1,rd,reg[rd]);
        }
        else if(fuc3==F3_LD)
        {
            reg[rd]=*(int64_t*)((uint8_t*)memory + addr);

            printf("ld  x%d,%d(x%d)      #reg[%d]=%016llx\n",rd,ext_signed(imm12,20),rs1,rd,reg[rd]);
        }
        
    }
    else if(OP==OP_I)
    {
        int64_t imm=ext_signed(imm12,20);
        shamt=getbit(inst,20,25);
        fuc7=getbit(inst,26,31);
        if(fuc3==F3_ADDI)
        {
            reg[rd]=reg[rs1]+imm;
            
            printf("addi x%d,x%d,%d    #reg[%d]=%x\n",rd,rs1,imm,rd,reg[rd]);
         }
        else if(fuc3==F3_SLLI&&fuc7==F7_SLLI)
        {
            reg[rd]=reg[rs1]<<shamt;

            printf("slli x%d,x%d,%d    #reg[%d]=%x\n",rd,rs1,shamt,rd,reg[rd]);
        }
        else if(fuc3==F3_SLTI)
        {
            reg[rd]=(signed)reg[rs1]<imm ? 1 : 0;
        }
        else if(fuc3==F3_XORI)
        {
            reg[rd]=reg[rs1]^imm;
        }
        else if(fuc3==F3_SRLI&&fuc7==F7_SRLI)
        {
            reg[rd]=(unsigned)reg[rs1]>>shamt;
        }
        else if(fuc3==F3_SRAI&&fuc7==F7_SRAI)
        {
            reg[rd]=reg[rs1]>>shamt;
        }
        else if(fuc3==F3_ORI)
        {
            reg[rd]=reg[rs1]|imm;
        }
        else if(fuc3==F3_ANDI)
        {
            reg[rd]=reg[rs1]&imm;
        }
        
    }
    else if(OP==OP_IW)
    {
        int32_t imm=ext_signed(imm12,20);
        // shamt=getbit(inst,20,24);
        if(fuc3==F3_ADDIW)
        {
			reg[rd]=(int64_t)(getbit(reg[rs1],0,31) + imm);

            printf("addiw x%d,x%d,%d    #reg[%d]=%x\n",rd,rs1,imm,rd,reg[rd]);
        }
        else
        {
			
        }
    }
    else if(OP==OP_SW)
    {
        imm12=(imm7<<5)|imm5;
        int64_t addr=reg[rs1]+ext_signed(imm12,20);
        if(addr<0)//MemAddrErr
        {
            printf("MemAddr Error!\n");
            return;
        }
        uint8_t* pos;
        if(fuc3==F3_SB)
        {
            pos=(uint8_t*)memory + addr;
            *pos=(char)reg[rs2];
        }
        else if(fuc3==F3_SH)
        {
            pos=(uint8_t*)memory + addr;
            *(short*)pos=(short)reg[rs2];
        }
        else if(fuc3==F3_SW)
        {
            pos=(uint8_t*)memory + addr;
            *(int*)pos=(int)reg[rs2];

            printf("sw x%d,%d(x%d)      #memory[%d]=%x\n",rs2,imm12,rs1,addr,reg[rs2]);
        }
        else if(fuc3==F3_SD)
        {
            pos=(uint8_t*)memory + addr;
            *(int64_t*)pos=reg[rs2];
            
            printf("sd x%d,%d(x%d)      #memory[%d]=%x memory[%d]=%x\n",rs2,imm12,rs1,addr,reg[rs2],addr+4,reg[rs2]>>32);
        }
    }
    else if(OP==OP_BEQ)
    {
        imm12=(getbit(inst,31,31)<<12)|(getbit(inst,7,7)<<11)|(getbit(inst,25,30)<<5)|(getbit(inst,8,11)<<1);
        int64_t imm=ext_signed(imm12,20);
        if(fuc3==F3_BEQ)
        {
			if(reg[rs1]==reg[rs2]){
                PC=PC-4+imm;
            }
        }
        else if(fuc3==F3_BNE)
        {
           if(reg[rs1]!=reg[rs2]){
                PC=PC-4+imm;
            }
        }
        else if(fuc3==F3_BLT)
        {
            if((signed)reg[rs1]<(signed)reg[rs2]){
                PC=PC-4+imm;
            }
        }
        else if(fuc3==F3_BGE)
        {
            if((signed)reg[rs1]>=(signed)reg[rs2]){
                PC=PC-4+imm;
            }

            printf("bge x%d,x%d,%d     #PC=%x\n",rs1,rs2,ext_signed(imm12,20),PC);
        }
    }
    else if(OP==OP_AUIPC)
    {
        reg[rd]=(PC-4)+(imm20<<12);
    }
    else if(OP==OP_LUI)
    {
        reg[rd]=imm20<<12;

        printf("lui x%d,%d      #reg[%d]=%x\n",rd,imm20,rd,reg[rd]);
    }
    else if(OP==OP_JAL)
    {
        imm20=(getbit(inst,31,31)<<20)|(getbit(inst,12,19)<<12)|(getbit(inst,20,20)<<11)|(getbit(inst,21,30)<<1);
        reg[rd]=PC;
        PC=PC-4+ext_signed(imm20,12);

        printf("jal x%d,%d          #PC=%x\n",rd,imm20,PC);
    }
    else if(OP==OP_JALR)
    {
        if(fuc3==F3_JALR)
        {
            reg[rd]=PC;
            PC=reg[rs1]+ext_signed(imm12,20);
            PC=PC&0xfffffffe;

            printf("jalr x%d,x%d,%d      #PC=%x\n",rd,rs1,ext_signed(imm12,20),PC);
        }
    }
    else if(OP==OP_SCALL)//系统调用指令
    {
        if(fuc3==F3_SCALL&&fuc7==F7_SCALL)
	    {
			if(reg[17]==64)////printf
			{
				int place=0,c=0;
				const void * t=&memory[reg[11]>>2];
				reg[10]=write((unsigned int)reg[10],t,(unsigned int)reg[12]);
			}
			else if(reg[17]==63)//scanf
			{

			}
			else if(reg[17]==169)//time
			{

			}
			else if(reg[17]==93)//exit
			{
				exit_flag=1;
			}
			else
			{

			}
        }
        else
        {
			
        }
    }
    else
    {

    }
}