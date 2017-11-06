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
		Tetrahedron,
		Sphere
	};
	Solid(Type t, int SphereFacets=15) {
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
			break;
		case Tetrahedron:
			nFaces=Tetrahedron::nFaces;
			Faces=Tetrahedron::Faces;
			nVerts=Tetrahedron::nVerts;
			Verts=Tetrahedron::Verts;
			break;
		case Sphere:
//https://stackoverflow.com/questions/23143921/python-program-to-create-sphere-coordinates-not-working
//http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html For the future
			nVerts=SphereFacets*SphereFacets;
			
			float *tVerts=new float[nVerts*3];
			for (int i=0;i<SphereFacets;i++)
				for (int j=0;j<SphereFacets;j++) {
					float t, f;
					t=i*2*M_PI/(float)(SphereFacets-1);
					f=j*M_PI/(float)(SphereFacets-1);
					
					tVerts[i*SphereFacets*3 + j*3]=sin(f)*sin(t);
					tVerts[i*SphereFacets*3 + j*3+1]=sin(f)*cos(t);
					tVerts[i*SphereFacets*3 + j*3+2]=cos(f);
				}
			nFaces=2*(SphereFacets-1)*(SphereFacets-1);
			int *tFaces=new int[3*nFaces];
			for (int i=0;i<SphereFacets-1;i++)
				for (int j=0;j<SphereFacets-1;j++) {
					tFaces[i*(SphereFacets-1)*6 + j*6]=i*SphereFacets+j;
					tFaces[i*(SphereFacets-1)*6 + j*6+1]=((i+1)%SphereFacets)*SphereFacets+j;
					tFaces[i*(SphereFacets-1)*6 + j*6+2]=(i)*SphereFacets+(j+1)%SphereFacets;
					tFaces[i*(SphereFacets-1)*6 + j*6+3]=((i+1)%SphereFacets)*SphereFacets+j;
					tFaces[i*(SphereFacets-1)*6 + j*6+4]=(i)*SphereFacets+(j+1)%SphereFacets;
					tFaces[i*(SphereFacets-1)*6 + j*6+5]=((i+1)%SphereFacets)*SphereFacets+(j+1)%SphereFacets;
				}
			Faces=tFaces; Verts=tVerts;
		} // switch
	} // Solid (constructor)
	
int nFaces;
const int *Faces;
int nVerts;
const float *Verts;
}; // class Solid

#endif // __POLYHEDRON_H
