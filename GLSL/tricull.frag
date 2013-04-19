varying vec3 lightdir;
varying vec3 ec_pos;

void main() {
	vec3 ec_normal = normalize(cross(dFdx(ec_pos), dFdy(ec_pos)));
    float diffuse_value = max(dot(ec_normal, lightdir), 0.0);
	vec4 diffuse = diffuse_value*gl_FrontMaterial.diffuse*gl_LightSource[0].diffuse;
	vec4 ambient = vec4(0.1, 0.1, 0.1, 1.0) * gl_LightSource[0].ambient;
	vec4 color = diffuse + ambient;

    // Set the output color of our current pixel
    gl_FragColor = vec4(color.xyz, 1.0 );
}