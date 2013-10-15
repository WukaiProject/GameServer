#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# Python性能分析
#===============================================================================
import os
import sys
import datetime
import cProfile
import pstats

if "_HasLoad" not in dir():
	PF = cProfile.Profile()
	FunctionNames = set()
	ModuleNames = set()

def profile_start():
	PF.enable()

def profile_end():
	PF.clear()
	PF.disable()

def profile_dump(file_name = None):
	import Path
	if not file_name:
		now = datetime.datetime.now()
		file_name = "Profile_%s_%s_%s_%s_%s_%s" % (now.year, now.month, now.day, now.hour, now.minute, now.second)
	file_path = Path.LOG_FOLDER_PATH + file_name
	PF.dump_stats(file_path)
	return file_name

def profile_show(file_name):
	import Path
	PS = pstats.Stats(Path.LOG_FOLDER_PATH + file_name)
	PS.strip_dirs()
	PS.sort_stats(2)
	PS.print_stats(.3)

def profile_dump_show(file_name = None):
	file_name = profile_dump(file_name)
	profile_show(file_name)

def show_return(name):
	if name.find(".") == -1:
		FunctionNames.add(name)
	else:
		ModuleNames.add(name)
	sys.setprofile(__OnReturn)

def __OnReturn(frame, event, arg):
	if event != "return":
		return
	functionName = frame.f_code.co_name
	fp = frame.f_code.co_filename
	pos = fp.find("PyCode")
	if pos != -1:
		fp = fp[pos + 7:]
	pos = fp.find(".")
	if pos != -1:
		fp = fp[:pos]
	moduleName = fp.replace(os.sep, ".")
	if functionName in FunctionNames or moduleName in ModuleNames:
		print "BLUE %s %s %s %s" % (moduleName, functionName, frame.f_lineno, str(arg))

def clear_return():
	FunctionNames.clear()
	ModuleNames.clear()
	sys.setprofile(None)
