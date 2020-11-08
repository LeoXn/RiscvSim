# RiscvSim
A RISCV simulator supporting rv64i instruction set with readelf function

#运行脚本说明

#运行方法:
	运行make命令，生成main可执行文件	
	运行脚本实例：./main a.out -s	
	a.out为RISCV的ELF文件，-s为调试选项，添加-s选择单步模式	

#-s单步模式说明：
	输入‘n’:运行下一步	
	输入‘e’:终止运行	
	输入‘r’:查看寄存器信息	
	输入‘m’:查看memory信息，要求输入开始地址和Word数，确认即可查看内容	
