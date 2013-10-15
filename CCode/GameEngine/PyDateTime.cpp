/*时间的Python接口模块*/
#include "GEPython.h"
#include "GEDateTime.h"

//////////////////////////////////////////////////////////////////////////
// PyDateTime模块
//////////////////////////////////////////////////////////////////////////
namespace GEPython
{
	PyObject* Now( PyObject* self, PyObject* args )
	{
		return GEDateTime::Instance()->Now().GetObj_NewRef();
	}

	PyObject* WeekDay( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI32(GEDateTime::Instance()->WeekDay());
	}

	PyObject* Year( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI32(GEDateTime::Instance()->Year());
	}

	PyObject* Month( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI32(GEDateTime::Instance()->Month());
	}

	PyObject* Day( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI32(GEDateTime::Instance()->Day());
	}

	PyObject* Hour( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI32(GEDateTime::Instance()->Hour());
	}

	PyObject* Minute( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI32(GEDateTime::Instance()->Minute());
	}

	PyObject* YearDay( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI32(GEDateTime::Instance()->YeayDay());
	}

	PyObject* Minutes( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI32(GEDateTime::Instance()->Minutes());
	}

	PyObject* Seconds( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI32(GEDateTime::Instance()->Seconds());
	}

	PyObject* Days( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI32(GEDateTime::Instance()->Days());
	}

	PyObject* Mseconds( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI64(GEDateTime::Instance()->MSeconds());
	}

	PyObject* TimeZoneSeconds( PyObject* self, PyObject* args )
	{
		return GEPython::PyObjFromUI32(GEDateTime::Instance()->TimeZoneSeconds());
	}

	PyObject* SetUnixTime( PyObject* self, PyObject* arg )
	{
		GE::Uint32 uUnixTime = 0;
		if (!GEPython::PyObjToUI32(arg, uUnixTime))
		{
			PY_PARAM_ERROR("UTCTime must be GE::Uint32.")
		}
		GEDateTime::Instance()->SetUnixTime(uUnixTime);
		Py_RETURN_NONE;
	}

	// PyDateTime_Methods[]
	static PyMethodDef PyDateTime_Methods[] = {
		{ "Now", Now, METH_NOARGS, "当前的本地时间" },
		{ "WeekDay", WeekDay, METH_NOARGS, "星期几（0，星期天；1，星期1 ...）" },
		{ "Year", Year, METH_NOARGS, "年（年份，如2011" },
		{ "Month", Month, METH_NOARGS, "月（月份，1 -- 12" },
		{ "Day", Day, METH_NOARGS, "日（日期，1 -- 31" },
		{ "Hour", Hour, METH_NOARGS, "时（小时，0 -- 23" },
		{ "Minute", Minute, METH_NOARGS, "分（分钟，0 -- 59" },
		{ "YearDay", YearDay, METH_NOARGS, "今年的第几天" },
		{ "Minutes", Minutes, METH_NOARGS, "从1970元到现在的分钟数" },
		{ "Seconds", Seconds, METH_NOARGS, "从1970元到现在的秒数" },
		{ "Days", Days, METH_NOARGS , "从1970元到现在的天数" },
		{ "Mseconds", Mseconds, METH_NOARGS , "进程启动到现在的毫秒数" },
		{ "TimeZoneSeconds", TimeZoneSeconds, METH_NOARGS , "服务端当前进程所在计算机的时区" },
		{ "SetUnixTime", SetUnixTime, METH_O , "设置当前时间" },
		{ NULL } // END_FLAG
	};

	// PyDateTime_Init
	void PyDateTime_Init( void )
	{
		// 断言下，必须Python虚拟机初始化了才行
		GE_ERROR(Py_IsInitialized());
		PyObject* pDatetime = Py_InitModule("cDateTime", PyDateTime_Methods);
		if (NULL == pDatetime)
		{
			PyErr_Print();
		}
	}
}

