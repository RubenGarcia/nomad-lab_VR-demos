#include "Grid.h"
#include "atoms.hpp" //for radius

grid::grid (float *m, float *M, int dims, float s):scale(s) {
	content = new std::vector<float*> [dims*dims*dims];
	for (int i=0;i<3;i++) {
		this->m[i]=m[i];
		this->M[i]=M[i];
		dif[i]=M[i]-m[i];
	}
	this->dims=dims;
	maxradius=0;
}

grid::~grid()
{
	delete [] content;
}

void grid::coordinates(const float pos[3], int c[3])
{
	for (int i=0;i<3;i++) {
		c[i]=floor((pos[i]-m[i])/dif[i]*dims);
		if (c[i]>=dims)
			c[i]=dims-1;
		else if (c[i]<0)
			c[i]=0;
	}
}

void grid::add (float *p) //compatible with the atoms xyzr
{
	int pos[3];
	coordinates (p, pos);
	
	content[pos[0]*dims*dims + pos[1]*dims+pos[2]].push_back(p);
	float ar=atomRadius(p[3]);
	if (ar>maxradius)
		maxradius=ar;
}

bool grid::compare (float *a, float *b)
{
	if (a<=b) //already returned when searching a beforehand
		return false;
	float d=0;
	for (int i=0;i<3;i++)
		d+=(a[i]-b[i])*(a[i]-b[i]);
	if (d*scale < (a[3]+b[3]))
		return true;
}

std::vector<float*> grid::find (float *p) 
{
	std::vector<float*> result;
	//search a sphere centered in p, of radius (p[3]+maxradius) *scale

	//start by searching a cube
	int mc[3];
	int Mc[3];
	float mp[3];
	float Mp[3];
	for (int i=0;i<3;i++) {
		mp[i]=p[i]-(atomRadius(p[3])+maxradius)/scale;
		Mp[i]=p[i]+(atomRadius(p[3])+maxradius)/scale;
	}
	coordinates(mp, mc);
	coordinates(Mp, Mc);
	for (int x=mc[0];x<Mc[0];x++)
		for (int y=mc[1];y<Mc[1];y++)
			for (int z=mc[2];z<Mc[2];z++) {
				const int c=x*dims*dims + y*dims+z;
				for (int i=0;i<content[c].size();i++) {
					if (compare (content[c][i], p))
						result.push_back(content[c][i]);
				}
			}
	return result;			
}

