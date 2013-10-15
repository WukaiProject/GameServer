#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 读取tab为分隔符的配置文件
#===============================================================================
# Win的UTF8的BOM字节
UTF8_BOM = chr(239) + chr(187) + chr(191)

def strip(s):
	'''
	去掉换行符
	@param s:字符串
	'''
	if s.endswith("\r\n"):
		return s[:-2]
	if s.endswith("\n"):
		return s[:-1]
	return s

class TabLine(object):
	
	def get_all_property(self):
		'''
		返回回一个字典
		这个字典保存了模板实例中的属性对应的函数引用
		'''
		property_dict = {}
		class_propertys = set(dir(self.__class__))
		for name in dir(self):
			# 如果这个字段名是属于类对象的，忽视之
			if name in class_propertys:
				continue
			property_dict[name] = getattr(self, name)
		return property_dict
	
	def read_bool(self, s):
		if s:
			return True if eval(s) else False
		else:
			return False
	
	def read_int(self, s):
		if s:
			return int(s)
		else:
			return 0
	
	def read_eval(self, s):
		if s:
			return eval(s)
		else:
			return None
	
	def read_time(self, s):
		import datetime
		return datetime.time(*eval(s))
	
	def read_date_time(self, s):
		import datetime
		return datetime.datetime(*eval(s))

class TabFile(object):
	def __init__(self, file_path):
		self.data_list = []
		self.file_path = file_path
		self.has_print = False
		with open(self.file_path, 'r') as f:
			self.__read_enname(f)
			self.__read_zhname(f)
			self.__read_data(f)
			
	@staticmethod
	def __line_to_list(s):
		return strip(s).split('\t')
	
	def __read_enname(self, f):
		'''
		读取配子表的每列的英文名
		@param f:文件对象
		'''
		enLine = f.readline()
		if enLine.startswith(UTF8_BOM):
			enLine = enLine[3:]
		self.enname_list = self.__line_to_list(enLine)
		assert len(set(self.enname_list)) == len(self.enname_list)
	
	def __check_with_enname(self, idx, row):
		'''
		检测一行数据是否和英文列名匹配
		@param idx:行索引
		@param row:行数据
		'''
		if len(row) == len(self.enname_list):
			return True
		print "GE_EXC, line(%s) not match enName (%s)" % (idx, self.file_path)
		for idx in xrange(max(len(self.enname_list), len(row))):
			if idx < len(self.enname_list):
				print self.enname_list[idx],
			else:
				print None,
			print " -- ",
			if idx < len(row):
				print row[idx]
			else:
				print None
		return False
	
	def __read_zhname(self, f):
		'''
		读取配子表的每列的中文名
		并且检测匹配问题
		@param f:文件对象
		'''
		self.zhname_list = self.__line_to_list(f.readline())
		self.__check_with_enname(0, self.zhname_list)
	
	def __read_data(self, f):
		'''
		读取配置表数据
		并且检测匹配问题
		@param f:文件对象
		'''
		for idx, line in enumerate(f):
			# 空行，忽视之
			if not line:
				continue
			row = self.__line_to_list(line)
			if self.__check_with_enname(idx, row):
				self.data_list.append(row)
			else:
				continue
	
	def __print_file_path(self):
		if self.has_print:
			return
		print "GE_EXC, %s" % self.file_path
		self.has_print = True
	
	def to_class_type(self, class_type):
		'''
		按照模板信息，将每行读成一个列表
		@param class_type:类对象
		'''
		obj = class_type()
		property_dict = obj.get_all_property()
		# 检测属性和英文名是否匹配
		if not (set(property_dict.iterkeys()) <= set(self.enname_list)):
			self.__print_file_path()
			for k in property_dict.iterkeys():
				if k in self.enname_list:
					continue
				print "GE_EXC, not match property(%s)" % k
			return []
		# 构建属性对应的列
		for idx, enName in enumerate(self.enname_list):
			if enName not in property_dict:
				continue
			property_dict[enName] = (property_dict[enName], idx)
		# 构建结果集
		row_list = []
		for rowIdx, row in enumerate(self.data_list):
			# 为该行构建个对象
			obj = class_type()
			# 对于每一个属性，从配置表中读取并转化之
			is_except = False
			for name, value in property_dict.iteritems():
				fun, idx = value
				try:
					setattr(obj, name, fun(row[idx]))
				except:
					self.__print_file_path()
					print "GE_EXC, line(%s) cell(%s) column(%s), value(%s) can't call by function(%s)" % (rowIdx + 3, idx, name, row[idx], fun.__name__)
					is_except = True
			if is_except is True:
				continue
			row_list.append(obj)
		return row_list
