/************************************************************************
安全静态（编译期确定）数组。
相比于std::vector，本安全数组支持bitcopy。
************************************************************************/
#pragma once

#include "GEInteger.h"
#include "GEDefine.h"

template<typename ElemType, GE::Uint16 arrSize>
class GEArray
{
public:
	GEArray() {}
	~GEArray() {}

public:
	void						Init(ElemType value);			//数组初始化
	ElemType& operator[](GE::Uint16 uIdx);						//数组索引访问（安全）
	ElemType&					At_Unsafe(GE::Uint16 uIdx);		//数组索引访问（不安全）
	GE::Uint16					MaxSize();						//数组最大大小
	ElemType*					HeadPrt();						//数组头指针

private:
	ElemType					m_arrData[arrSize];				//数组
};

template<typename ElemType, GE::Uint16 arrSize>
void GEArray<ElemType, arrSize>::Init( ElemType value )
{
	GE_ITER_UI16(idx, arrSize)
	{
		this->m_arrData[idx] = value;
	}
}

template<typename ElemType, GE::Uint16 arrSize>
ElemType& GEArray<ElemType, arrSize>::operator[]( GE::Uint16 uIdx )
{
	GE_ERROR(uIdx < MaxSize());
	return m_arrData[uIdx];
}

template<typename ElemType, GE::Uint16 arrSize>
ElemType& GEArray<ElemType, arrSize>::At_Unsafe( GE::Uint16 uIdx )
{
	return m_arrData[uIdx];
}

template<typename ElemType, GE::Uint16 arrSize>
GE::Uint16 GEArray<ElemType, arrSize>::MaxSize()
{
	return arrSize;
}

template<typename ElemType, GE::Uint16 arrSize>
ElemType* GEArray<ElemType, arrSize>::HeadPrt()
{
	return m_arrData;
}

