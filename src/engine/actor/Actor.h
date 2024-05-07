#pragma once

#include "../System.h"
#include "../Camera.h"
#include "../volume/AABB.h"

namespace Atlas {

    namespace Actor {

        class Actor {

        public:
            Actor() = default;
            virtual ~Actor() {}

            inline virtual void SetMatrix(mat4 matrix) { this->matrix = matrix; matrixChanged = true; };
            inline mat4 GetMatrix() const { return matrix; }
            inline bool HasMatrixChanged() const { return matrixChanged; }
            virtual void Update(Camera camera, float deltaTime,
                mat4 parentTransform, bool parentUpdate) = 0;

            Volume::AABB aabb = Volume::AABB{ vec3{-1.0f}, vec3{1.0f} };
            mat4 globalMatrix = mat4{ 1.0f };
            mat4x3 inverseGlobalMatrix = mat4x3{ 1.0f };

            bool matrixChanged = true;
            bool visible = true;
            bool dontCull = false;

            std::string name;

        private:
            mat4 matrix = mat4(1.0f);

        };

    }

}