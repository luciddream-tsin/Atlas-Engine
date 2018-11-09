#ifndef SHADERBATCH_H
#define SHADERBATCH_H

#include "../System.h"
#include "Shader.h"
#include "ShaderConfigBatch.h"

#include <vector>

class ShaderBatch {

public:
	ShaderBatch();

	void AddComponent(int32_t type, const char* filename);

	void AddConfig(ShaderConfig* config);

	void RemoveConfig(ShaderConfig* config);

	Uniform* GetUniform(const char* uniformName);

	void Bind(int32_t shaderID);

	vector<ShaderSource*> components;
	vector<ShaderConfigBatch*> configBatches;
	vector<Uniform*> uniforms;	

	int32_t boundShaderID;

};

#endif