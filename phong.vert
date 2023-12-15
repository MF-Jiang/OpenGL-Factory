#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aCol;
layout(location = 2) in vec3 aNor;
layout(location = 3) in vec3 aTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 col;
out vec3 nor;
out vec3 FragPos;
out vec2 tex;


void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.f);
	FragPos = vec3(model * vec4(aPos, 1.f));
	col = aCol;
	nor = mat3(transpose(inverse(model))) * aNor;
	tex = aTex.xy;
}
