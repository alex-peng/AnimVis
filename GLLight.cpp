#include "GLLight.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <memory.h>

namespace AMT
{
unsigned int CGLLight::numLights = 0;

CGLLight::CGLLight()
{
	m_IsOn = true;
	float arr1[] = {0.5f, 0.5f, 0.5f, 1.0f };
	mat_specular = new float[4]; memcpy( mat_specular, arr1, 4*sizeof(float) );
	float arr2[] = {0.8f, 0.8f, 0.8f, 1.0f };
	mat_diffuse = new float[4]; memcpy( mat_diffuse, arr2, 4*sizeof(float) );
	float arr3[] = {0.8f, 0.8f, 0.8f, 1.0f };
	mat_ambient = new float[4]; memcpy( mat_ambient, arr3, 4*sizeof(float) );
	mat_shininess = new float[1]; mat_shininess[0] = 50.0f;
	float arr4[] = { 0.0, 0.0, 10.0, 1.0 };
	light_position = new float[4]; memcpy( light_position, arr4, 4*sizeof(float) );
	m_LightIndex = numLights;
	numLights++;

	glLightfv(GL_LIGHT0+m_LightIndex, GL_SPECULAR, mat_specular);
	glLightfv(GL_LIGHT0+m_LightIndex, GL_AMBIENT, mat_ambient);
	glLightfv(GL_LIGHT0+m_LightIndex, GL_DIFFUSE, mat_diffuse);
	glLightfv(GL_LIGHT0+m_LightIndex, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0+m_LightIndex, GL_POSITION, light_position);
}

CGLLight::~CGLLight()
{
	delete [] mat_specular;
	delete [] mat_diffuse;
	delete [] mat_ambient;
	delete [] mat_shininess;
	delete [] light_position;
	numLights--;
}

void CGLLight::SetPosition( float p0, float p1, float p2, float p3 )
{
	light_position[0] = p0;
	light_position[1] = p1;
	light_position[2] = p2;
	light_position[3] = p3;
}

CGLLight* CGLLight::CreateLight()
{
	if ( numLights >= GL_MAX_LIGHTS )
		return NULL;
	CGLLight* glLight = new CGLLight;
	return glLight;
}

void CGLLight::TurnOn()
{
	glEnable(GL_LIGHT0+m_LightIndex);
	m_IsOn = true;
}

void CGLLight::TurnOff()
{
	glDisable(GL_LIGHT0+m_LightIndex);
	m_IsOn = false;
}

}// end name space
