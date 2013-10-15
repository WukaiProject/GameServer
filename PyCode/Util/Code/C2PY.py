#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 解析c++代码
#===============================================================================
import os
import re
	
class CppFile(object):
	'''
	c++文件
	'''
	def __init__(self, fileName):
		self.fileName = fileName
		self.lines = []
		self.cFuns = {}
		self.cFunsOrder = []
		self.pyDefs = {}
		self.cFunDefs = {}
		self.index = 0
	
	def Parse(self):
		'''
		解析这个c++文件
		'''
		f = open(self.fileName, "r")
		self.lines = f.readlines()
		f.close()
		self.index = 0
		size = len(self.lines)
		while self.index < size:
			self.index += 1
			line = self.lines[self.index - 1]
			if self.ParsePyDef(line):
				continue
			if self.ParsePyFunDef(line):
				continue
			if self.ParsePyFun(line):
				continue
	
	def GetContent(self, st):
		'''
		从引号中获取内容
		@param st:
		'''
		return st.strip().strip('"').strip()
	
	def GetParams(self, paramstr):
		params = paramstr.split(",")
		pattern = re.compile(r"\S+")
		args = []
		for param in params:
			items = re.findall(pattern, param)
			argType = "".join(items[:-1])
			argName = items[-1]
			args.append( FunArg(self.index, argType, argName) )
		return args
		
	def ParsePyDef(self, line):
		'''
		
		@param line:
		'''
		# 匹配定义
		pattern = re.compile(r"\W*\{(.*?,.*?,.*?,.*?)\}")
		match = pattern.match(line)
		if not match:
			return False
		s = match.group(1)
		spl = s.split(",")
		namePattern = re.compile(r'\s*"(\w+)"\s*')
		nameMatch = namePattern.match(spl[0])
		if not nameMatch:
			print "GE_ERROR %s Line%s ParsePyDef :[%s] have space char"%(self.fileName, self.index, spl[0])
			assert(False)
		name = nameMatch.group(1)
		if name in self.pyDefs:
			oldPyDef = self.pyDefs[name]
			print "GE_ERROR %s Line%s ParsePyDef :[%s] have repeat PyDef oldLine(%s)"%(self.fileName, self.index, spl[0], oldPyDef.index)
			assert(False)
		funStr = spl[1][ (spl[1].find(")") + 1): ]
		funName = funStr.strip()
		self.pyDefs[name] = PyDef(self.index - 1, name, funName, spl[2].strip(), self.GetContent(spl[3]), self)
		return True
		
	def ParsePyFunDef(self, line):
		pattern = re.compile(r"^\W*PyObject\s*\*\s*(\w+)\s*\((.*(PyObject\*)+.*)\);")
		match = pattern.match(line)
		if not match:
			return False
		funName = match.group(1)
		params = match.group(2)
		args = self.GetParams(params)
		if funName in self.cFunDefs:
			oldFunDef = self.cFunDefs[funName]
			print "GE_ERROR %s Line%s ParsePyFunDef :[%s] have repeat PyFunDef oldLine(%s)"%(self.fileName, self.index, funName, oldFunDef.index)
			assert False
		self.cFunDefs[funName] = CFunDef(self.index - 1, funName, args)
		return True
	
	def ParsePyFun(self, line):
		pattern = re.compile(r"^\W*PyObject\*\s*(\w+)\s*\((\s*(.*?)\s*,\s*(PyObject\*)[^,]*)\)")
		match = pattern.match(line)
		if not match:
			return False
		
		funName = match.group(1)
		params = match.group(2)
		args = self.GetParams(params)
		if funName in self.cFuns:
			oldFun = self.cFuns[funName]
			print "GE_ERROR %s Line%s ParsePyFun :[%s] have repeat PyFun oldLine(%s)"%(self.fileName, self.index, funName, oldFun.index)
			assert False
		fun = CFun(self.index, funName, args)
		
		self.cFuns[funName] = fun
		self.cFunsOrder.append(fun)
		fun.Parse(self.lines, self)
		self.index += fun.size
		return True
	
class CFunDef(object):
	def __init__(self, index, funName, funArgs):
		self.index = index
		self.funName = funName
		self.funArgs = funArgs

ParseType = {"PyArg_ParseTuple":"()", "PyArg_Parse":"()"}
BuildType = {"Py_BuildValue":"()"}
Py2CType = {"PyObjToUI8":"UI8", "PyObjToUI16":"UI16", "PyObjToI32":"I32", 
		  "PyObjToUI32":"UI32", "PyObjToB4":"UI32", "PyObjToUI64":"UI64",
		  "PyObjToB8":"UI64",
		  "PyObjAsUI8":"UI8", "PyObjAsUI16":"UI16", "PyObjAsI32":"I32", 
		  "PyObjAsUI32":"UI32", "PyObjAsB4":"UI32", "PyObjAsUI64":"UI64",
		  "PyObjAsB8":"UI64"}

Py2CType_0 = {"PyString_AS_STRING":"string"}

ParmType = {"s":"string", "H":"UI16", "B":"UI8", "|":"|", "I":"UI32", "O":"PyObject*", "K":"UI64", "L":"I64", "i":"I32", "h":"I16"}

TypeParm = {}
for key, value in ParmType.iteritems():
	assert value not in TypeParm
	TypeParm[value] = key

C2PyType = {"PyObjFromI32":"I32", "PyObjFromUI32":"UI32",
			  "PyObjFromI64":"I64", "PyObjFromUI64":"UI64"
			  ,"PyObjFromB4":"B4","PyObjFromB8":"B8", "PyString_FromString":"String",
			  "PyObj_NewRef":"Object", "List_NewRef":"[]"
			  }

RetType = {"PY_RETUTN_BOOL":"BOOL", "Py_RETURN_NONE":"None", "Py_RETURN_TRUE":"True", "Py_RETURN_FALSE":"False"}


class CFun(CFunDef):
	def __init__(self, index, funName, funArgs):
		CFunDef.__init__(self, index, funName, funArgs)
		self.size = 0
		self.rets = []
		self.wannings = []
		self.cFunArg = None
	
	def GetMethodType(self):
		return self.cFunArg.GetMethodType()
	
	def GetFunArgs(self):
		return self.cFunArg.GetFunArgs()
	
	def GetFunArgsDef(self):
		return self.cFunArg.GetFunArgsDef()
	
	def GetReturns(self):
		return self.rets
		
	def Parse(self, lines, cppFile):
		'''
		解析
		@param lines:
		'''
		size = 0
		left = 0
		right = 0
		while not (left > 0 and right > 0 and left == right) :
			
			ind = (size + self.index)
			if ind >= len(lines):
				print "GE_ERROR ", self.funName, self.index, size
				assert False
			line = lines[size + self.index]
			if self.IsBegin(line):
				left += 1
			elif self.IsEnd(line):
				right += 1
			size += 1
		self.size += size
		
		argLists = self.funArgs[0:]
		root = CFunArg(None, 'O')
		root.Accept(None, [CFunArg(argL.argName, argL.argType, False) for argL in argLists])
		wPattern = re.compile(r"\w+")
		wPattern2 = re.compile(r'''\"(.*?)\"''')
		paPattern = re.compile(r'''(\(\s*(\w+)\s*,(.*?)\))''')
		paPattern_0 = re.compile(r'''.*?(\(\s*(\w+)\s*\))''')
		SSize = self.size
		SIndex = self.index
		for index in xrange(SIndex, SIndex + SSize):
			line = lines[index]
			ites = re.findall(wPattern, line)
			for i, ite in enumerate(ites):
				if ite in RetType:
					self.rets.append((index, RetType[ite]))
					break;
				if ite == "return":
					# 判断其它的返回值类型
					tTypes = ites[i + 1:]
					isReturnOK = False
					for tType in tTypes:
						if tType in C2PyType:
							self.rets.append((index, C2PyType[tType]))
							isReturnOK = True
							break
						elif tType == "NULL":
							isReturnOK = True
							break
						elif tType in BuildType:
							isReturnOK = True
							newLine = line[line.index(tType) + len(tType):]
							argTypes = newLine.split(",")[0]
							argTypes = re.findall(wPattern2, argTypes)[0]
							r =  ",".join([ParmType[k] for k in argTypes])
							self.rets.append((index, r))
							break
					if not isReturnOK:
						self.rets.append((index, "PyObject*"))
						self.wannings.append(index)
						#print "GE_WANNING %s Line%s ParsePyFun [%s]: return unknow type(%s)"%(cppFile.fileName, index, self.funName, line.strip())
#						assert(False)
						continue
					break
				if ite in Py2CType:
					newLine = line[line.index(ite) + len(ite):]
					paMatch = paPattern.match(newLine)
					if paMatch:
						pyArgName = paMatch.group(2)
						cArgName = self.FormString( paMatch.group(3), wPattern )
						cArg = CFunArg(cArgName, TypeParm[Py2CType[ite]])
						root.Accept(pyArgName, [cArg])
					else:
						print "GE_ERROR %s Line%s ParsePyFun [%s]: can't match paPattern line(%s)"%(cppFile.fileName, index, self.funName, line)
						assert False
					break
				
				if ite in Py2CType_0:
					sI = line.index(ite) + len(ite)
					newLine = line[sI:]
					paMatch_0 = paPattern_0.match(newLine)
					if paMatch_0:
						pyArgName = paMatch_0.group(2)
						pyTypeName = TypeParm[Py2CType_0[ite]]
						root.Accept(pyArgName, [CFunArg("c"+pyArgName, pyTypeName)])
					else:
						print "GE_ERROR %s Line%s ParsePyFun [%s]: can't match paPattern_0 line(%s)"%(cppFile.fileName, index, self.funName, line)
						assert False
					break
				
				if ite in ParseType:
					sI = line.index(ite) + len(ite)
					newLine = line[sI:]
					newLine = newLine[newLine.index("(") + 1:].strip()
					spls = newLine.split(",")
					splPyName  = spls[0].strip()
					splTypes = spls[1].strip().strip('"')
					#					arg0 = spl.strip().strip("(")
					cFunArgs = []
					j = 0
					for i, t in enumerate(splTypes):
						if t == "|":
							cFunArgs.append(CFunArg("|", t))
							continue
						if t in ParmType:
							splCName = self.GetSimpleName(spls[2 + j], wPattern)
							j += 1
							cFunArgs.append(CFunArg(splCName, t))
							continue
						elif t:
							print "GE_ERROR %s Line%s ParsePyFun [%s]: can't Parse Type t(%s)"%(cppFile.fileName, index, self.funName, t)
							assert False
#					print index, self.funName, 
#					for arg in cFunArgs:
#						print (arg.argName , arg.argType), 
#					print 
					root.Accept(splPyName, cFunArgs)
					break
				
				self.AcceptArg(ite, root.children, index, line)
#		if root.children[1].children and len(root.children[1].children):
		self.cFunArg = root.children[1]
	
	def AcceptArg(self, name, argList, index , line):
		for i, arg in enumerate(argList):
			if arg.AcceptUse(name) and i > 0:
				self.wannings.append(index)
	
	def GetSimpleName(self, s, pattern):
		s2 = s[0:]
		leftIndex = s2.find("(")
		if leftIndex > 0:
			s2 = s2[0 : leftIndex]
		rightIndex = s2.find(")")
		if rightIndex > 0:
			s2 = s2[0 : rightIndex]
		return self.FormString(s, pattern)

	def FormString(self, s, pattern):
		sl = re.findall(pattern, s)
		return "_".join(sl)
	
	def IsBegin(self, line):
		return line.strip() == "{"
		
	def IsEnd(self, line):
		sp = line.strip()
		return sp == "};" or sp == "}"

class PyDef(object):
	'''
	@param object:
	'''
	def __init__(self, index, name, funName, pyType, pyComment, cpp = None):
		self.index = index
		self.name = name
		self.funName = funName
		self.pyType = pyType
		self.pyComment = pyComment
		self.cpp = cpp
		
		
class FunArg(object):
	'''
	函数参数
	'''
	def __init__(self, index, argType, argName):
		self.index = index
		if argType not in ParmType:
			self.argType = "O"
		else:
			self.argType = argType
		self.argName = argName
		
class CFunArg(object):
	'''
	装换为c函数的参数
	'''
	def __init__(self, argName, argType, isUsed =True):
		self.argName = argName
		self.argType = argType
		self.children = None
		self.isUsed = isUsed
		
	def GetMethodType(self):
		if self.children:
			if len(self.children) > 1:
				return "METH_VARARGS"
			else:
				return "METH_O"
		elif self.isUsed:
			return "METH_O"
		else:
			return "METH_NOARGS"
		
	def Accept(self, argName, cFunArgs):
		if self.argName == argName:
			if self.children is None:
				if self.argType != "O":
					print "GE_ERROR CFunArg(%s) have argType (%s) so should no parse"%(self.argName, self.argType)
				self.children = cFunArgs
				self.isUsed = True
				return True
			else:
				print "GE_ERROR CFunArg(%s) have children(%s) "%(self.argName, self.children)
				assert False
		else:
			if self.children:
				for child in self.children:
					if child.Accept(argName, cFunArgs):
						self.isUsed = True
						return True
		return False
	
	def AcceptUse(self, argName):
		if self.argName == argName:
			self.isUsed = True
			return True
		return False
	
	def GetFunArgs(self, isDefault = False):
		if not self.children:
			if self.isUsed:
				if isDefault:
					return " %s=None "%(self.argName)
				else:
					return " %s"%(self.argName)
			else:
				return " "
		l = []
		isDf = False
		for child in self.children:
			if child.argType == "|":
				isDf = True
				continue
			l.append(child.GetFunArgs(isDf))
		return ",".join(l)
	
	def GetFunArgsDef(self, isDefault = False):
		if not self.children:
			if self.isUsed:
				if isDefault:
					return ["@param %s : %s"%(self.argName, ParmType[self.argType])]
				else:
					return ["@param %s : %s"%(self.argName, ParmType[self.argType])]
			else:
				return []
		s = []
		isDf = False
		for child in self.children:
			if child.argType == "|":
				isDf = True
				continue
			s.extend(child.GetFunArgsDef(isDf))
		return s
	
	def Test(self):
		if not self.children:
			if self.isUsed:
				return "%s#%s"%(self.argName, self.argType)
			else:
				return "#"
		s = "("
		for child in self.children:
			s += child.Test() + ", "
		s += ")"
		return s

class PyHelp(object):
	def __init__(self, helpFile, path, cppParentFolder):
		self.helpFile = path + helpFile
		self.files = []
		self.cppParentFolder = cppParentFolder
		
	def Append(self, cppFile):
		if self.files.count(cppFile) > 0:
			return
		else:
			self.files.append(cppFile)
	
		# 检查定义和实现是否正确
	def Parse(self):
		cpps = []
		pyNameDefs = {}
		funNameDefs = {}
		
		for f in self.files:
			cpp = CppFile(self.cppParentFolder +os.sep + f)
			cpp.Parse()
#			print cpp.pyDefs
			for k, v in cpp.pyDefs.iteritems():
				assert k not in pyNameDefs
				pyNameDefs[k] = v
				if v.funName in funNameDefs:
					ov = funNameDefs[v.funName]
					print "GE_ERROR file(%s) funName(%s) line(%s) repeat old funName (%s) line(%s)"%(cpp.fileName, v.funName, v.index, ov.funName, ov.index)
				funNameDefs[v.funName] = v
			cpps.append(cpp)
		#收集所有的定义
		for cpp in cpps:
			for key, cfun in cpp.cFuns.iteritems():
				if key not in funNameDefs:
					print "GE_ERROR, not in defs ", cpp.fileName, key
					assert False
				pydef = funNameDefs[key]
				mtype = cfun.GetMethodType()
				if mtype != pydef.pyType:
					pass
					#print "GE_WANNING, file(%s) method(%s) have a error method def(%s) to  " % (pydef.cpp.fileName, key, pydef.pyType)
					#print "		file(%s) functions(%s) method type(%s) args%s " % (cpp.fileName, key, mtype, cfun.GetFunArgs())
					#if not cfun.wannings:
					#	assert False
		readLines = []
		try:
			fr = open(self.helpFile, "r")
			readLines = fr.readlines()
		except:
			pass
		# 自动生成的代码使用标识
		
		start_sign = "#automatic_start"
		end_sign = "#automatic_end"
		
		f = open(self.helpFile, "w")
		startIndex = -1
		endIndex = -1
		for i in xrange(0, len(readLines)):
			line = readLines[i]
			if startIndex == -1 and line.startswith(start_sign):
				startIndex = i
			if line.startswith(end_sign):
				endIndex = i
		if startIndex == -1:
			startIndex = 0
		if endIndex == -1:
			endIndex = len(readLines)
		
		headlines = readLines[0:startIndex]
		taillines = readLines[endIndex + 1:]

		if headlines:
			f.writelines(headlines)
		else:
			f.write("#!/usr/bin/env python\n")
			f.write("# -*- coding:UTF-8 -*-\n")
		f.write(start_sign)
		f.write("\n")
		for cpp in cpps:
			if cpp.cFunsOrder:
				ls = []
				ls.append("# ---------------------%s------------------------"%(cpp.fileName))
				ls.append("# 有%s个Py函数定义"%(len(cpp.cFunsOrder)))
				ls.append("")
				for fun in cpp.cFunsOrder:
					
					funName = fun.funName
					pydef = funNameDefs[funName]
					argNames = fun.GetFunArgs()
					ls.append( "def %s(%s):"%(pydef.name, argNames) )
					argDefs = fun.GetFunArgsDef()
					ls.append("	'''")
					ls.append("	line %s"%(fun.index))
					ls.append("	%s"%(pydef.pyComment))
					for argDef in argDefs:
						ls.append("	%s"%(argDef))
					rets = fun.GetReturns()
					for ret in rets:
						index, r = ret
						ls.append("	@return: %s \n			 line %s %s"%(r, index, cpp.lines[index].strip()))	
					for warn in fun.wannings:
						ls.append("	@warning: %s"%(cpp.lines[warn].strip()))
					ls.append("	@see : %s"%(pydef.cpp.lines[pydef.index].strip()) )
					ls.append("	'''")
					ls.append("")
				for l in ls:
					f.write(l+"\n")
		f.write(end_sign)
		f.write("\n")
		if taillines:
			f.writelines(taillines)
		f.close()
		print ">>> build file(%s)" % self.helpFile


