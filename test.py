import sys
import subprocess

def addIndents(s, n, indent = "\t"):
	lines = []
	line  = ""
	for c in s:
		if (c == "\n"):
			if (line != ""):
				lines.append(line)
				line = ""
		else:
			line += c
	if (line != ""):
		lines.append(line)
	indent *= n
	res = ""
	for l in lines:
		res += indent + l + "\n"
	return res

class testcase:
	out  = ""
	file = ""
	def __init__(self, file):
		self.file = file

def main(params: [str]):
	file    = None
	record  = False
	verbose = False
	for i in range(len(params)):
		if params[i] == "-r":
			record = True
		elif params[i] == "-v":
			verbose = True
		else:
			file = params[i]

	f         = open("tests.txt", "r")
	testsFile = f.read().splitlines()
	f.close()
	testDir   = ""
	tests     = []
	curTest   = None
	for line in testsFile:
		if line.startswith("# "):
			if curTest != None:
				tests.append(curTest)
				curTest = None
			testDir = line[2:]
		elif line.startswith("## "):
			if curTest != None:
				tests.append(curTest)
			curTest = testcase(testDir + "/" + line[3:])
		else:
			curTest.out += line + "\n"
	if curTest != None:
		tests.append(curTest)

	# @Note: For colorcodes, see here: https://stackoverflow.cNone/a/2616912/13764271
	succ  = 0
	total = 0
	testedFile = False
	for t in tests:
		if file == None or file == t.file:
			if file == t.file:
				testedFile = True
			total += 1
			p      = subprocess.run(["run.bat", t.file, "-d"], capture_output=True)
			stdout = p.stdout.decode() + "\n" + p.stderr.decode()
			stdout = "".join(stdout.split("\r")).strip()
			exp    = t.out.strip()

			if (stdout == exp):
				succ += 1
				print("\033[32mTest '" + t.file + "' successful!\033[0m")
			elif (record):
				print("Test '" + t.file + "' re-recorded.")
			else:
				print("\033[31mTest '" + t.file + "' failed:\033[0m")
				if verbose:
					print("  Expected:")
					print(addIndents(exp, 2, "  "))
					print("  Received:")
					print(addIndents(stdout, 2, "  "))

	if file != None and not testedFile:
		if record:
			p      = subprocess.run(["run.bat", file, "-d"], capture_output=True)
			t      = testcase(file)
			stdout = p.stdout.decode() + "\n" + p.stderr.decode()
			stdout = "".join(stdout.split("\r")).strip()
			t.out  = stdout
			tests.append(t)
			print("Test '" + file + "' recorded.")
		else:
			print("\033[31mCannot test '" + file + "' without an expected output in tests.txt.\033[0m")

	print(str(succ) + " out of " + str(total) + " tests succeeded")

	dir = ""
	newTestsFile = ""
	for t in tests:
		paths = t.file.split("/")
		d     = ""
		if len(paths) > 1:
			d = "/".join(paths[0:-1]).strip()
		if d != dir:
			newTestsFile += "# " + d + "\n"
			dir = d
		newTestsFile += "## " + paths[-1] + "\n"
		newTestsFile += t.out
	f = open("tests.txt", "w")
	f.write(newTestsFile)
	f.close()

x = sys.argv[1:]
main(x)