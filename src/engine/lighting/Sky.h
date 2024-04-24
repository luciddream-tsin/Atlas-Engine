#pragma once

#include "../System.h"

#include "EnvironmentProbe.h"
#include "Atmosphere.h"
#include "VolumetricClouds.h"

namespace Atlas {

    namespace Lighting {

        class Sky {

        public:
            Sky();


            vec3 planetCenter = vec3(0.0f, -650000.0f, 0.0f);
            float planetRadius = 649000.0f;


        };

    }

}