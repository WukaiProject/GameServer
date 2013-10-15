#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 解析C++代码生成相应的Python智能提示函数
#===============================================================================
import Path

def trip_chars(s, chars = (' ', '\t')):
	'''
	去掉字符串前后的特点字符
	@param s:字符串
	@param charTuple:字符串前后忽视的char集合
	'''
	_begin = None
	_end = 0
	for _idx, _c in enumerate(s):
		if _c not in chars:
			if _begin is None:
				_begin = _idx
			_end = _idx
	if _begin is None:
		return ""
	else:
		return s[_begin:_end + 1]

def parse_comment(s):
	'''
	从一个字符串中解析出如代码部分和注释部分
	'''
	_pos = s.find(r"//")
	if _pos == -1:
		return s, ""
	else:
		return s[:_pos], s[_pos + 2:]

def parse_equal(s):
	'''
	从一个字符串中解析出一个等式的左右两边的字符串
	'''
	_pos = s.find("=")
	if _pos == -1:
		return None, None
	_l = s[:_pos]
	_r = s[_pos + 1:]
	return _l, _r

def is_in_enum_begin(line, name):
	'''
	是否是一个枚举的开始的一行
	@param line:一行字符串
	@param name:结构体名
	'''
	_code, _annota = parse_comment(line)
	_pe = _code.find("enum")
	_pn = _code.find(name)
	return _pn > _pe + 1 and _pe > -1

def is_begin(line):
	'''
	是否是代码块开始了
	@param line:一行字符串
	'''
	_code, _annota = parse_comment(line)
	return  _code.find("{") != -1

def is_end(line):
	'''
	是否是代码块结束了
	@param line:一行字符串
	'''
	_code, _annota = parse_comment(line)
	return  _code.find("}") != -1

def parse_enum_info(line, last_value):
	'''
	解析出一行C枚举代码的枚举信息
	@param line:字符串行
	@param last_value:上次的枚举值
	@return:枚举名, 枚举值, 注释
	'''
	_code, _annota = parse_comment(line)
	_pos = _code.find(",")
	if _pos != -1:
		_code = _code[:_pos]
	_enumKey, _enumValue = parse_equal(_code)
	if _enumKey:
		_enumKey = trip_chars(_enumKey)
		_enumValue = int(trip_chars(_enumValue))
	else:
		_enumKey = trip_chars(_code)
		_enumValue = last_value + 1
	return _enumKey, _enumValue, _annota

class CFile(object):
	def __init__(self, file_path):
		if file_path.find("CCode") == -1:
			file_path = Path.CCODE_FOLDER_PATH + file_path
		with open(file_path) as f:
			self.file_list = f.read().split("\n")
	
	def GetEnumerate(self, name):
		is_in_enum = False
		is_in_begin = False
		last_value = -1
		ret = []
		for line in self.fileList:
			line = trip_chars(line)
			if not line:
				continue
			if is_in_enum_begin(line, name):
				is_in_enum = True
				continue
			if not is_in_enum:
				continue
			if is_begin(line):
				is_in_begin = True
				continue
			if not is_in_begin:
				continue
			if is_end(line):
				return ret
			
			key, value, annoto = parse_enum_info(line, last_value)
			if key:
				last_value = value
				ret.append((key, value, annoto))
		return ret
