#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 随机的辅助模块
#===============================================================================
import random

# 按照概率随机的类
class RandomRate(object):
	def __init__(self):
		self.random_list = []
		self.total_rate = 0
	
	def add(self, rate, item):
		'''
		增加一个随机项
		@param rate:权重
		@param item:随机元素
		'''
		self.random_list.append((rate, item))
		self.total_rate += rate
	
	def random_one(self):
		'''
		按照权重随机出一个元素
		'''
		total = self.total_rate
		if total == 0:
			return None
		ran = random.randint(0, total - 1)
		for rate, item in self.random_list:
			total -= rate
			if total <= ran:
				return item
		return None
	
	def random_many(self, cnt = 1):
		'''
		按照权重不重复随机出最多cnt个元素的列表
		'''
		# 加速优化
		RANDOM_LIST = self.random_list
		copy_len = len(RANDOM_LIST)
		copy_total = self.total_rate
		if cnt >= copy_len:
			return [item for _, item in RANDOM_LIST]
		# 结果集
		many = []
		# 多次随机
		for _ in xrange(cnt):
			# 不能随机了
			if copy_total == 0:
				break
			# 随机一个
			total = copy_total
			ran = random.randint(0, total - 1)
			for idx in xrange(copy_len):
				info = RANDOM_LIST[idx]
				rate, item = info
				total -= rate
				# 随到了
				if total <= ran:
					many.append(item)
					# 将随机到的项调换到特定位置，并减少一个查找长度和一定的随机权重
					RANDOM_LIST[idx] = RANDOM_LIST[copy_len - 1]
					RANDOM_LIST[copy_len - 1] = info
					copy_len -= 1
					copy_total -= rate
					break
		return many


if __name__ == "__main__":
	r = RandomRate()
	r.add(10, 10)
	r.add(20, 20)
	r.add(40, 40)
	r.add(80, 80)
	r.add(160, 160)
	for _ in xrange(20):
		for _ in xrange(5):
			print r.random_many(4),
		print
