#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#define SHADER_UNIFORM_STR(S) #S
#define SHADER_UNIFORM_STRI(S) SHADER_UNIFORM_STR(S)

#define GLSL_DEFINITION(name) uniform name
#define GLSL_SSBO_DEFINITION(name) struct name##Data
#define F_DEFINITION(name) struct name

#define GLSL_MAT4(name) mat4 name;
#define F_MAT4(name) alignas(16) glm::mat4 name;

#define GLSL_FLOAT(name)  float name;
#define GLSL_VEC2(name) vec2 name;
#define GLSL_VEC3(name) vec3 name;
#define GLSL_VEC4(name) vec4 name;
#define F_FLOAT(name) alignas(16) float name;
#define F_VEC2(name) alignas(16) glm::vec2 name;
#define F_VEC3(name) alignas(16) glm::vec3 name;
#define F_VEC4(name) alignas(16) glm::vec4 name;

#define GLSL_UNIFORM_STRING(set, name, structure) std::string("layout(binding=") + (set) + ",std140) " SHADER_UNIFORM_STR(structure) " " + (name) + ";" 
#define GLSL_SSBO_STRING(set, name, structure, structureName) std::string(SHADER_UNIFORM_STR(structure)) + "; layout(std140,binding=" + (set) + ") readonly buffer " + (structureName) + " { " + (structureName) + "Data objects[]; } " + (name) + ";"
#define GLSL_PUSHBUFFER_STRING(name, structure) std::string("layout(binding=64,std140) " SHADER_UNIFORM_STR(structure) " ") + (name) + ";"

#define CAMERA_SHADER_STRUCTURE(D, M4) \
D(CameraShaderBuffer) \
{ \
M4(View) \
M4(Proj) \
M4(InvView) \
M4(InvProj) \
M4(ViewProj) \
}
#define GLSL_CAMERA_SHADER_STRUCTURE CAMERA_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_MAT4)

#define MODEL_SHADER_NAME ModelShaderBuffer
#define MODEL_SHADER_NAMESTR SHADER_UNIFORM_STRI(MODEL_SHADER_NAME)
#define MODEL_SHADER_STRUCTURE(D, M4) \
D(ModelShaderBuffer) \
{ \
M4(Model) \
M4(InvModel) \
}
#define GLSL_MODEL_SHADER_STRUCTURE MODEL_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_MAT4)
#define GLSL_MODEL_SSBO_STRUCTURE MODEL_SHADER_STRUCTURE(GLSL_SSBO_DEFINITION, GLSL_MAT4)

#define TIME_SHADER_BUFFER(D, V2) \
D(TimeShaderBuffer) \
{ \
V2(Time) \
}
#define GLSL_TIME_SHADER_STRUCTURE TIME_SHADER_BUFFER(GLSL_DEFINITION, GLSL_VEC2)

CAMERA_SHADER_STRUCTURE(F_DEFINITION, F_MAT4);
MODEL_SHADER_STRUCTURE(F_DEFINITION, F_MAT4);
TIME_SHADER_BUFFER(F_DEFINITION, F_VEC2);
