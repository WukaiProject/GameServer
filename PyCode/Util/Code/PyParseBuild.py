#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 模块解析和构建
#===============================================================================
import os
import types
import string
import Path

def pycode_round(l):
	'''
	将一行python代码截尾（注意，会自动截掉注释）
	@param l:一行代码
	'''
	# 去注释
	pos = l.find('#')
	if pos != -1:
		l = l[:pos]
	# 去掉末尾的tab和空格
	while 1:
		if not l: break
		if l[-1] in ('	', ' '):
			l = l[:-1]
		else:
			break
	return l

def pycode_trip(l):
	'''
	将python代码字符串的头尾的空格和tab去掉
	@param l:代码字符串
	'''
	# 去掉头部tab和空格
	while 1:
		if not l: break
		if l[0] in ('	', ' '):
			l = l[1:]
		else:
			break
	# 去掉末尾的tab和空格
	while 1:
		if not l: break
		if l[-1] in ('	', ' '):
			l = l[:-1]
		else:
			break
	return l

class PyFile(object):
	def __init__(self, module_name):
		self.module_name = module_name
		self.file_path = Path.PYCODE_FOLDER_PATH + module_name.replace('.', os.sep) + ".py"
		with open(self.file_path) as f:
			self.file_list = f.read().split('\n')
			for idx, line in enumerate(self.file_list):
				if line.endswith('\r'):
					self.file_list[idx] = line[:-1]
	
	def replace_code(self, code_list, begin_flag = "AutoCodeBegin", end_flag = "AutoCodeEnd"):
		isreplace, hasbegin, hasend = False, False, False
		new_list = []
		for l in self.file_list:
			if l.find(begin_flag) != -1:
				# 必须没遇到过开始标识
				assert(not hasbegin)
				isreplace = True
				hasbegin = True
				new_list.append(l)
				new_list.extend(code_list)
				continue
			if l.find(end_flag) != -1:
				# 必须先遇到开始标识
				assert(hasbegin)
				isreplace = False
				hasend = True
				new_list.append(l)
				continue
			if not isreplace:
				new_list.append(l)
		if hasbegin and hasend:
			self.file_list = new_list
		else:
			print "module(%s) not find begin flag(%s) and end flat(%s)" % (self.module_name, hasbegin, hasend)
	
	def replace_enumerate(self, enum_name, enum_info, begin_flag = "AutoCodeBegin", end_flag = "AutoCodeEnd"):
		code_list = ["# %s" % enum_name]
		for info in enum_info:
			code_list.append("%s = %s\t\t#%s" % info)
		self.replace_code(code_list, begin_flag, end_flag)
	
	def write(self):
		# 没有内容，不写
		if not self.file_list:
			return
		with open(self.file_path, 'w') as f:
			f.write('\n'.join(self.file_list))
		print ">>> build module(%s)" % self.module_name
	
	def get_equal_info(self):
		'''
		获取模块中的等式信息
		@return: [(key, value, zs), ...]
		'''
		result = []
		for l in self.file_list:
			pos = l.find('#')
			if pos == -1:
				co, zs = l, ""
			else:
				co, zs = l[:pos], l[pos + 1:]
			pos = co.find('=')
			if pos == -1:
				continue
			k, v = pycode_trip(co[:pos]), pycode_trip(co[pos + 1:])
			result.append((k, v, zs))
		return result
	
	def get_model_execute_codes_on_reload(self):
		'''
		获取在relaod的时候会执行的代码信息
		'''
		result = []
		tab = False
		for idx, l in enumerate(self.file_list):
			# 注释
			if l.startswith("#"):
				continue
			# 截尾
			l = pycode_round(l)
			# 空字符串
			if not l:
				continue
			# 代码以缩进开头
			if l.startswith('	'):
				# 如果此时会执行缩进代码，记录之
				if tab:
					result.append((idx, l))
				# 此行代码已经处理完毕了
				continue
			else:
				# 标记缩进逻辑结束了
				tab = False
			# 导入
			if l.startswith("from "):
				continue
			if l.startswith("import "):
				continue
			# 函数定义
			if l.startswith("def"):
				continue
			# 类定义
			if l.startswith("class"):
				continue
			# 必定是测试逻辑和reloa的时候不会执行的代码，忽视之
			if l.startswith("if __name__ == '__main__'") \
			or l.startswith('if __name__ == "__main__"') \
			or l.startswith("if '_HasLoad' not in dir():") \
			or l.startswith('if "_HasLoad" not in dir():'):
				continue
			# 以导致缩进开头的关键字开始的代码行，标记会执行缩进代码
			if l.startswith("if ") \
			or l.startswith("for ") \
			or l.startswith("elif") \
			or l.startswith("else") :
				tab = True
			result.append((idx, l))
		return result

class PyModule(object):
	CODE_CHARS = string.letters + string.digits + "_" + '"'
	def __init__(self, module):
		self.module = module
		self.pyfile = PyFile(module.__name__)
		self.funs = []
		self.clss = []
		for name in dir(self.module):
			obj = getattr(self.module, name)
			if type(obj) == types.FunctionType and obj.__module__ == self.module.__name__:
				self.funs.append(obj)
			elif type(obj) in (types.TypeType, types.ClassType) and obj.__module__ == self.module.__name__:
				self.clss.append(obj)
	
	def __has_user_obj(self, l, objName):
		pos = l.find(objName)
		if pos < 0: return False
		p = pos -1
		if p >= 0 and l[p] in self.CODE_CHARS:
			return False
		n = pos + len(objName)
		if n < len(l) and l[n] in self.CODE_CHARS:
			return False
		return True
	
	def check_self_inherit(self):
		'''
		检测一个模块中是否有类继承该模块的其他类
		'''
		errorList = []
		for c1 in self.clss:
			for c2 in self.clss:
				if c1 == c2: continue
				if not issubclass(c1, c2): continue
				errorList.append((c1, c2))
		if not errorList: return
		print "-------- module(%s)" % self.module.__name__
		for c1, c2 in errorList:
			print "%s is subclass of %s" % (c1.__name__, c2.__name__)
	
	def check_self_user(self):
		'''
		检测一个模块中，是否有reload执行的代码中使用了该模块的函数和类
		'''
		errorList = []
		for idx, l in self.pyfile.get_model_execute_codes_on_reload():
			for fun in self.funs:
				if not self.__has_user_obj(l, fun.__name__):
					continue
				errorList.append((idx, l, fun.__name__))
			for cls in self.clss:
				if not self.__has_user_obj(l, cls.__name__):
					continue
				errorList.append((idx, l, cls.__name__))
		if not errorList: return
		print "======== module(%s)" % self.module.__name__
		for idx, l, name in errorList:
			print "%s line(%s) user %s" % (idx, l, name)
	
	def get_enumerate_info(self, uniq = True):
		'''
		获取这个模块中枚举的信息
		@return: [(变量名, 值, 注释)]
		'''
		result = []
		us = set()
		for k, _, zs in self.pyfile.get_equal_info():
			if not hasattr(self.module, k):
				continue
			if uniq and k in us:
				continue
			result.append((k, getattr(self.module, k), zs))
			us.add(k)
		return result
