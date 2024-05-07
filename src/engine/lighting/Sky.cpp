#include "Sky.h"

namespace Atlas {

    namespace Lighting {

        Sky::Sky() {



        }

        EnvironmentProbe* Sky::GetProbe() {

            // Prioritize user loaded cubemaps
            if (probe) return probe.get();

            return nullptr;

        }

    }

}