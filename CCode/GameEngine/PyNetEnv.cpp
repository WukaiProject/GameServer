/*引擎进程模块的Python接口*/
#include "GEPython.h"
#include "GENetEnv.h"

//////////////////////////////////////////////////////////////////////////
// PyNetEnv模块
//////////////////////////////////////////////////////////////////////////
namespace GEPython
{
	PyObject* SetCanForward(PyObject* self, PyObject* arg)
	{
		GE::Uint32 uSessionID = MAX_UINT32;
		if (!GEPython::PyObjToUI32(arg, uSessionID))
		{
			PY_PARAM_ERROR("param must be GE::Uint32.");
		}
		GENetEnv::Instance()->ForwardSessionID() = uSessionID;
		GENetEnv::Instance()->SetCanForward();
		Py_RETURN_NONE;
	}

	PyObject* SetCanCombineMsg(PyObject* self, PyObject* arg)
	{
		GENetEnv::Instance()->SetCanCombineMsg();
		Py_RETURN_NONE;
	}

	PyObject* SetTWGMaxSize(PyObject* self, PyObject* arg)
	{
		GE::Uint16 uSize = 0;
		if (!GEPython::PyObjToUI16(arg, uSize))
		{
			PY_PARAM_ERROR("param must be GE::Uint16.");
		}
		GENetEnv::Instance()->SetTGWMaxSize(uSize);
		Py_RETURN_NONE;
	}

	// PyNetEnv_Methods[]
	static PyMethodDef PyNetEnv_Methods[] = {
		{ "SetCanForward", SetCanForward, METH_O, "设置网络层消息可重定向" },
		{ "SetCanCombineMsg", SetCanCombineMsg, METH_NOARGS, "设置网络层消息可拼包" },
		{ "SetTWGMaxSize", SetTWGMaxSize, METH_NOARGS, "设置腾讯TGW的换行符数量" },
		{ NULL } // END_FLAG
	};

	// PyNetEnv_Init
	void PyNetEnv_Init( void )
	{
		// 断言下，必须Python虚拟机初始化了才行
		GE_ERROR(Py_IsInitialized());
		PyObject* pNetEnv = Py_InitModule("cNetEnv", PyNetEnv_Methods);
		if (NULL == pNetEnv)
		{
			PyErr_Print();
		}
	}
}

