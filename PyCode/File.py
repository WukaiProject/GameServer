#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 文件
#===============================================================================
import os
import sys
path = os.path.dirname(os.path.realpath(__file__))
path = path[:path.find("PyCode") + 6]
if path not in sys.path: sys.path.append(path)
path = path.replace("PyCode", "PyHelp")
if path not in sys.path: sys.path.append(path)

import md5
import Env
import Path

#===============================================================================
# 文件检测校验
#===============================================================================
def __get_all_file_path(folder, suffixs = set()):
	# 修正后缀
	if not isinstance(suffixs, set):
		suffixs = set(suffixs)
	# 获取所有的文件路径
	file_path_list = []
	for _folder, _, _file_names in os.walk(folder):
		for file_name in _file_names:
			file_path = _folder + os.sep + file_name
			pos = file_path.rfind(".")
			if pos > 0:
				suffix = file_path[pos + 1:]
				if suffix.lower() in suffixs:
					file_path_list.append(file_path[len(folder):])
	# 进行文件路径排序（解决不同操作系统文件表示方式差异）
	file_path_list.sort(key=lambda it:it.replace(os.sep, "."))
	# 返回之
	return file_path_list

def __get_all_file_md5(folder, suffixs):
	m = md5.new()
	for file_path in __get_all_file_path(folder, suffixs):
		f = open(Path.PYCODE_FOLDER_PATH + file_path)
		m.update(f.read())
		f.close()
	return str(m.hexdigest())

# 检测并修复.h和.cpp文件的最后空行的问题。
# 该问题会导致g++编译警告
def fix_ccode_file():
	for file_path in __get_all_file_path(Path.CCODE_FOLDER_PATH, ["h", "cpp"]):
		rewrite = 0
		with open(Path.CCODE_FOLDER_PATH + file_path) as f:
			file_list = f.read().split("\n")
			if file_list[-1]:
				rewrite = 2
			elif file_list[-2]:
				rewrite = 1
			if rewrite:
				for _ in xrange(rewrite):
					file_list.append("")
		if rewrite == 0:
			continue
		print "--> fix", file_path
		with open(Path.CCODE_FOLDER_PATH + file_path, "w") as f:
			f.write("\n".join(file_list))
			f.flush()

# 计算所有Python文件的md5值
def computer_pycode_md5():
	return __get_all_file_md5(Path.PYCODE_FOLDER_PATH, ["py"])

# 计算所有配置文件的md5值
def computer_config_md5():
	return __get_all_file_md5(Path.CONFIG_FOLDER_PATH, ["txt"])

# 计算所有可执行文件的md5值
def computer_bin_md5():
	if Env.Windows:
		return "nomd5"
	m = md5.new()
	f = open(Path.BIN_FOLDER_PATH + "GameServer")
	m.update(f.read())
	f.close()
	return str(m.hexdigest())

if __name__ == "__main__":
	print computer_pycode_md5()
	print computer_config_md5()
	print computer_bin_md5()
