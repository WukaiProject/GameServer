/*引擎进程模块的Python接口*/
#include "GEPython.h"
#include "GEProcess.h"
#include "GEIO.h"
#include "GEGUID64.h"
#include "GENetMessage.h"

#define BLUE_FLAG "BLUE"
#define RED_FLAG "RED"
#define YELLOW_FLAG "YELLOW"
#define GREEN_FLAG "GREEN"
#define SELECT_FLAG "SELECT"
#define INSERT_FLAG "INSERT"
#define CREATE_FLAG "CREATE"
#define ALTER_FLAG "ALTER"
#define DROP_FLAG "DROP"


//////////////////////////////////////////////////////////////////////////
// PyProcess模块
//////////////////////////////////////////////////////////////////////////
namespace GEPython
{
	PyObject* write( PyObject* self, PyObject* arg )
	{
		static GE::Uint16 ES = static_cast<GE::Uint16>(sizeof(EXCEPT_FLAG) - 1);
		static GE::Uint16 TS = static_cast<GE::Uint16>(sizeof(TRACEBACK_FLAG) - 1);

		if (!PyString_CheckExact(arg))
		{
			PY_PARAM_ERROR("param must be string.");
		}
		// 解析出字符串
		char * sz = PyString_AS_STRING(arg);
		GE::Uint16 uSize = static_cast<GE::Uint16>(PyObject_Length(arg));
		// 查找字符串是否以相关标识开头
		bool bflag = false;
		if (uSize >= ES && 0 == memcmp(sz, EXCEPT_FLAG, ES))
		{
			bflag = true;	
		}
		if (false == bflag && uSize >= TS && 0 == memcmp(sz, TRACEBACK_FLAG, TS))
		{
			bflag = true;
		}
		// 如果有特殊标志，则分情况处理之
		if (bflag)
		{
	#ifdef WIN
			GE_EXC<<sz;
	#elif LINUX
			GE_OUT<<GEDateTime::Instance()->Hour()<<":"<<GEDateTime::Instance()->Minute()<<":"<<GEDateTime::Instance()->Second()<<" "<<sz;
	#endif
		}
		// 没有特殊标志，普通打印之
		else
		{
	#ifdef WIN
			if (0 == memcmp(sz, BLUE_FLAG, sizeof(BLUE_FLAG) - 1))
			{
				GE_OUT<<Blue<<sz;
			}
			else if(0 == memcmp(sz, RED_FLAG, sizeof(RED_FLAG) - 1))
			{
				GE_OUT<<Red<<sz;
			}
			else if(0 == memcmp(sz, YELLOW_FLAG, sizeof(YELLOW_FLAG) - 1))
			{
				GE_OUT<<Yellow<<sz;
			}
			else if(0 == memcmp(sz, GREEN_FLAG, sizeof(GREEN_FLAG) - 1))
			{
				GE_OUT<<Green<<sz;
			}
			// MySQL语句的标识
			else if (0 == memcmp(sz, SELECT_FLAG, sizeof(SELECT_FLAG) - 1))
			{
				GE_OUT<<Blue<<sz;
			}
			else if (0 == memcmp(sz, INSERT_FLAG, sizeof(INSERT_FLAG) - 1))
			{
				GE_OUT<<Blue<<sz;
			}
			else if (0 == memcmp(sz, CREATE_FLAG, sizeof(CREATE_FLAG) - 1))
			{
				GE_OUT<<Blue<<sz;
			}
			else if (0 == memcmp(sz, ALTER_FLAG, sizeof(ALTER_FLAG) - 1))
			{
				GE_OUT<<Blue<<sz;
			}
			else if (0 == memcmp(sz, DROP_FLAG, sizeof(DROP_FLAG) - 1))
			{
				GE_OUT<<Blue<<sz;
			}
			else
			{
				GE_OUT<<sz;
			}
	#elif LINUX
			GE_OUT<<sz;
	#endif
		
		}
		// 强制刷新
		std::cout.flush();
		Py_RETURN_NONE;
	}

	// 重定向C++的输出和错误流
	PyObject* RedirectOutAndErrBuf( PyObject* self, PyObject* arg )
	{
		// 解析出输出文件路径
		char* sFilePath = NULL;
		if (!PyString_CheckExact(arg))
		{
			PY_PARAM_ERROR("file path must a string!");
		}
		sFilePath = PyString_AS_STRING(arg);
		// 重定向之
		RedirectOutAndErrStream(sFilePath);
		Py_RETURN_NONE;
	}

	PyObject* AllotGUID64( PyObject* self, PyObject* arg )
	{
		return GEPython::PyObjFromUI64(GEGUID64::Instance()->AllotGUID());
	}

	PyObject* PackB8( PyObject* self, PyObject* args)
	{
		GE::Uint32 uI32_0 = 0;
		GE::Uint32 uI32_1 = 0;
		if (!PyArg_ParseTuple(args, "II", &uI32_1, &uI32_0))
		{
			return NULL;
		}
		GE::B8 b8(uI32_0, uI32_1);
		return GEPython::PyObjFromUI64(b8.UI64());
	}

	PyObject* UnpackB8( PyObject* self, PyObject* arg)
	{
		GE::Uint64 ui64 = 0;
		if (!GEPython::PyObjToUI64(arg, ui64))
		{
			PY_PARAM_ERROR("param must be GE::Uint64.");
		}
		GE::B8& b8 = GE_AS_B8(ui64);
		return Py_BuildValue("II", b8.UI32_1(), b8.UI32_0());
	}

	PyObject* PackB4( PyObject* self, PyObject* args)
	{
		GE::Uint16 uI16_0 = 0;
		GE::Uint16 uI16_1 = 0;
		if (!PyArg_ParseTuple(args, "HH", &uI16_1, &uI16_0))
		{
			return NULL;
		}
		GE::B4 b4(uI16_0, uI16_1);
		return GEPython::PyObjFromUI32(b4.UI32());
	}

	PyObject* UnpackB4( PyObject* self, PyObject* arg)
	{
		GE::Uint32 ui32 = 0;
		if (!GEPython::PyObjToUI32(arg, ui32))
		{
			PY_PARAM_ERROR("param must be GE::B4.");
		}
		GE::B4& b4 = GE_AS_B4(ui32);
		return Py_BuildValue("HH", b4.UI16_1(), b4.UI16_0());
	}

	PyObject* UnpackB4ToUI8(PyObject* self, PyObject* arg)
	{
		GE::Uint32 ui32 = 0;
		if (!GEPython::PyObjToUI32(arg, ui32))
		{
			PY_PARAM_ERROR("param must be GE::B4.");
		}
		GE::B4& b4 = GE_AS_B4(ui32);
		return Py_BuildValue("BBBB", b4.UI8_3(), b4.UI8_2(), b4.UI8_1(), b4.UI8_0());
	}

	PyObject* PyObjPackSize(PyObject* self, PyObject* arg)
	{
		GE::Int32 Size = PackMessage::Instance()->PyObjSize(arg, MAX_UINT16);
		return GEPython::PyObjFromI32(Size);
	}

	// PyProcess_Methods[]
	static PyMethodDef PyProcess_Methods[] = {
		{ "write", write, METH_O, "重定向Python输出" },
		{ "RedirectOutBuf", RedirectOutAndErrBuf, METH_O, "重定向进程输出和错误流" },
		{ "AllotGUID64", AllotGUID64, METH_NOARGS, "分配一个全球唯一ID" },
		{ "PackB8", PackB8, METH_VARARGS, "将俩个B4打包成为B8，第一个参数是高位B4，第二个参数是低位B4" },
		{ "UnpackB8", UnpackB8, METH_O, "将B8解包成两个B4，第一个结果是高位B4，第二个结果是低位B4" },
		{ "PackB4", PackB4, METH_VARARGS, "将俩个Uint16打包成为B4，第一个参数是高位Uint16，第二个参数是低位Uint16" },
		{ "UnpackB4", UnpackB4, METH_O, "将B4解包成两个Uint16，第一个结果是高位Uint16，第二个结果是低位Uint16" },
		{ "UnpackB4ToUI8", UnpackB4ToUI8, METH_O, "将UI32解包成四个Uint8"},
		{ "PyObjPackSize", PyObjPackSize, METH_O, "计算Python对象打包长度" },
		{ NULL } // END_FLAG
	};

	// PyProcess_Init
	void PyProcess_Init( void )
	{
		// 断言下，必须Python虚拟机初始化了才行
		GE_ERROR(Py_IsInitialized());
		PyObject* pProcess = Py_InitModule("cProcess", PyProcess_Methods);
		if (NULL == pProcess)
		{
			PyErr_Print();
		}
		PyModule_AddObject(pProcess, "ProcessType", PyString_FromString(GEProcess::Instance()->ProcessType().c_str()));
		PyModule_AddIntConstant(pProcess, "ProcessID", GEProcess::Instance()->ProcessID());
		PyModule_AddIntConstant(pProcess, "ListenPort", GEProcess::Instance()->LisPort());
		if (GEProcess::Instance()->IsWin())
		{
			Py_INCREF(Py_True);
			PyModule_AddObject(pProcess, "IsWin", Py_True);
		}
		else
		{
			Py_INCREF(Py_False);
			PyModule_AddObject(pProcess, "IsWin", Py_False);
		}
		if (GEProcess::Instance()->IsDebug())
		{
			Py_INCREF(Py_True);
			PyModule_AddObject(pProcess, "IsDebug", Py_True);
		}
		else
		{
			Py_INCREF(Py_False);
			PyModule_AddObject(pProcess, "IsDebug", Py_False);
		}
	}
}

