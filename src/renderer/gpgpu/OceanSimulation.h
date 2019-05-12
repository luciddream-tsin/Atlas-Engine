#ifndef AE_OCEANSIMULATION_H
#define AE_OCEANSIMULATION_H

#include "../../System.h"
#include "../helper/NoiseGenerator.h"
#include "../../texture/Texture2D.h"
#include "../../shader/Shader.h"

#include <vector>

namespace Atlas {

	namespace Renderer {

		namespace GPGPU {

			class OceanSimulation {
			
			public:
				OceanSimulation() {}

				OceanSimulation(int32_t N, int32_t L);

				void SetState(float waveAmplitude, vec2 waveDirection,
					float windSpeed, float windDependency);

				void Compute();

				Texture::Texture2D displacementMap;
				Texture::Texture2D normalMap;

				Texture::Texture2D twiddleIndices;

			private:
				void ComputeH0();

				void ComputeTwiddleIndices();

				int32_t ReverseBits(int32_t data, int32_t bitCount);

				int32_t N;
				int32_t L;

				float waveAmplitude = 0.35f;
				vec2 waveDirection = vec2(0.8f, 0.6f);

				float windSpeed = 600.0f;
				float windDependency = 0.07f;

				Shader::Shader h0;
				Shader::Shader ht;
				Shader::Shader twiddle;
				Shader::Shader butterfly;
				Shader::Shader inversion;
				Shader::Shader normal;

				// Precomputed noise textures
				Texture::Texture2D noise0;
				Texture::Texture2D noise1;
				Texture::Texture2D noise2;
				Texture::Texture2D noise3;

				Texture::Texture2D h0K;
				Texture::Texture2D h0MinusK;

				Texture::Texture2D hTDy;
				Texture::Texture2D hTDx;
				Texture::Texture2D hTDz;

				Texture::Texture2D hTDyPingpong;
				Texture::Texture2D hTDxPingpong;
				Texture::Texture2D hTDzPingpong;

				Shader::Uniform* htNUniform;
				Shader::Uniform* htLUniform;
				Shader::Uniform* htTimeUniform;

				Shader::Uniform* butterflyStageUniform;
				Shader::Uniform* butterflyPingpongUniform;
				Shader::Uniform* butterflyDirectionUniform;

				Shader::Uniform* inversionNUniform;
				Shader::Uniform* inversionPingpongUniform;

				Shader::Uniform* normalNUniform;

			};

		}

	}

}


#endif