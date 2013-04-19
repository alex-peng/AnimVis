#pragma once

namespace AMT
{

class CGLLight
{
public:
	static CGLLight* CreateLight();
	static unsigned int numLights;
	bool IsOn() { return m_IsOn; }
	void TurnOn();
	void TurnOff();
	void SetPosition( float p0, float p1, float p2, float p3 );
	~CGLLight();
private:
	float* mat_specular;
	float* mat_diffuse;
	float* mat_ambient;
	float* mat_shininess;
	float* light_position;
	bool m_IsOn;
	unsigned int m_LightIndex;
	CGLLight();
};

}