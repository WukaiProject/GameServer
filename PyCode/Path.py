#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 系统文件路径模块
#===============================================================================
import os

class Folder(object):
	def __init__(self, exists_folder):
		# 断言目录存在
		assert os.path.exists(exists_folder)
		assert os.path.isdir(exists_folder)
		# 修正为标准目录
		if not exists_folder.endswith(os.sep):
			exists_folder += os.sep
		# 保存之
		self.folder = exists_folder
	
	def append(self, folder_name):
		if isinstance(folder_name, (tuple, list)):
			for _name in folder_name:
				assert isinstance(_name, str)
				self.cd(folder_name)
		elif isinstance(folder_name, str):
			self.folder = self.folder + folder_name + os.sep
			# 确保目录存在
			if os.path.exists(self.folder):
				if os.path.isdir(self.folder):
					return self
				else:
					raise Exception("%s is not folder" % self.folder)
			else:
				# 多进程下的安全创建目录
				try:
					os.mkdir(self.folder)
				except:
					# 如果创建失败，有肯能是多进程同时启动的情况下被其他进程创建了
					# 断言此时必定有这个目录存在
					assert os.path.exists(self.folder) and os.path.isdir(self.folder)
				return self
	
	def get_folder_path(self):
		return self.folder
	
	def get_file_path(self, file_name):
		return self.folder + file_name

#===============================================================================
# 一些固定目录的定义
#===============================================================================
# 当前目录
CURRENT_FOLDER_PATH = os.path.dirname(os.path.realpath(__file__)) + os.sep
# 根目录
ROOT_FOLDER_PATH = CURRENT_FOLDER_PATH[:-7]
# C代码目录
CCODE_FOLDER_PATH = ROOT_FOLDER_PATH + "CCode" + os.sep
# Python代码目录
PYCODE_FOLDER_PATH = ROOT_FOLDER_PATH + "PyCode" + os.sep
# 系统脚本（批处理、shell脚本）目录
SCRIPT_FOLDER_PATH = ROOT_FOLDER_PATH + "Script" + os.sep
# 配置文件目录
CONFIG_FOLDER_PATH = ROOT_FOLDER_PATH + "Config" + os.sep
# Log目录
LOG_FOLDER_PATH = ROOT_FOLDER_PATH + "Log" + os.sep
# Bin目录
BIN_FOLDER_PATH = ROOT_FOLDER_PATH + "Bin" + os.sep


if __name__ == "__main__":
	print "CURRENT_FOLDER_PATH", CURRENT_FOLDER_PATH
	print "ROOT_FOLDER_PATH", ROOT_FOLDER_PATH
	print "CCODE_FOLDER_PATH", CCODE_FOLDER_PATH
	print "PYCODE_FOLDER_PATH", PYCODE_FOLDER_PATH
	print "SCRIPT_FOLDER_PATH", SCRIPT_FOLDER_PATH
	print "CONFIG_FOLDER_PATH", CONFIG_FOLDER_PATH
	print "LOG_FOLDER_PATH", LOG_FOLDER_PATH
	print "BIN_FOLDER_PATH", BIN_FOLDER_PATH
