#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# XRLAM("Config.Table_Global")
#===============================================================================
# 全局表
#===============================================================================
# 构建工作环境
import os
import sys
path = os.path.dirname(os.path.realpath(__file__))
path = path[:path.find("PyCode") + 6]
if path not in sys.path: sys.path.append(path)
path = path.replace("PyCode", "PyHelp")
if path not in sys.path: sys.path.append(path)

# 真正导入
import Env
from Config import Base
from Util.MySQL import Table, Column

def build_all_table():
	Table.MySQL.connect(*Base.GLOBAL_MYSQL)
	Table.MySQL.use_database("global_%s" % Env.Operators)
	for value in globals().itervalues():
		if not isinstance(value, Table.Table):
			continue
		value.Build()
	Table.MySQL.disconnect()

if Env.Windows:
	message = Table.Table("message", 1024)
	message.AddColumn(Column.IntColumn("msg_id", "smallint", True, False, "消息ID"))
	message.AddColumn(Column.StringColumn("msg_name", 60, "消息名"))
	message.AddColumn(Column.StringColumn("msg_introduct", 60, "消息说明"))
	message.AddKey(Column.ToPRI("msg_id"))
	message.AddKey(Column.ToUNI("msg_name"))
	
	transaction = Table.Table("transaction", 1024)
	transaction.AddColumn(Column.IntColumn("tra_id", "smallint", True, False, "事务ID"))
	transaction.AddColumn(Column.StringColumn("tra_name", 60, "事务名"))
	transaction.AddColumn(Column.StringColumn("tra_introduct", 60, "事务说明"))
	transaction.AddKey(Column.ToPRI("tra_id"))
	transaction.AddKey(Column.ToUNI("tra_name"))

computer = Table.Table("computer")
computer.AddColumn(Column.StringColumn("computer_name", 60, "计算机名"))
computer.AddColumn(Column.StringColumn("computer_ip", 60, "网卡IP"))
computer.AddKey(Column.ToPRI("computer_name"))
computer.AddKey(Column.ToUNI("computer_ip"))

mysql = Table.Table("mysql")
mysql.AddColumn(Column.StringColumn("mysql_name", 60, "MySQL名"))
mysql.AddColumn(Column.StringColumn("mysql_ip", 60, "连接IP"))
mysql.AddColumn(Column.IntColumn("mysql_port", "smallint", True, False, "连接端口"))
mysql.AddColumn(Column.StringColumn("mysql_user", 60, "连接用户名"))
mysql.AddColumn(Column.StringColumn("mysql_pwd", 60, "连接密码"))
mysql.AddKey(Column.ToPRI("mysql_name"))

zone = Table.Table("zone")
zone.AddColumn(Column.IntColumn("zone_id", "smallint", True, False, "区ID"))
zone.AddColumn(Column.StringColumn("zone_name", 60, "区名"))
zone.AddColumn(Column.StringColumn("zone_type", 60, "区类型"))
zone.AddColumn(Column.StringColumn("zone_language", 60, "区语言"))
zone.AddColumn(Column.ObjColumn("zone_merge", "合区信息"))
zone.AddColumn(computer.GetColumn("computer_name"))
zone.AddColumn(mysql.GetColumn("mysql_name"))
zone.AddColumn(Column.StringColumn("public_ip", 60, "公网IP"))
zone.AddColumn(Column.IntColumn("public_port", "smallint", True, False, "公网端口"))
zone.AddColumn(Column.ObjColumn("process_info", "进程信息[进程KEY, ...]"))
zone.AddColumn(Column.IntColumn("merged_zone_id", "smallint", True, False, "被合到区的ID"))
zone.AddColumn(Column.ObjColumn("merge_zone_info", "合区ID信息(zone_id --> merge datetime)"))
zone.AddKey(Column.ToPRI("zone_id"))

process = Table.Table("process")
process.AddColumn(Column.StringColumn("process_key", 60, "进程Key(进程类型_进程ID)"))
process.AddColumn(Column.StringColumn("process_type", 60, "进程类型"))
process.AddColumn(Column.IntColumn("process_id", "smallint", True, False, "进程ID"))
process.AddColumn(Column.IntColumn("process_port", "smallint", True, False, "进程监听端口"))
process.AddColumn(zone.GetColumn("zone_id"))

computer_tmp = computer.Clone("computer_tmp")
mysql_tmp = mysql.Clone("mysql_tmp")
zone_tmp = zone.Clone("zone_tmp")
process_tmp = process.Clone("process_tmp")

if __name__ == "__main__":
	build_all_table()
