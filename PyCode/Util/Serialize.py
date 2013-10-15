#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# Python对象序列化和反序列化
#===============================================================================
import cPickle
import datetime
datetime

string_to_obj = cPickle.loads

def obj_to_string(o):
	return cPickle.dumps(o, 1)
