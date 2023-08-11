#include "GUIText.h"
#include "Font.h"
#include "Engine/Program.h"
#include "glad/glad.h"

GUIText::GUIText(glm::vec2 position, glm::vec2 relativeTo, glm::vec2 size,GLFWwindow* window, char* string, int stringLength, Font* fontUsed):GUIBase(position,relativeTo,size,window),font{fontUsed}
{
	renderProgram = new Program("Engine/Shaders/GUIText.vert", "Engine/Shaders/GUIText.frag",Program::filePath);
	renderProgram->setUniformBufferBlockBinding("windowData", 0);

	generateNewString(string, stringLength);
}

GUIText::~GUIText()
{
	delete[] characterArray;
}

void GUIText::generateNewString(char* newString, int newStringLength)
{
	//If the strings are identical, don't do anything
	if(newStringLength==stringLength)
	{
		bool same{ true };
		for(int i=0;i<stringLength;i++)
		{
			same = same && newString[i] == string[i];
		}
		if (same) { return; }
	}

	//Cleanup old array (textures don't need to be deleted, since that is handled in the font)
	if(characterArray!=nullptr){ delete[] characterArray; }
	

	characterArray = new character[newStringLength];

	for(int i=0;i<newStringLength;i++)
	{
		characterArray[i] = font->getCharacter(newString[i]);
	}
	string = newString;
	stringLength = newStringLength;
}

void GUIText::render()
{
	/*
	 Bind the buffers and array
		VAO + element array
	Use program
		Set uniforms
			yOffset(y position of text)
			textureID(0)
			
			relativeTo
			textColour
	 Store curXOffset(start 0)
	 Iterate through each character
		Set xOffset to curXOffset
		Set size to the size of the character
		Bind character texture
		Render character to screen
		Add characterWidth to curXOffset
	 */

	glBindVertexArray(vaoId);	//Bind VAO and element array buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);

	renderProgram->use();	//use program and set non-character specific uniforms
	renderProgram->setVec2("relativeTo", relativeTo);

	renderProgram->setVec3("colour", glm::vec3{1,1,1});	
	renderProgram->setInt("tex", 0);

	float curXPos = position.x;
	for(int i=0;i<stringLength;i++)
	{
		const character& current = characterArray[i];
		renderProgram->setFloat("xPos", curXPos+current.bearing.x);
		renderProgram->setFloat("yPos", position.y-(current.size.y-current.bearing.y));
		renderProgram->setVec2("size", current.size);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, current.textureID);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		curXPos += (current.advance >> 6);	//Bit shifted to get in pixels 
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

