#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# XRLAM("Config.Zone")
#===============================================================================
# 区配置
#===============================================================================
from Config import Help, Table_Global

def __copy_table(cur, from_table, to_table):
	cur.execute(from_table.GetSelectAllValueSQL(""))
	result = cur.fetchall()
	cur.execute("truncate %s;" % to_table.name)
	cur.executemany(to_table.GetInsertAllValueSQL(), result)

def copy_from_tmp():
	con = Help.connect_global()
	with con as cur:
		__copy_table(cur, Table_Global.computer_tmp, Table_Global.computer)
		__copy_table(cur, Table_Global.mysql_tmp, Table_Global.mysql)
		__copy_table(cur, Table_Global.zone_tmp, Table_Global.zone)
		__copy_table(cur, Table_Global.process_tmp, Table_Global.process)
		cur.close()
	con.close()

def copy_to_tmp():
	con = Help.connect_global()
	with con as cur:
		__copy_table(cur, Table_Global.computer, Table_Global.computer_tmp)
		__copy_table(cur, Table_Global.mysql, Table_Global.mysql_tmp)
		__copy_table(cur, Table_Global.zone, Table_Global.zone_tmp)
		__copy_table(cur, Table_Global.process, Table_Global.process_tmp)
		cur.close()
	con.close()

def add_computer(computer_name, computer_ip):
	con = Help.connect_global()
	with con as cur:
		cur.execute("insert into computer_tmp (computer_name, computer_ip) values(%s, %s);", (computer_name, computer_ip))
		cur.close()
	con.close()

def add_mysql(mysql_name, mysql_ip, mysql_port, mysql_user, mysql_pwd):
	con = Help.connect_global()
	with con as cur:
		cur.execute("insert into computer_tmp (mysql_name, mysql_ip, mysql_port, mysql_user, mysql_pwd) values(%s, %s);", (computer_name, computer_ip))
		cur.close()
	con.close()
	
