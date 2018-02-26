/*
# Copyright 2016-2018 The NOMAD Developers Group
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #     http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
*/


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
