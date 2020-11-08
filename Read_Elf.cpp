#include"Read_Elf.h"

FILE *elf=NULL;
Elf64_Ehdr elf64_hdr;

//Program headers
unsigned int padr=0;
unsigned int psize=0;
unsigned int pnum=0;

//Section Headers
unsigned int sadr=0;
unsigned int ssize=0;
unsigned int snum=0;

//Symbol table
unsigned int symnum=0;
unsigned int symadr=0;
unsigned int symsize=0;
unsigned int symlocn=0;

//用于指示 包含节名称的字符串是第几个节（从零开始计数）
unsigned int _index=0;

//字符串表在文件中地址，其内容包括.symtab和.debug节中的符号表
uint64_t stradr=0;


bool open_file(char *elf_path)
{
	if((file=fopen(elf_path,"rb"))==NULL) 
		return false;
	return true;
}

void read_elf(char *elf_path)
{
	if(!open_file(elf_path))
		return ;

	elf=fopen("elf_info","w");

	fprintf(elf,"ELF Header:\n");
	read_Elf_header();

	fprintf(elf,"\n\nSection Headers:\n");
	read_elf_sections();

	fprintf(elf,"\n\nProgram Headers:\n");
	read_Phdr();

	fprintf(elf,"\n\nSymbol table:\n");
	read_symtable();

	fclose(elf);
}

void read_Elf_header()
{
	//file should be relocated
	fread(&elf64_hdr,1,sizeof(elf64_hdr),file);
		
	fprintf(elf," magic: 	");
	for(int i=0;i<16;i++){
		fprintf(elf,"%02x ",elf64_hdr.e_ident[i]);
	}
	fprintf(elf,"\n");

	//Class
	fprintf(elf," Class: 							");
	switch (elf64_hdr.e_ident[4])
	{
	case 0:
		fprintf(elf,"ELFCLASSNONE\n");
		break;
	case 1:
		fprintf(elf,"ELFCLASS32\n");
		break;
	case 2:
		fprintf(elf,"ELFCLASS64\n");
		break;
	default:
		break;
	}
	
	//Data
	fprintf(elf," Data:  							");
	switch (elf64_hdr.e_ident[5])
	{
	case 0:
		fprintf(elf,"ELFDATANONE\n");
		break;
	case 1:
		fprintf(elf,"2's complement, little endian\n");
		break;
	case 2:
		fprintf(elf,"2's complement, big endian\n");
		break;
	default:
		break;
	}
		
	// Version
	fprintf(elf," Version:							%d (current)\n",elf64_hdr.e_version);

	fprintf(elf," OS/ABI: 							UNIX - System V\n");
	
	fprintf(elf," ABI Version:						0\n");
	
	//Type
	fprintf(elf," Type:  							");
	switch (elf64_hdr.e_type)
	{
	case 0:
		fprintf(elf,"NONE (No file type)\n");
		break;
	case 1:
		fprintf(elf,"REL (Relocatable file)\n");
		break;
	case 2:
		fprintf(elf,"EXEC (Executable file)\n");
		break;
	case 3:
		fprintf(elf,"DYN (Shared object file)\n");
		break;
	case 4:
		fprintf(elf,"CORE (Core file)\n");
		break;
	case 0xfe00:
		fprintf(elf,"LOOS (Operating system-specific)\n");
		break;
	case 0xfeff:
		fprintf(elf,"HIOS (Operating system-specific)\n");
		break;
	case 0xff00:
		fprintf(elf,"LOPROC (Processor-specific)\n");
		break;
	case 0xffff:
		fprintf(elf,"HIPROC (Processor-specific)\n");
		break;
	default:
		break;
	}
	
	//Machine
	fprintf(elf," Machine:							");
	switch (elf64_hdr.e_machine)
	{
	case 0x3e:
		fprintf(elf,"Advanced Micro Devices X86-64\n");
		break;
	case 0xf3:
		fprintf(elf,"RISC-V\n");
		break;
	default:
		fprintf(elf,"\n");
		break;
	}

	fprintf(elf," Version:							0x%x\n",elf64_hdr.e_version);

	fprintf(elf," Entry point address:				0x%x\n",elf64_hdr.e_entry);
	entry=elf64_hdr.e_entry;
	 
	fprintf(elf," Start of program headers:			%d (bytes into  file)\n",elf64_hdr.e_phoff);
	padr=elf64_hdr.e_phoff;

	fprintf(elf," Start of section headers:			%d (bytes into  file)\n",elf64_hdr.e_shoff);
	sadr=elf64_hdr.e_shoff;

	fprintf(elf," Flags: 							0x%x\n",elf64_hdr.e_flags);

	fprintf(elf," Size of this header:				%d (bytes)\n",elf64_hdr.e_ehsize);

	fprintf(elf," Size of program headers:			%d (bytes)\n",elf64_hdr.e_phentsize);
	psize=elf64_hdr.e_phentsize;

	fprintf(elf," Number of program headers: 		%d\n",elf64_hdr.e_phnum);
	pnum=elf64_hdr.e_phnum;

	fprintf(elf," Size of section headers:			%d (bytes)\n",elf64_hdr.e_shentsize);
	ssize=elf64_hdr.e_shentsize;

	fprintf(elf," Number of section headers: 		%d\n",elf64_hdr.e_shnum);
	snum=elf64_hdr.e_shnum;

	fprintf(elf," Section header string table index: %d\n",elf64_hdr.e_shstrndx);
	_index=elf64_hdr.e_shstrndx;

}

void read_elf_sections()
{

	Elf64_Shdr elf64_shdr;

	//读取节名称的节
	fseek(file,sadr + (_index*ssize),SEEK_SET);
	fread(&elf64_shdr,1,sizeof(elf64_shdr),file);
	//读取节名表
	fseek(file,elf64_shdr.sh_offset,SEEK_SET);
	char *buf = (char *)malloc(elf64_shdr.sh_size);
	fread(buf,1,elf64_shdr.sh_size,file);

	char name[20]={0};
	int cnt=0;

	fseek(file,sadr,SEEK_SET);
	// snum=0;
	if(snum){
		fprintf(elf," [Nr]	Name				Type				Address				Offset\n"
					"		Size				Entsize				Flags  Link   Info   Align\n");
	} 
	for(int c=0;c<snum;c++)
	{
		fprintf(elf," [%3d]	",c);
		
		//file should be relocated
		fread(&elf64_shdr,1,sizeof(elf64_shdr),file);

		//Get Name
		uint32_t str = elf64_shdr.sh_name;
		while(buf[str]!='\0'){
			name[cnt++]=buf[str++];
		}
		name[cnt]='\0';
		cnt=0;

		// symtab and strtab
		if(strcmp(name,".symtab")==0){
			symadr=elf64_shdr.sh_offset;
			symsize=elf64_shdr.sh_size;
			symnum=symsize/elf64_shdr.sh_entsize;
			// symlocn=elf64_shdr.sh_info/elf64_shdr.sh_entsize;	
		}
		if(strcmp(name,".strtab")==0){
			stradr=elf64_shdr.sh_offset;
		}

		//Name
		fprintf(elf,"%-18s	",name);

		//Type
		switch (elf64_shdr.sh_type)
		{
		case 0:
			fprintf(elf,"NULL		");
			break;
		case 1:
			fprintf(elf,"PROGBITS	");
			break;
		case 2:
			fprintf(elf,"SYMTAB		");
			break;
		case 3:
			fprintf(elf,"STRTAB		");
			break;
		case 4:
			fprintf(elf,"RELA		");
			break;
		case 5:
			fprintf(elf,"HASH		");
			break;
		case 6:
			fprintf(elf,"DYNAMIC 	 ");
			break;
		case 7:
			fprintf(elf,"NOTE		");
			break;
		case 8:
			fprintf(elf,"NOBITS		");
			break;
		case 9:
			fprintf(elf,"REL		");
			break;
		case 10:
			fprintf(elf,"SHLIB		");
			break;
		case 11:
			fprintf(elf,"DYNSYM		");
			break;
		case 14:
			fprintf(elf,"INIT_ARRAY	");
			break;
		case 15:
			fprintf(elf,"FINI_ARRAY	");
			break;
		case 16:
			fprintf(elf,"PREINIT_ARRAY");
			break;
		case 17:
			fprintf(elf,"GROUP		");
			break;
		case 18:
			fprintf(elf,"SYMTAB_SHNDX ");
			break;
		case 0x60000000:
			fprintf(elf,"LOOS		");
			break;
		case 0x6fffffff:
			fprintf(elf,"HIOS		");
			break;
		case 0x70000000:
			fprintf(elf,"LOPROC		");
			break;
		case 0x7fffffff:
			fprintf(elf,"LOPROC		");
			break;
		case 0x80000000:
			fprintf(elf,"LOUSER		");
			break;
		case 0x8fffffff:
			fprintf(elf,"HIUSER		");
			break;
		default:
			fprintf(elf,"%08x	",elf64_shdr.sh_type);
			break;
		}
		fprintf(elf,"		");

		//Adderss
		fprintf(elf,"%016x	",elf64_shdr.sh_addr);

		//Offset
		fprintf(elf,"%016x 	\n",elf64_shdr.sh_offset);

		//Size
		fprintf(elf,"		%016x	",elf64_shdr.sh_size);

		//Entsize
		fprintf(elf,"%016x	",elf64_shdr.sh_entsize);

		//Flags   
		fprintf(elf,"%x		",elf64_shdr.sh_flags);
		
		//Link
		fprintf(elf,"%d		",elf64_shdr.sh_link);

		//Info  
		fprintf(elf,"%d		",elf64_shdr.sh_info);

		//Align 
		fprintf(elf,"%d	\n",elf64_shdr.sh_addralign);

 	}
	free(buf);
}

void read_Phdr()
{
	Elf64_Phdr elf64_phdr;
	fseek(file,padr,SEEK_SET);
	// pnum=0;
	if(pnum){
		fprintf(elf," [Nr]	Type			Offset				VirtAddr			PhysAddr\n"
					"						FileSiz				MemSiz				Flags	Align\n");
	} 
	bool first_addr=true;
	for(int c=0;c<pnum;c++)
	{
		fprintf(elf," [%3d]	",c);
			
		//file should be relocated
		fread(&elf64_phdr,1,sizeof(elf64_phdr),file);

		//Type
		switch (elf64_phdr.p_type)
		{
		case 0:
			fprintf(elf,"NULL		");
			break;
		case 1:
			fprintf(elf,"LOAD		");
			//首址
			if(first_addr){
				cadr=elf64_phdr.p_offset;
				vadr=elf64_phdr.p_vaddr;
				first_addr=false;
			}
			csize = elf64_phdr.p_offset - cadr + elf64_phdr.p_filesz;
			break;
		case 2:
			fprintf(elf,"DYNAMIC 	");
			break;
		case 3:
			fprintf(elf,"INTERP		");
			break;
		case 4:
			fprintf(elf,"NOTE		");
			break;
		case 5:
			fprintf(elf,"SHLIB		");
			break;
		case 6:
			fprintf(elf,"PHDR		");
			break;
		case 7:
			fprintf(elf,"TLS 		");
			break;
		case 0x6474e550:
			fprintf(elf,"GNU_EH_FRAME");
			break;
		case 0x6474e551:
			fprintf(elf,"GNU_STACK	 ");
			break;
		case 0x6474e552:
			fprintf(elf,"GNU_RELRO	");
			break;
		default:
			fprintf(elf,"%x		",elf64_phdr.p_type);
			break;
		}
		fprintf(elf,"	");

		//Offset   
		fprintf(elf,"0x%016x	",elf64_phdr.p_offset);
		
		//VirtAddr
		fprintf(elf,"0x%016x	",elf64_phdr.p_vaddr);
		
		//PhysAddr
		fprintf(elf,"0x%016x\n",elf64_phdr.p_paddr);
		fprintf(elf,"						");
		//FileSiz 
		fprintf(elf,"0x%016x	",elf64_phdr.p_filesz);

		//MemSiz
		fprintf(elf,"0x%016x	",elf64_phdr.p_memsz);
		
		//Flags  
		fprintf(elf,"%d		",elf64_phdr.p_flags);
		
		//Align
		fprintf(elf,"0x%x\n",elf64_phdr.p_align);
	}
}

void read_symtable()
{
	Elf64_Sym elf64_sym;

	char name[40]={0};
	int cnt=0;
	
	fseek(file,symadr,SEEK_SET);
	// symnum=0;
	if(symnum){
		fprintf(elf," Num		Value			Size	Type	Bind	Vis		 Ndx	Name\n");
	}

	for(int c=0;c<symnum;c++)
	{
		fprintf(elf," [%3d]   ",c);
		
		fseek(file, symadr + c * sizeof(elf64_sym), SEEK_SET);

		//file should be relocated
		fread(&elf64_sym,1,sizeof(elf64_sym),file);

		//Value
		fprintf(elf,"%016x	",elf64_sym.st_value);

		//Size
		fprintf(elf,"%4d	",elf64_sym.st_size);
		
		//Type
		unsigned int type = elf64_sym.st_info&0xf;
		switch (type)
		{
		case 0:
			fprintf(elf,"NOTYPE");
			break;
		case 1:
			fprintf(elf,"OBJECT");
			break;
		case 2:
			fprintf(elf,"FUNC");
			break;
		case 3:
			fprintf(elf,"SECTION");
			break;
		case 4:
			fprintf(elf,"FILE");
			break;
		case 5:
			fprintf(elf,"COMMON");
			break;
		case 6:
			fprintf(elf,"TLS ");
			break;
		case 10:
			fprintf(elf,"LOOS");
			break;
		case 12:
			fprintf(elf,"HIOS");
			break;
		case 13:
			fprintf(elf,"LOPROC");
			break;
		case 15:
			fprintf(elf,"HIPROC");
			break;
		default:
			fprintf(elf,"	");
			break;
		}
		fprintf(elf,"	");

		//Bind
		unsigned int bind = elf64_sym.st_info>>4;
		switch (bind)
		{
		case 0:
			fprintf(elf,"LOCAL");
			break;
		case 1:
			fprintf(elf,"GLOBAL");
			break;
		case 2:
			fprintf(elf,"WEAK");
			break;
		case 10:
			fprintf(elf,"LOOS");
			break;
		case 12:
			fprintf(elf,"HIOS");
			break;
		case 13:
			fprintf(elf,"LOPROC");
			break;
		case 15:
			fprintf(elf,"HIPROC");
			break;
		default:
			fprintf(elf,"	");
			break;
		}
		fprintf(elf,"	");

		//Vis
		unsigned int vis = elf64_sym.st_other&0x3;
		switch (vis)
		{
		case 0:
			fprintf(elf,"DEFAULT");
			break;
		case 1:
			fprintf(elf,"INTERNAL");
			break;
		case 2:
			fprintf(elf,"HIDDEN");
			break;
		case 3:
			fprintf(elf,"PROTECTED");
			break;
		default:
			fprintf(elf,"	");
			break;
		}
		fprintf(elf,"	");

		//Ndx
		switch (type)
		{
		case 0:
			fprintf(elf," UND	");
			break;
		case 4:
			fprintf(elf," ABS	");
			break;
		default:
			fprintf(elf,"%4d	",elf64_sym.st_shndx);
			break;
		}
		
		//Name
		uint64_t str = stradr + elf64_sym.st_name;
		fseek(file,str,SEEK_SET);
		fread(&name[cnt],1,1,file);

		while (name[cnt]!='\0')
		{
			cnt++;
			fread(&name[cnt],1,1,file);
		}
		cnt=0;

		fprintf(elf,"%-40s   \n",name);

		if(strcmp(name,"__global_pointer$")==0){
			gp=elf64_sym.st_value;
		}
		else if(strcmp(name,"main")==0){
			madr=elf64_sym.st_value;
			endPC=madr+elf64_sym.st_size;
		}

	}
	fflush(elf);
}


// int main(int agrc,char *argv[])
// {
// 	read_elf(argv[1]);
// 	return 0;
// }