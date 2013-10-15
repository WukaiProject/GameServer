#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# XRLAM("Config.Help")
#===============================================================================
# 配置的辅助模块
#===============================================================================
import os
import Env
import Path
from Common import Int
from Util import TabFile
from Config import Base

#===============================================================================
# 常用的获取数据库连接的函数簇
#===============================================================================
def roleid_to_zoneid(role_id):
	return role_id / Int.P2_32

def connect_zone_info(zone_id):
	con = connect_global()
	with con as cur:
		cur.execute("select mysql_ip, mysql_port, mysql_user, mysql_pwd from (select mysql_name from zone where zone_id = %s) as s1 left join mysql using(mysql_name);" % zone_id)
		result = cur.fetchall()
		if result:
			mysql_ip, mysql_port, mysql_user, mysql_pwd = result[0]
			db = "zone_%s" % zone_id
			return mysql_ip, mysql_port, mysql_user, mysql_pwd, db
		else:
			return None

def connect_mysql(ip, port, user, pwd, db):
	import MySQLdb
	return MySQLdb.connect(host = ip, port = port, user = user, passwd = pwd, db = db, charset = "utf8", use_unicode = False)

def connect_global():
	ip, port, user, pwd = Base.GLOBAL_MYSQL
	db = "global_%s" % Env.Operators
	return connect_mysql(ip, port, user, pwd, db)

def connect_houtai():
	ip, port, user, pwd = Base.HOUTAI_MYSQL
	db = "houtai_%s" % Env.Operators
	return connect_mysql(ip, port, user, pwd, db)

def connect_zone(zone_id):
	return connect_mysql(*connect_zone_info(zone_id))

def connect_role(role_id):
	return connect_zone(roleid_to_zoneid(role_id))

#===============================================================================
# 配置表读取器
#===============================================================================
class TabRead(TabFile.TabLine):
	'''
	以Tab为分隔符的文本表的读取器。
	Paths 是以工程的Config目录为根目录的路径列表。
	Plugs 是一个对功能模块的需求集合，只有当前进程具有Plugs中的某个功能才能够读取数据。
	'''
	Paths = []
	Plugs = set()
	
	@classmethod
	def read_as_list(cls):
		file_path = Path.CONFIG_FOLDER_PATH + os.sep.join(cls.Paths)
		if cls.Plugs & Env.Plugs:
			print "GE_EXC, read(%s) with out functions(%s)." % (file_path, " ".join(cls.Plugs))
			return []
		tabfile = TabFile.TabFile(cls.get_file_path())
		return tabfile.to_class_type(cls)
	
	@classmethod
	def read_as_dict(cls, name):
		file_path = Path.CONFIG_FOLDER_PATH + os.sep.join(cls.Paths)
		if cls.Plugs & Env.Plugs:
			print "GE_EXC, read(%s) with out functions(%s)." % (file_path, " ".join(cls.Plugs))
			return {}
		d = {}
		file_path = cls.get_file_path()
		tabfile = TabFile.TabFile(cls.get_file_path())
		for l in tabfile.to_class_type(cls):
			key = getattr(l, name)
			if key in d:
				print "GE_EXC, read(%s) find repeat key(%s)." % (file_path, key)
			d[key] = l
		return d

#===============================================================================
# ID分配器
#===============================================================================
class DBAllotMgr(object):
	'''
	依赖数据库的自动ID分配器，用于自动分配唯一ID
	'''
	def __init__(self, table_name, column_id, column_name, column_introduct):
		self.table_name = table_name
		self.column_id = column_id
		self.column_name = column_name
		self.column_introduct = column_introduct
		self.allot = {}
		self.names = set()
		# 如果是Windows下的逻辑环境，则读取数据库中的所有数据
		if Env.Windows and "logic" in Env.Plugs:
			self.max_id = 1024
			self.con = connect_global()
			# 这里要开启大事务
			self.con.autocommit(False)
			self.cur = self.con.cursor()
			self.cur.execute("select %s, %s, %s from %s for update;" % (self.column_id, self.column_name, self.column_introduct, self.table_name))
			for row in self.cur.fetchall():
				self.allot[row[1]] = (row[0], row[2])
				if row[0] > self.max_id:
					self.max_id = row[0]
		# 否则，需要读取文件
		else:
			tabfile = TabFile.TabFile("%s%s.txt" % (Path.CONFIG_FOLDER_PATH, self.table_name))
			for row in tabfile.data_list:
				self.allot[row[1]] = (row[0], row[2])
	
	def allot(self, name, instroduct):
		# 名字唯一
		assert name not in self.names
		self.names.add(name)
		# 如果是Windows下的逻辑环境，则需要和数据库互动来自动分配ID
		if Env.Windows and "logic" in Env.Plugs:
			info = self.allot.get(name)
			# 不存在，则需要分配之
			if info is None:
				self.max_id += 1
				self.cur.execute("insert into %s (%s, %s, %s) " % (self.table_name, self.column_id, self.column_name, self.column_introduct) + " values(%s, %s, %s);", (self.max_id, name, instroduct))
				return self.max_id
			# 说明有改动，则更新说明
			if info[1] != instroduct:
				self.cur.execute("update %s set %s = %s where %s = %s;" % (self.table_name, self.column_introduct, "%s", self.column_name, "%s"), (instroduct, name))
			return info[0]
		# 否则，只能返回文件中的ID
		else:
			return self.allot[name][0]
	
	def tidy(self):
		# 必须在Windows环境中
		assert Env.Windows and "logic" in Env.Plugs
		for name in self.allot.iterkeys():
			if name in self.names:
				continue
			# 多余的项删除
			self.cur.execute("delete from %s where %s = %s;" % (self.table_name, self.column_name, "%s"), name)
	
	def write(self):
		# 必须在Windows环境中
		assert Env.Windows and "logic" in Env.Plugs
		with open("%s%s.txt" % (Path.CONFIG_FOLDER_PATH, self.table_name), "w") as f:
			for column_name, (column_id, column_instroduct) in self.allot.iteritems():
				f.write("%s\t%s\t%s\n" % (column_id, column_name, column_instroduct))
	
	def final(self):
		# 如果是Windows下的逻辑环境，需要清理数据库连接
		if Env.Windows and "logic" in Env.Plugs:
			self.cur.close()
			self.con.commit()
			self.con.close()
