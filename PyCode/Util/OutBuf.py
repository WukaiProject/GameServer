#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 输出缓存
#===============================================================================
import os
import StringIO

class OutBuf(object):
	def __init__(self):
		self.buf = StringIO.StringIO()
	
	def __enter__(self):
		self.out = os.sys.stdout
		self.err = os.sys.stderr
		os.sys.stdout = self.buf
		os.sys.stderr = self.buf
		return self
	
	def __exit__(self, _type, _value, _traceback):
		os.sys.stdout = self.out
		os.sys.stderr = self.err
		return True
	
	def get_value(self):
		return self.buf.getvalue()
	
	def pprint(self, s):
		self.out.write(s)
