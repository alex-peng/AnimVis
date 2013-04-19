#include "object.h"
#include "memory.h"

namespace AMT
{
CRenderObject::CRenderObject( bool _visiable, float _alpha )
		: m_visiable( _visiable ), m_VisiableTest(true), alpha( _alpha ),	beginx (0.0f),beginy(0.0f), m_ShaderReady(false) {};

void CRenderObject::ResetPosition()
{
	memcpy( position, boundingBox.center, 3*sizeof(float) );
}

}