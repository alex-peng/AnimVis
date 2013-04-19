#ifndef _OBJECT_H_
#define _OBJECT_H_

namespace AMT
{

typedef enum {
	WIREFRAME = 0,
	GOURAUD,
	TEXTURED
} TRenderMode;

class CBoundingBox
{
public:
	float center[3];
	float size;
	CBoundingBox() : size(0.0f) {};
};

class CRenderObject
{
public:
	bool m_visiable;
	bool m_VisiableTest;
	bool m_ShaderReady;
	float alpha; // For transparent display
    float beginx, beginy;  /* position of mouse */
	float position[3];
	float m_VisibiltyTime;

	float quat[4];
	CBoundingBox boundingBox;
	CRenderObject( bool _visiable = true, float _alpha = 1.0f );
	void ResetPosition();
	virtual void draw() = 0;
	virtual void destroyMyself() = 0;
	virtual unsigned int getFaceNumber() = 0;
	virtual void ChangeRenderMode(TRenderMode rm ) = 0;
	virtual void SetShader() = 0;
	virtual unsigned int getVisFaceNumber() { return getFaceNumber(); }
};

}

#endif