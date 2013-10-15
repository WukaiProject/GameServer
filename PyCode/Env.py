#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 脚本环境
#===============================================================================
import platform

# 是否Windows平台
Windows = True
# 当前机器网卡IP
IP = "127.0.0.1"
# 服务进程所属的运营商
Operators = ""
# 服务进程的功能集合
Plugs = set()

# 构建相关环境
if "_HasLoad" not in dir():
	if platform.system() == "Windows":
		Windows = True
	else:
		Windows = False
	# IP
	if Windows:
		import socket
		IP = socket.gethostbyname(socket.gethostname())
	else:
		import os
		ips = os.popen("/sbin/ifconfig | grep 'inet addr' | awk '{print $2}'").read()
		for add in ips.split('\n'):
			if not add:
				continue
			ip = add[5:]
			if ip == "127.0.0.1":
				continue
			IP = ip
			break
		else:
			assert False
	# 运营商
	f = open(__file__[:__file__.find("Env")] + "Env.txt")
	Operators = f.read()
	f.close()
	if Operators.endswith("\n"):
		Operators = Operators[:-1]
	if Operators.endswith("\t"):
		Operators = Operators[:-1]


if __name__ == "__main__":
	print "Windows", Windows
	print "IP", IP
	print "Operators", Operators
	print "Plugs", Plugs
