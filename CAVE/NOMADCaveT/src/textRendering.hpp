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

#ifndef TEXT_RENDERING_HPP
#define TEXT_RENDERING_HPP

#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H 

#include <m3dshaderLoader.h>

///Text rendering class for OpenGL 4 and Freetype 2
///Compatible with libsynch

///Based on http://learnopengl.com/#!In-Practice/Text-Rendering
///License of original work: CC0 1.0 Universal
///https://creativecommons.org/publicdomain/zero/1.0/legalcode

///Adapted by Ruben Garcia (garcia@lrz.de) for use within NOMAD and
///libSynch
///License: Apache 2.0 (as required by NOMAD)

namespace TextRendering {

struct Character {
    GLuint     TextureID;  // ID handle of the glyph texture
    glm::ivec2 Size;       // Size of glyph
    glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
    GLuint     Advance;    // Offset to advance to next glyph
};

class Text {
///use within synchlib: call init in SceneManager constructor and 
///render in displayFunction. use pvmat*wand as proj parameter for wand-aligned
///use glm::decompose to use axis-aligned, wand-centred
///Warning, this code uses glgenvertexarrays and requires
///glewExperimental = GL_TRUE; GLenum err = glewInit(); 
///for glew under 1.20
private:
std::map<GLchar, Character> Characters;
GLuint VAO, VBO;
m3d::ShaderLoader s;
public:
Text();
bool init(std::string font);
///This function may only be used with a correctly setup OpenGL environment
///@arg font: The path to a ttf font in disk to load
bool render(std::string text, GLfloat x, GLfloat y, GLfloat scale, 
	glm::vec3 color, glm::mat4 proj);
///This function may only be used with a correctly setup OpenGL environment
///Carriage return is not supported for now
};

}
#endif
