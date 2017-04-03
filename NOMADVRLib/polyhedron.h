#ifndef __POLYHEDRON_H
#define __POLYHEDRON_H

class Icosahedron {
public:
    static const int nFaces;
	static const int Faces[];
    static const int nVerts;
	static const float Verts[];
};

class Octahedron {
public:
    static const int nFaces;
    static const int Faces[];
    static const int nVerts;
    static const float Verts[];
};

class Tetrahedron {
public:
    static const int nFaces;
    static const int Faces[];
    static const int nVerts;
    static const float Verts[];
};

class Solid {
public:
	enum Type {
		Icosahedron,
		Octahedron,
		Tetrahedron
	};
	Solid(Type t) {
		switch (t) {
		case Icosahedron:
			nFaces=Icosahedron::nFaces;
			Faces=Icosahedron::Faces;
			nVerts=Icosahedron::nVerts;
			Verts=Icosahedron::Verts;
			break;
		case Octahedron:
			nFaces=Octahedron::nFaces;
			Faces=Octahedron::Faces;
			nVerts=Octahedron::nVerts;
			Verts=Octahedron::Verts;
		case Tetrahedron:
			nFaces=Tetrahedron::nFaces;
			Faces=Tetrahedron::Faces;
			nVerts=Tetrahedron::nVerts;
			Verts=Tetrahedron::Verts;
		}
	}
int nFaces;
const int *Faces;
int nVerts;
const float *Verts;
};

#endif // __POLYHEDRON_H