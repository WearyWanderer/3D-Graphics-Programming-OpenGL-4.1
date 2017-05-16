#version 330

uniform mat4 model_xform;
uniform mat4 projection_view_model_xform;

in vec3 vertex_position;
in vec3 vertex_normal;
in vec2 vertex_texcoord;

out vec2 varying_texcoord;
out vec3 varying_normal;
out vec3 varying_position;
out vec3 barycentric_position;

void main(void)
{
    gl_Position = projection_view_model_xform * vec4(vertex_position, 1.0);

	//both varying's used for lighting
	varying_normal = mat3(model_xform) * vertex_normal; 
	varying_position =  mat4x3(model_xform) * vec4(vertex_position, 1.0);

	//below is the texturing components
	varying_texcoord = vertex_texcoord;
	//vec3 normal_colour = mat3(model_xform) * vertex_normal;
	//normal_colour_final = 0.5 + 0.5 * normal_colour;

	
}
