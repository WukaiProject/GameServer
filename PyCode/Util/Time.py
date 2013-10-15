#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 时间辅助模块
#===============================================================================
import time
import datetime

def as_datetime(argv):
	'''
	将一个参数转换为datetime对象
	@param argv:可以是时间tuple或者datetime对象
	'''
	if isinstance(argv, datetime):
		return argv
	else:
		return datetime.datetime(*argv)

def unix_time_to_datetime(unix_time):
	'''
	将一个Unix时间转换为date time
	@param unix_time:Unix时间
	'''
	tp = time.localtime(unix_time)
	print tp
	return datetime.datetime(tp[0], tp[1], tp[2], tp[3], tp[4], tp[5])

def datetime_to_unix_time(dt):
	'''
	将一个date time转换为Unix时间
	@param dt:ate time
	'''
	return int(time.mktime(dt.timetuple()))

def delta_to_string(delta):
	'''
	将一个time delta对象转换为字符串表示
	@param delta:time delta对象
	'''
	return second_to_string(delta.total_seconds())

ONE_MINUTE = 60
ONE_HOUR = 3600
ONE_DAY = 24 * 3600
def second_to_string(second):
	'''
	将一个相差的秒数转为字符串表示
	@param second:秒数
	'''
	l = []
	day, second = divmod(second, ONE_DAY)
	if day: l.append("%s天" % day)
	hour, second = divmod(second, ONE_HOUR)
	if hour: l.append("%s小时" % hour)
	minute, second = divmod(second, ONE_MINUTE)
	if minute: l.append("%s分钟" % minute)
	if second: l.append("%s秒" % second)
	return "".join(l)

def get_year_week(dt):
	'''
	一年中的第几周（以星期一为一周的开始），类型为decimal number，范围[0,53]
	在度过新年时，直到一周的全部7天都在该年中时，才计算为第0周
	只当指定了年份才有效
	@param dt:
	'''
	return int(dt.strftime("%W"))

def get_week_day(dt):
	'''
	返回改时间是这一周的第几天 [1, 7]
	星期一为1 ... 星期天为7
	@param dt:
	'''
	return dt.isoweekday()

BASE_DATETIME = datetime.datetime(2013, 1, 1)
def get_day(dt):
	'''
	将时间转换为一个连续的天数
	@param dt:
	'''
	delta = dt - BASE_DATETIME
	return delta.days
