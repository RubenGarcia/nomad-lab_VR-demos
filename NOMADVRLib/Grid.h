#include <vector>

class grid 
{
public:
	grid (float *m, float *M, int dims, float s);
	~grid();
	void add (float *p); //compatible with the atoms xyzr
	std::vector<float*> find (float *p);
private:
	void coordinates(const float pos[3], int c[3]);
	bool compare (float *a, float *b);
	std::vector <float*> *content;
	float m[3];
	float M[3];
	float dif[3];
	int dims;
	float maxradius;
	const float scale;
}; // grid
