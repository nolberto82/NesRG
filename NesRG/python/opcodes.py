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

with open("../src/include/opcodes.h","w") as wfile:

	wfile.write("#pragma once\n\n")

	wfile.write('enum opcid\n{\n\t')

	for i,l in enumerate(oplist):
		wfile.write('%s,' % (l.upper()))

	wfile.write('\n};\n\n')

	wfile.write('''struct disasmdata\n{\n\tconst char* name;\n\topcid id;
\tint mode;\n\tint size;\n\tint cycles;\n\tint extracycle;\n};\n\n''')

	wfile.write('enum addrmode\n{' '\
	\n\timpl,\n\taccu,\n\timme,\n\tzerp,\n\tzerx,\n\tzery,\n' '\
	abso,\n\tabsx,\n\tabsy,\n\tindx,\n\tindy,\n\tindi,\n' '\
	rela,\n\terro\n};\n\n')

	wfile.write('static const char* mode_formats[]\n{' '\
	\n\t{"%04X %-8.02X  %-3s"}, //impl' '\
	\n\t{"%04X %-8.02X  %-3s A"}, //accu' '\
	\n\t{"%04X %02X %-5.02X  %-3s #$%02X"}, //imme' '\
	\n\t{"%04X %02X %-5.02X  %-3s $%02X = $%02X"}, //zerp' '\
	\n\t{"%04X %02X %-5.02X  %-3s $%02X,X @ $%02X = $%02X"}, //zerx' '\
	\n\t{"%04X %02X %-5.02X  %-3s $%02X,Y @ $%02X = $%02X"}, //zery' '\
	\n\t{"%04X %02X %02X %-2.02X  %-3s $%04X = $%02X"}, //abso' '\
	\n\t{"%04X %02X %02X %-2.02X  %-3s $%04X,X @ $%04X = $%02X"}, //absx' '\
	\n\t{"%04X %02X %02X %-2.02X  %-3s $%04X,Y @ $%04X = $%02X"}, //absy' '\
	\n\t{"%04X %02X %-5.02X  %-3s ($%02X,X) @ $%04X = $%02X"}, //indx' '\
	\n\t{"%04X %02X %-5.02X  %-3s ($%02X),Y @ $%04X = $%02X"}, //indy' '\
	\n\t{"%04X %02X %02X %-2.02X  %-3s ($%04X) @ $%04X = $%02X"}, //indi' '\
	\n\t{"%04X %02X %-5.02X  %-3s $%04X = $%02X"}, //rela' '\
	\n};\n\n')

	wfile.write("static struct disasmdata disasm[256] = \n{\n")
	opid = 0
	for l in lines:
		if l[1] == 'err':
			wfile.write('\t{"%s", opcid::%s ,%so, %s, %s, %s}, //0x%02X\n' % (l[0].upper(),l[0].upper(),l[1],1,l[3],l[4],opid))
		else:
			wfile.write('\t{"%s", opcid::%s, %s, %s, %s, %s}, //0x%02X\n' % (l[0].upper(),l[0].upper(),l[1],l[2],l[3],l[4],opid))
		opid += 1
		#print("{%s,%s,%s,%s}," % (l[0],l[1],l[2],l[3]))

	wfile.write("};")

with open("switchcases.txt", "w") as wfile:
	wfile.write('switch ()\n{')

	for i,l in enumerate(oplist):
		wfile.write('\ncase opcid::%s:\n{\n\tbreak;\n}' % l.replace('\t', '').replace('\n', '').upper())

	wfile.write('\n}')