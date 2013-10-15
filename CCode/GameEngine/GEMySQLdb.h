/************************************************************************
提出MySQLdb的部分头文件，用于拓展MySQLdb
************************************************************************/
#pragma once

#include "pymemcompat.h"
#include "structmember.h"
#if defined(MS_WINDOWS)
#include <winsock2.h>
#include <windows.h>
#include <config-win.h>
#else
#include "my_config.h"
#endif
#include "mysql.h"
#include "mysqld_error.h"
#include "errmsg.h"

typedef struct {
	PyObject_HEAD
	MYSQL connection;
	int open;
	PyObject *converter;
} _mysql_ConnectionObject;

void init_mysql(void);
_mysql_ConnectionObject*			Py_mysql_ConnectionObject_FromObj(PyObject* pObj);

