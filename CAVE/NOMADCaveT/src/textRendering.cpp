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

#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <GL/glew.h>
#include <m3dshaderLoader.h>
#include "textRendering.hpp"

#include "defines.h"

namespace TextRendering {

Text::Text()
{
}

bool Text::init(std::string font) 
{
GLenum err;
//initialization
FT_Library ft;
FT_Face face;

if (FT_Init_FreeType(&ft)) {
    std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	return false;
}

if (FT_New_Face(ft, font.c_str(), 0, &face)) {
    std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;  
	return false;
}

FT_Set_Pixel_Sizes(face, 0, 96);  

while ((err = glGetError()) != GL_NO_ERROR) 
	std::cerr <<__FUNCTION__<<"line" << __LINE__<< " OpenGL error " << err << std::endl;

glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

//load of glyphs
for (GLubyte c = 0; c < 128; c++)
{
    // Load character glyph 
    if (FT_Load_Char(face, c, FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << c <<std::endl;
        continue;
    }
    // Generate texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
    );
    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Now store character for later use
    Character character = {
        texture, 
        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        face->glyph->advance.x
    };
    Characters.insert(std::pair<GLchar, Character>(c, character));
	while ((err = glGetError()) != GL_NO_ERROR) 
		std::cerr <<__FUNCTION__<<"line" << __LINE__<< " OpenGL error " << err << std::endl;
} //for

//cleanup
FT_Done_Face(face);
FT_Done_FreeType(ft);

while ((err = glGetError()) != GL_NO_ERROR) 
	std::cerr <<__FUNCTION__<<"line" << __LINE__<< " OpenGL error " << err << std::endl;

//vaos
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO);
glBindVertexArray(VAO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);      

while ((err = glGetError()) != GL_NO_ERROR) 
	std::cerr <<__FUNCTION__<<"line" << __LINE__<< " OpenGL error " << err << std::endl;

//loading of shaders
if (!s.loadShadersFromFiles(SHADERPATH "text.vert", SHADERPATH "text.frag"))
	return false;

while ((err = glGetError()) != GL_NO_ERROR) 
	std::cerr <<__FUNCTION__<<"line" << __LINE__<< " OpenGL error " << err << std::endl;

return err== GL_NO_ERROR;
} //bool Text::init(std::string font) 

bool Text::render(std::string text, GLfloat ix, GLfloat iy, GLfloat scale, 
	glm::vec3 color, glm::mat4 proj)
{
GLfloat x=ix, y=iy;
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
  // Activate corresponding render state	
    s.begin();
    s.setUniform("textColor", color);
    s.setUniformMatrix("projection", false, proj);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
	if (*c=='\n') {
		x=ix;
		y-=Characters[*c].Size.y * scale;
		continue;
	}
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

	s.end();
	return true;
} // render

} // namespace TextRendering

