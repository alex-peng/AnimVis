//CIRL GPU Vertex Program: Derek Anderson and Robert Luke
// very simple vertex shader

void main()
{	
	//Transform the vertex (ModelViewProj matrix)
	//gl_Position = ftransform();
	gl_FrontColor = gl_Color;
	gl_Position = gl_Vertex;
}
