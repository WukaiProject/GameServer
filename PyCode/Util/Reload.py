#!/usr/bin/env python
# -*- coding:UTF-8 -*-
#===============================================================================
# 模块重载
# 1.此模块中的reload是使用旧类和函数对象，函数的逻辑对象是用新的
# 2.这样做的好处是类和函数可以被第3方保存，reload模块后第3方的逻辑也同时改变
# 3.对于非类和函数对象，第3方的使用由模块开始引用之，能够使得reload模块后是使用新的对象
# 4.注意，使用本模块中的reload函数，要求被reload的模块在reload时执行的的模块级代码中
#    不能使用被reload模块中定义的逻辑对象
#===============================================================================
import types

def xreload(module, instead_data = False):
	'''
	重载某个模块
	@param module:模块对象
	@param instead_data:是否替换数据。False：只重载逻辑对象（类、函数）； True：重载逻辑和其他对象
	'''
	# 记录模块重载之前的所有顶层对象
	old_module = {}
	for name in dir(module):
		old_module[name] = getattr(module, name)
	
	# 重载模块
	new_module = reload(module)
	assert( id(module) == id(new_module))
	
	for name in dir(new_module):
		# 如果新的对象名不在旧模块中，不必处理
		if name not in old_module:
			continue
		
		old_obj = old_module[name]
		new_obj = getattr(new_module, name)
		
		# 如果新对象的类型和旧对象的类型不同警告之
		if type(old_obj) != type(new_obj):
			print "GE_EXC, after reload module(%s), obj(%s)'s old type(%s) != new type(%s)" % (module.__name__, name, str(type(old_obj)), str(type(new_obj)))
			# 如果不替换数据，则使用旧对象
			if not instead_data:
				setattr(new_module, name, old_obj)
			continue
		
		# 尝试更新逻辑对象
		if update(old_obj, new_obj, instead_data):
			# 用更新后的旧逻辑替换reload后的新逻辑，保证函数指针不变
			setattr(new_module, name, old_obj)
		else:
			# 如果不替换数据, 非逻辑对象使用旧的对象
			if not instead_data:
				setattr(new_module, name, old_obj)

def update_function(oldFun, newFun):
	oldFun.func_code = newFun.func_code
	oldFun.func_defaults = newFun.func_defaults
	oldFun.func_doc = newFun.func_doc

def update_unbound_method_type(oldMethod, newMethod):
	update_function(oldMethod.im_func, newMethod.im_func)

def update_class(oldClass, newClass, instead_data):
	for name in newClass.__dict__.iterkeys():
		# 类实例的__dict__属性不可替换
		if name in ("__dict__", "__doc__"):
			continue
		
		new_obj = getattr(newClass, name)
		old_obj = getattr(oldClass, name, None)
		# 如果新的对象不在旧的对象中，添加之
		if old_obj is None:
			# 如果是类函数，要重新构建
			if type(new_obj) == types.UnboundMethodType:
				new_obj = types.UnboundMethodType(new_obj.im__func, None, oldClass)
			setattr(oldClass, name, new_obj)
			continue
		
		# 如果新的对象和旧的对象的类型不同，警告之
		if type(old_obj) != type(new_obj):
			print "GE_EXC, after reload module(%s),class(%s)'s obj(%s)'s old type(%s) != new type(%s)" % (newClass.__name__, newClass.__name__, name, str(type(old_obj)), str(type(new_obj)))
			# 如果要替换数据，则用新的对象替换旧的对象
			if instead_data:
				setattr(oldClass, name, new_obj)
			continue
		
		# 尝试更新逻辑对象
		if update(old_obj, new_obj, instead_data):
			pass
		else:
			# 如果要替换非逻辑对象，则用新的对象替换旧的对象
			if instead_data:
				setattr(oldClass, name, new_obj)

def update(old_obj, new_obj, instead_data):
	old_objType = type(old_obj)
	if old_objType == types.FunctionType:
		update_function(old_obj, new_obj)
	elif old_objType in (types.TypeType, types.ClassType):
		update_class(old_obj, new_obj, instead_data)
	elif old_objType == types.UnboundMethodType:
		update_unbound_method_type(old_obj, new_obj)
	else:
		return False
	return True
