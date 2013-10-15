#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# MySQL动态表定义的基本列
# 关于个列的默认值，权衡考虑下决定还是写死吧
#===============================================================================
from Common import Int

INFO_NAME = 0
INFO_TYPE = 1
INFO_NULL = 2
INFO_KEY = 3
INFO_EXTEND = 5

# 整数
class IntColumn(object):
	TINY_INT = "tinyint(3)"
	SMALL_INT = "smallint(6)"
	INT = "int(10)"
	BIG_INT = "bigint(20)"
	# 定义各种组合的取值范围
	SCOPE = {(TINY_INT, ""): (Int.MIN_INT8, Int.MAX_INT8),
			(TINY_INT, "unsigned"): (Int.MIN_UINT8, Int.MAX_UINT8),
			(SMALL_INT, ""): (Int.MIN_INT16, Int.MAX_INT16),
			(SMALL_INT, "unsigned"): (Int.MIN_UINT16, Int.MAX_UINT16),
			(INT, ""): (Int.MIN_INT32, Int.MAX_INT32),
			(INT, "unsigned"): (Int.MIN_UINT32, Int.MAX_UINT32),
			}
	
	def __init__(self, name, tint = INT, unsigned = False, auto_increment = False, comment = ""):
		'''
		Python:Int --> MYSQL:TinyInt SmallInt Int BigInt
		@param name:列名
		@param tint:类型 TINY_INT SMALL_INT INT BIG_INT
		@param unsigned:是否是无符号的
		@param auto_increment:是否自增长
		@param comment:注释
		'''
		assert(-1 == comment.find("'"))
		self.name = name
		# 好用
		tint = tint.lower()
		if tint.startswith("tiny"):
			self.tint = self.TINY_INT
		elif tint.startswith("small"):
			self.tint = self.SMALL_INT
		elif tint.startswith("int"):
			self.tint = self.INT
		elif tint.startswith("big"):
			self.tint = self.BIG_INT
		else:
			print "GE_EXC, unknow tint(%s), use default int." % tint
			self.tint = self.INT
		self.unsigned = "unsigned" if unsigned else ""
		self.auto_increment = "auto_increment" if auto_increment else ""
		self.default = "DEFAULT 0" if not auto_increment else ""
		self.comment = comment
	
	def CreateColumnSQL(self):
		return "%s %s %s NOT NULL %s %s COMMENT '%s'" % (self.name, self.tint, self.unsigned, self.auto_increment, self.default, self.comment)
	
	def AddColumnSQL(self, dtable):
		return "ALTER TABLE %s ADD %s %s %s NOT NULL %s %s COMMENT '%s'" % (dtable.name, self.name, self.tint, self.unsigned, self.auto_increment, self.default, self.comment)
	
	def ChangeColumnSQL(self, dtable):
		return "ALTER TABLE %s MODIFY %s %s %s NOT NULL %s %s COMMENT '%s'" % (dtable.name, self.name, self.tint, self.unsigned, self.auto_increment, self.default, self.comment)
	
	def __GetTypeLevel(self, tint):
		tint = tint.lower()
		if tint.startswith(self.TINY_INT):
			return 1
		if tint.startswith(self.SMALL_INT):
			return 2
		if tint.startswith(self.INT):
			return 3
		if tint.startswith(self.BIG_INT):
			return 4
		return 0
	
	def IsEqual(self, info, dtable):
		if self.unsigned:
			tint = "%s %s" % (self.tint, self.unsigned)
		else:
			tint = self.tint
		if info[INFO_TYPE] != tint:
			return False
		if not self.auto_increment:
			return True
		return info[INFO_EXTEND] == self.auto_increment
	
	def IsCompatible(self, info, dtable):
		# 如果没变，兼容
		if self.IsEqual(info, dtable):
			return True
		# 如果自增长改变了，不兼容
		if info[INFO_EXTEND] != self.auto_increment:
			return False
		# 计算整数等级
		olevel = self.__GetTypeLevel(info[INFO_TYPE])
		nlevle = self.__GetTypeLevel(self.tint)
		# 计算不出等级，不兼容
		if olevel == 0 or nlevle == 0:
			return False
		# 新的等级大于等于旧的等级，兼容
		if nlevle >= olevel:
			return True
		# 如果整数的范围缩小了，检测已有的数据是否在新的范围内
		minv, maxv = self.SCOPE[(self.tint, self.unsigned)]
		return dtable.IsScope(self.name, minv, maxv)
	
# 字符串
class StringColumn(object):
	def __init__(self, name, maxsize = 10000, comment = ""):
		'''
		Python:string --> MYSQL:varchar
		@param name:列名
		@param maxsize:字符串最大长度
		@param comment:注释
		'''
		assert(-1 == comment.find("'"))
		self.name = name
		self.maxsize = maxsize
		self.comment = comment
	
	def CreateColumnSQL(self):
		return "%s varchar(%s) NOT NULL DEFAULT '' COMMENT '%s'" % (self.name, self.maxsize, self.comment)
	
	def AddColumnSQL(self, dtable):
		return "ALTER TABLE %s ADD %s varchar(%s) NOT NULL DEFAULT '' COMMENT '%s'" % (dtable.name, self.name, self.maxsize, self.comment)
	
	def ChangeColumnSQL(self, dtable):
		return "ALTER TABLE %s MODIFY %s varchar(%s) NOT NULL DEFAULT '' COMMENT '%s'" % (dtable.name, self.name, self.maxsize, self.comment)
	
	def IsEqual(self, info, dtable):
		return info[INFO_TYPE] == "varchar(%s)" % self.maxsize
	
	def IsCompatible(self, info, dtable):
		return self.IsEqual(info, dtable)

class TextColumn(object):
	def __init__(self, name, comment = ""):
		'''
		Python:string --> MYSQL:varchar
		@param name:列名
		@param maxsize:字符串最大长度
		@param comment:注释
		'''
		assert(-1 == comment.find("'"))
		self.name = name
		self.comment = comment
	
	def CreateColumnSQL(self):
		return "%s text NOT NULL COMMENT '%s'" % (self.name, self.comment)
	
	def AddColumnSQL(self, dtable):
		return "ALTER TABLE %s ADD %s text NOT NULL COMMENT '%s'" % (dtable.name, self.name, self.comment)
	
	def ChangeColumnSQL(self, dtable):
		return "ALTER TABLE %s MODIFY %s text NOT NULL COMMENT '%s'" % (dtable.name, self.name, self.comment)
	
	def IsEqual(self, info, dtable):
		return info[INFO_TYPE] == "text"
	
	def IsCompatible(self, info, dtable):
		return self.IsEqual(info, dtable)

# 除整数和字符串以外的其他Python对象
class ObjColumn(object):
	def __init__(self, name, comment = ""):
		'''
		Python:obj --> MYSQL:blob
		@param name:列名
		@param comment:注释
		'''
		assert(-1 == comment.find("'"))
		self.name = name
		self.comment = comment
	
	def CreateColumnSQL(self):
		return "%s blob NULL COMMENT '%s'" % (self.name, self.comment)
	
	def AddColumnSQL(self, dtable):
		return "ALTER TABLE %s ADD %s blob NULL COMMENT '%s'" % (dtable.name, self.name, self.comment)
	
	def ChangeColumnSQL(self, dtable):
		return "ALTER TABLE %s MODIFY %s blob NULL COMMENT '%s'" % (dtable.name, self.name, self.comment)
	
	def Python2MySQL(self, v):
		return repr(v)
	
	def MySQL2Python(self, v):
		return eval(v)
	
	def IsEqual(self, info, dtable):
		return info[INFO_TYPE] == "blob"
	
	def IsCompatible(self, info, dtable):
		return self.IsEqual(info, dtable)

class DateTimeColumn(object):
	def __init__(self, name, comment = ""):
		'''
		Python:datetime --> MYSQL:datetime
		@param name:列名
		@param comment:注释
		'''
		assert(-1 == comment.find("'"))
		self.name = name
		self.comment = comment
	
	def CreateColumnSQL(self):
		return "%s datetime NOT NULL DEFAULT '2012-1-1 0:0:0' COMMENT '%s'" % (self.name, self.comment)
	
	def AddColumnSQL(self, dtable):
		return "ALTER TABLE %s ADD %s datetime NOT NULL DEFAULT '2012-1-1 0:0:0' COMMENT '%s'" % (dtable.name, self.name, self.comment)
	
	def ChangeColumnSQL(self, dtable):
		return "ALTER TABLE %s MODIFY %s datetime NOT NULL DEFAULT '2012-1-1 0:0:0' COMMENT '%s'" % (dtable.name, self.name, self.comment)
	
	def IsEqual(self, info, dtable):
		return info[INFO_TYPE] == "datetime"
	
	def IsCompatible(self, info, dtable):
		return self.IsEqual(info, dtable)

class Key(object):
	NO = ""			#无约束
	MUL = "MUL"		#索引
	UNI = "UNI"		#唯一索引
	PRI = "PRI"		#主键
	def __init__(self, name, tkey = MUL):
		'''
		主键
		@param name:列名
		'''
		self.name = name
		tkey = tkey.upper()
		if tkey in (self.NO, self.MUL, self.UNI, self.PRI):
			self.tkey = tkey
		else:
			#非法约束
			self.tkey = self.NO
			print "GE_EXC, unknown key(%s) use default NO." % tkey
	
	def CreateKeySQL(self):
		if self.tkey == self.MUL:
			return "KEY %s(%s)" % (self.name, self.name)
		elif self.tkey == self.UNI:
			return "UNIQUE KEY %s(%s)" % (self.name, self.name)
		elif self.tkey == self.PRI:
			return "PRIMARY KEY (%s)" % self.name
		assert(False)
	
	def AddKeySQL(self, dtable):
		if self.tkey == self.MUL:
			return "ALTER TABLE %s ADD INDEX %s(%s)" % (dtable.name, self.name, self.name)
		elif self.tkey == self.UNI:
			return "ALTER TABLE %s ADD UNIQUE %s(%s)" % (dtable.name, self.name, self.name)
		elif self.tkey == self.PRI:
			return "ALTER TABLE %s ADD PRIMARY KEY (%s)" % (dtable.name, self.name)
		assert(False)
	
	def IsUnique(self):
		return self.tkey == self.UNI or self.tkey == self.PRI

	def IsEqual(self, info, dtable):
		return info[INFO_KEY] == self.tkey

def ToMUL(name):
	'''
	构造一个列的索引描述
	@param name:索引名（或者列对象）
	'''
	_name = getattr(name, "name", None)
	if _name:
		name = _name
	assert isinstance(name, str)
	return Key(name, Key.MUL)

def ToUNI(name):
	'''
	构造一个列的唯一索引描述
	@param name:索引名（或者列对象）
	'''
	_name = getattr(name, "name", None)
	if _name:
		name = _name
	assert isinstance(name, str)
	return Key(name, Key.UNI)

def ToPRI(name):
	'''
	构造一个列的主键索引描述
	@param name:索引名（或者列对象）
	'''
	_name = getattr(name, "name", None)
	if _name:
		name = _name
	assert isinstance(name, str)
	return Key(name, Key.PRI)

def DropColumnSQL(info, dtable):
	assert(not dtable.HasData())
	return "ALTER TABLE %s DROP %s" % (dtable.name, info[INFO_NAME])

def DropKeySQL(info, dtable):
	name = info[INFO_NAME]
	tkey = info[INFO_KEY]
	if tkey == Key.MUL:
		return "ALTER TABLE %s DROP INDEX %s" % (dtable.name, name)
	elif tkey == Key.UNI:
		return "ALTER TABLE %s DROP INDEX %s" % (dtable.name, name)
	elif tkey == Key.PRI:
		return "ALTER TABLE %s DROP PRIMARY KEY" % (dtable.name)
	assert(False)
