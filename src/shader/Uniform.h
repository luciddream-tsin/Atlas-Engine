#ifndef UNIFORM_H
#define UNIFORM_H

#include "../System.h"

class ShaderBatch;

class Uniform {

public:
	Uniform(uint32_t shaderID, const char* uniformName, ShaderBatch* shaderBatch = nullptr, int32_t ID = 0);

	void SetValue(int32_t value);

	void SetValue(float value);

	void SetValue(bool value);

	void SetValue(mat4 value);

	void SetValue(mat3 value);

	void SetValue(vec4 value);

	void SetValue(vec3 value);

	void SetValue(vec2 value);

	void SetValue(int32_t *value, int32_t length);

	void SetValue(float *value, int32_t length);

	const char* name;

private:
	int32_t ID;

	ShaderBatch* shaderBatch;

	inline Uniform* GetBatchUniform();

};

#endif