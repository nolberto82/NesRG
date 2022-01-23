import csv

file = open("opcodes.csv", "r")
csvread = csv.reader(file)
header = next(csvread)
#print(header)
lines = []

opstring = '''adc,and,asl,bcc,bcs,beq,bit,bmi,bne,bpl,brk,bvc,bvs,clc,
\tcld,cli,clv,cmp,cpx,cpy,dec,dex,dey,eor,inc,inx,iny,jmp,
\tjsr,lda,ldx,ldy,lsr,nop,ora,pha,php,pla,plp,rol,ror,rti,
\trts,sbc,sec,sed,sei,sta,stx,sty,tax,tay,tsx,txa,txs,tya,err'''

oplist = opstring.split(",")

#print(oplist)

for line in csvread:
	lines.append(line[1:])

file.close()

with open("include/opcodes.h","w") as wfile:

	wfile.write("#pragma once\n\n")

	wfile.write('enum opcid\n{\n\t')

	for i,l in enumerate(oplist):
		wfile.write('%s,' % (l.upper()))

	wfile.write('\n};\n\n')

	wfile.write('''struct disasmdata\n{\n\tchar* name;\n\topcid id;
\tint mode;\n\tint size;\n\tint cycles;\n};\n\n''')

	wfile.write('enum addrmode\n{' '\
	\n\timme,\n\tzerp,\n\tzerx,\n\tzery,\n' '\
	abso,\n\tabsx,\n\tabsy,\n' '\
	indx,\n\tindy,\n\tindi,\n' '\
	rela,\n\timpl,\n\taccu,\n\terro\n};\n\n')

	#wfile.write('static u16(Cpu::*cpufuncs)();\n\n')

	wfile.write("static struct disasmdata disasm[256] = \n{\n")
	opid = 0
	for l in lines:
		if l[1] == 'err':
			wfile.write('\t{"%s", opcid::%s ,%so, %s, %s}, //0x%02X\n' % (l[0],l[0].upper(),l[1],1,l[3],opid))
		else:
			wfile.write('\t{"%s", opcid::%s, %s, %s, %s}, //0x%02X\n' % (l[0],l[0].upper(),l[1],l[2],l[3],opid))
		opid += 1
		#print("{%s,%s,%s,%s}," % (l[0],l[1],l[2],l[3]))

	wfile.write("};")

with open("switchcases.txt", "w") as wfile:
	wfile.write('switch ()\n{')

	for i,l in enumerate(oplist):
		wfile.write('\ncase opcid::%s:\n{\n\tbreak;\n}' % l.replace('\t', '').replace('\n', '').upper())

	wfile.write('\n}')