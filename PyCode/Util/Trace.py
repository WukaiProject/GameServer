#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 警告
#===============================================================================
import inspect

def stack_warn(warn = ""):
	print "Traceback(show stack):"
	frame_list = inspect.stack()
	for idx in xrange(len(frame_list) - 1, 0, -1):
		frame = frame_list[idx]
		print '  File "%s", line %s, in %s' % (frame[1], frame[2], frame[3])
		source =  frame[4]
		if source:
			source = source[0]
			for start in xrange(len(source)):
				if source[start] != '\t' and source[start] != ' ':
					source = source[start:]
					break
			print '    %s' % source,
	print "StackError:%s" % warn
