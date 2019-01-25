#ifndef MESHACTOR_H
#define MESHACTOR_H

#include "../System.h"
#include "Mesh.h"

class MeshActor {

public:

	/**
	 *
	 * @param mesh
	 */
	MeshActor(Mesh* mesh);

	void Update();

	mat4 modelMatrix;
	mat4 transformedMatrix;

	Mesh* const mesh;

	bool castShadow;

	bool visible;

};

#endif