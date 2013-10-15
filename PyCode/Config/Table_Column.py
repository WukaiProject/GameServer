#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# XRLAM("Config.Table_Column")
#===============================================================================
# 公共列
#===============================================================================
# 构建工作环境
import os
import sys
path = os.path.dirname(os.path.realpath(__file__))
path = path[:path.find("PyCode") + 6]
if path not in sys.path: sys.path.append(path)
path = path.replace("PyCode", "PyHelp")
if path not in sys.path: sys.path.append(path)

#真正导入
from Util.MySQL import Column
Column

