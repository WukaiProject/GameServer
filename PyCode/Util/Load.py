#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 模块预载入
#===============================================================================
import os
import sys
import traceback

def get_all_module(root_floder, suffixs = ["py", "pyc", "pyo"]):
	'''
	获取root_floder目录下所有模块文件的信息（注意，去除了__init__模块）
	@param root_floder:根目录
	@param suffixs:文件后缀
	@return: 模块名的集合
	'''
	# 修正suffixs查找
	if not isinstance(suffixs, set):
		suffixs = set(suffixs)
	# 非模块名路径长度
	unmodule_name_path_len = len(root_floder)
	# 结果集
	result = set()
	# 遍历所有的文件信息
	for dirpath, _, filenames in os.walk(root_floder):
		# 遍历所有的文件
		for fi in filenames:
			# __init__文件，导入包
			if fi == "__init__.py":
				fpns = dirpath
			# 构造文件路径
			else:
				fp = dirpath + os.sep + fi
				# 解析文件后缀
				pos = fp.rfind('.')
				if pos == -1:
					continue
				fpns, su = fp[:pos], fp[pos + 1:]
				# 不是模块文件，忽视之
				if su not in suffixs:
					continue
			# 将无后缀的文件路径变化为模块名
			module_name = fpns[unmodule_name_path_len:].replace(os.sep, '.')
			# 加入结果集
			if module_name: result.add(module_name)
	return result

def Load_modules(module_names):
	'''
	预导入模块，并设置标识位
	@param module_names:模块名集合
	'''
	moduleList = []
	for module_name in module_names:
		try:
			# 导入模块
			__import__(module_name)
			# 获取模块对象
			module = sys.modules[module_name]
			moduleList.append(module)
			# 标记该模块被预导入
			setattr(module, "_HasLoad", None)
		except:
			traceback.print_exc()
	return moduleList

def Load_part_module(startswith = ""):
	'''
	载入一部分的模块
	@param startswith:模块名开始
	'''
	import Path
	module_names = [modulename for modulename in get_all_module(Path.PYCODE_FOLDER_PATH, ["py"]) if modulename.startswith(startswith)]
	return Load_modules(module_names)

def Load_not_part_module(startswith = ""):
	'''
	载入一部分的模块
	@param startswith:模块名开始
	'''
	import Path
	module_names = [modulename for modulename in get_all_module(Path.PYCODE_FOLDER_PATH, ["py"]) if not modulename.startswith(startswith)]
	return Load_modules(module_names)
