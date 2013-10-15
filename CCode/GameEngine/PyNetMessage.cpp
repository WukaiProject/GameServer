/*引擎进程模块的Python接口*/
#include "GEPython.h"
#include "GENetMessage.h"

//////////////////////////////////////////////////////////////////////////
// PyNetMessage模块
//////////////////////////////////////////////////////////////////////////
namespace GEPython
{
	PyObject* PackPyMsg(PyObject* self, PyObject* arg)
	{
		GE::Uint16 uMsgType = 0;
		PyObject* pyObj = Py_None;
		if (!PyArg_ParseTuple(arg, "HO", &uMsgType, &pyObj))
		{
			return NULL;
		}
		PackMessage::Instance()->Reset();
		PackMessage::Instance()->PackMsg(uMsgType);
		PackMessage::Instance()->PackPyObj(pyObj);
		Py_RETURN_NONE;
	}

	PyObject* MsgToString(PyObject* self, PyObject* arg)
	{
		/*
		Return value: New reference.
		Return a new string object with a copy of the string v as value and length len on success, and NULL on failure.
		If v is NULL, the contents of the string are uninitialized.
		*/
		return PyString_FromStringAndSize(static_cast<const char*>(PackMessage::Instance()->GetHead()), PackMessage::Instance()->GetSize());
	}

	PyObject* StringToMsg(PyObject* self, PyObject* arg)
	{
		if (!PyString_CheckExact(arg))
		{
			PY_PARAM_ERROR("param must be string.");
		}
		const void* pHead = static_cast<const void*>(PyString_AS_STRING(arg));
		GE::Uint16 uSize = static_cast<GE::Uint16>(PyString_GET_SIZE(arg));
		if (!PackMessage::Instance()->Assign(pHead, uSize))
		{
			PY_PARAM_ERROR("string is too long.");
		}
		Py_RETURN_NONE;
	}

	// PyNetMessage_Methods[]
	static PyMethodDef PyNetMessage_Methods[] = {
		{ "PackPyMsg", PackPyMsg, METH_VARARGS, "预打包一个消息" },
		{ "MsgToString", MsgToString, METH_NOARGS, "消息序列化" },
		{ "StringToMsg", StringToMsg, METH_O, "消息反序列化" },
		{ NULL } // END_FLAG
	};

	// PyNetMessage_Init
	void PyNetMessage_Init( void )
	{
		// 断言下，必须Python虚拟机初始化了才行
		GE_ERROR(Py_IsInitialized());
		PyObject* pNetMessage = Py_InitModule("cNetMessage", PyNetMessage_Methods);
		if (NULL == pNetMessage)
		{
			PyErr_Print();
		}
	}
}

