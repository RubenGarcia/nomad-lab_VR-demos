#include <glm/vec3.hpp>
/// for measuring length (2 points), angle (3 points), 
/// and dihedral angle (4 points)
struct SelectedPoints {
	int number;
	glm::vec3 p[4];
};
