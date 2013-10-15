#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# XRLAM("Config.Base")
#===============================================================================
# 最基本的配置
#===============================================================================
import Env

# 默认语言
DEFUAL_LANGUAGE = "chinese"
# 全局MySQL
GLOBAL_MYSQL = ("192.168.1.108", 3306, "root", "^O$11;`)_FWz")
# 全局HTTP
GLOBAL_HTTP = ("192.168.1.110", 8000)
# 后台MySQL
HOUTAI_MYSQL = ("192.168.1.108", 3306, "root", "^O$11;`)_FWz")
# 语言包种类
LANGUAGES = [DEFUAL_LANGUAGE]
# 测试区ID集合
TEST_ZONE_IDS = set(xrange(60000))

# 根据不同的运营商，修正上面的基本设置
if Env.Operators:
	pass
