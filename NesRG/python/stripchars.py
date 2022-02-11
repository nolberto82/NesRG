import glob

lines = []
files = []

files = glob.glob('./*.log')

with open(files[0], "r") as rfile:
	for s in rfile:
		if "RTS" in s:
			i = s.index("RTS")
			lines.append(s[0:i+3] + '\n')
		else:
			lines.append(s)
			#print(s[0:i + 3])

with open("nestest_new.log","w") as wfile:
	for s in lines:
		wfile.write(s)