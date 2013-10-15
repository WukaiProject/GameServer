#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 回调辅助模块
# 注意，注册的函数回调的时候可能发生异常，要处理之
#===============================================================================
import datetime
import traceback
from Util import Time

class Callback(object):
	def __init__(self, key = None):
		self.fun_list = []
	
	def reg(self, fun, deadtime = (2038, 1, 1), index = -1):
		'''
		注册一个函数
		@param fun:函数对象
		@param deadtime:过期时间
		@param index:索引
		'''
		# 判断过期
		ddt = Time.as_datetime(deadtime)
		if ddt <= datetime.datetime.now():
			return
		# 检测是否重复
		if fun in self.fun_list:
			print "GE_EXC, repeat reg function(%s, %s)" % (fun.__module__, fun.__name__)
			return
		# 按照索引插入
		if index < 0:
			self.fun_list.append(fun)
		else:
			self.fun_list.insert(index, fun)
	
	def call(self, *argv):
		'''
		调用所有的注册的函数
		'''
		except_funs = []
		# 尝试调用各个注册的函数
		for fun in self.fun_list:
			try:
				fun(*argv)
			except:
				# 有异常记录，并打印异常
				except_funs.append(fun)
				traceback.print_exc()
		# 没异常，直接返回
		if not except_funs:
			return
		# 否则移除发生了异常的函数
		for fun in except_funs:
			#self.fun_list.remove(fun)
			print "GE_EXC, !!!!!!Remove except call Fun (%s)!!!!!!" % fun.__name__

# 本地函数注册回调
class Callbacks(object):
	def __init__(self):
		self.fundict = {}
	
	def reg(self, funtype, fun, deadtime = (2038, 1, 1), index = -1):
		'''
		注册一个回调函数
		@param funtype:函数类型
		@param fun:回调函数
		@param deadtime:过期时间
		@param index:索引
		'''
		# 判断过期
		ddt = Time.as_datetime(deadtime)
		if ddt <= datetime.datetime.now():
			return
		if funtype not in self.fundict:
			callback = self.fundict[funtype] = Callback()
		else:
			callback = self.fundict[funtype]
		callback.reg(fun, deadtime, index)
	
	def call(self, funtype, *argv):
		'''
		调用某个类型的所有回调函数
		@param funtype:
		'''
		callback = self.fundict.get(funtype)
		if callback:
			callback.call(*argv)
	
	def callex(self, funtype, *argv):
		'''
		调用某个类型的所有回调函数
		@param funtype:
		'''
		callback = self.fundict.get(funtype)
		if callback:
			callback.call(*argv)
		else:
			#没注册调用函数就调用了这个类型 
			print "GE_EXC, LocalCallbacks(%s) has not type(%s) callback." % (self.key, funtype)
