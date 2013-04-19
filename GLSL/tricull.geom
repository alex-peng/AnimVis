#version 120 
#extension GL_EXT_geometry_shader4 : enable

//CIRL GPU Geometry Program: Derek Anderson and Robert Luke
// very simple geometry shader

/*
GEOMETRY SHADER DOCUMENTATION

Geometry language built-in outputs:
	varying out vec4 gl_FrontColor;
	varying out vec4 gl_BackColor;
	varying out vec4 gl_FrontSecondaryColor;
	varying out vec4 gl_BackSecondaryColor;
	varying out vec4 gl_TexCoord[]; 	// at most gl_MaxTextureCoords
	varying out float gl_FogFragCoord; 

Geometry language input varying variables:
	varying in vec4 gl_FrontColorIn[gl_VerticesIn];
	varying in vec4 gl_BackColorIn[gl_VerticesIn];
	varying in vec4 gl_FrontSecondaryColorIn[gl_VerticesIn];
	varying in vec4 gl_BackSecondaryColorIn[gl_VerticesIn];
	varying in vec4 gl_TexCoordIn[gl_VerticesIn][]; 	// at most will be// gl_MaxTextureCoords
	varying in float gl_FogFragCoordIn[gl_VerticesIn];
	varying in vec4 gl_PositionIn[gl_VerticesIn];
	varying in float gl_PointSizeIn[gl_VerticesIn];
	varying in vec4 gl_ClipVertexIn[gl_VerticesIn];

Geometry Shader Function 
	
	This section contains functions that are geometry language specific.
	
	Syntax:
	 void EmitVertex(); 
	 void EndPrimitive(); 
	
	Description:
	
	 The function EmitVertex() specifies that a vertex is completed. A vertex is added to the current output primitive using the current values of the varying output variables and the current values of the special built-in output variables gl_PointSize, gl_ClipVertex, gl_Layer, gl_Position and gl_PrimitiveID. The values of any unwritten output variables are undefined. The values of all varying output variables and the special built-in output variables are undefined after a call to EmitVertex(). If a geometry shader, in one invocation, emits more vertices than the valueGEOMETRY_VERTICES_OUT_EXT, these emits may have no effect. 
 
	The function EndPrimitive() specifies that the current output primitive is completed and a new output primitive (of the same type) should be started. This function does not emit a vertex. The effect of EndPrimitive() is roughly equivalent to calling End followed by a new Begin, where the primitive mode is taken from the program object parameter GEOMETRY_OUTPUT_TYPE_EXT. If the output primitive type is POINTS, calling EndPrimitive() is optional.
*/

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//varying out vec3 normal;

varying out vec3 lightdir;
varying out vec3 ec_pos;

void main(void)
{
	int i;
	vec4 pc_pos[3];
	pc_pos[0] = gl_ProjectionMatrix * gl_PositionIn[0];
	pc_pos[1] = gl_ProjectionMatrix * gl_PositionIn[1];
	pc_pos[2] = gl_ProjectionMatrix * gl_PositionIn[2];
	
	int outOfBound[6] = int[6]( 0, 0, 0, 0, 0, 0 );
	
	for (i=0; i<3; i++)
	{
	  if ( pc_pos[i].x >  pc_pos[i].w ) outOfBound[0]++;
	  if ( pc_pos[i].x < -pc_pos[i].w ) outOfBound[1]++;
	  if ( pc_pos[i].y >  pc_pos[i].w ) outOfBound[2]++;
	  if ( pc_pos[i].y < -pc_pos[i].w ) outOfBound[3]++;
	  if ( pc_pos[i].z >  pc_pos[i].w ) outOfBound[4]++;
	  if ( pc_pos[i].z < -pc_pos[i].w ) outOfBound[5]++;
	}
	
	bool inFrustum = true;
   
	for (int i=0; i<6; i++)
	{
		if ( outOfBound[i] == 3 )
		{
			inFrustum = false;
		}
	}
	if( inFrustum )
	{
		for(i=0; i< gl_VerticesIn; i++){
			ec_pos = gl_PositionIn[i].xyz;
			lightdir = normalize(gl_LightSource[0].position.xyz-ec_pos);
			gl_Position = gl_ProjectionMatrix * gl_PositionIn[i];
			EmitVertex();
		}
		EndPrimitive();
	}
}