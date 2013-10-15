#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# URL辅助模块
#===============================================================================
import urllib

def url_decode(query):
	'''
	将URL解析出key-value
	@param query:
	'''
	d = {}
	a = query.split('&')
	for s in a:
		if s.find('='):
			k,v = map(urllib.unquote, s.split('='))
			try:
				d[k].append(v)
			except KeyError:
				d[k] = [v]
	return d

# 将key-value进行URL编码
url_encode = urllib.urlencode
