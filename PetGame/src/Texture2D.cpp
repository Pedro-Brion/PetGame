#include "Texture2D.h"
#include "stb_image.h"
#include <glad/glad.h>
#include <iostream>

PetGame::Texture2D::Texture2D()
	:m_width(0),
	m_height(0),
	m_internalFormat(GL_RGBA),
	m_imageFormat(GL_RGBA),
	m_wrapS(GL_REPEAT),
	m_wrapT(GL_REPEAT),
	m_filterMin(GL_NEAREST_MIPMAP_LINEAR),
	m_filterMax(GL_NEAREST)
{
	glGenTextures(1, &ID);
}


PetGame::Texture2D::~Texture2D()
{
}

void PetGame::Texture2D::Generate(int width, int height, unsigned char* data)
{
	m_width = width;
	m_height = height;

	glBindTexture(GL_TEXTURE_2D, ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filterMin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filterMax);

	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, m_width, m_height, 0, m_imageFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
}

void PetGame::Texture2D::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, ID);
}

bool PetGame::Texture2D::Load(const char* filePath)
{
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
	std::cout << "Loading " << filePath << std::endl;
	if (data) {

		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		else
		{
			std::cout << "Texture format not supported for " << filePath << std::endl;
			return false;
			stbi_image_free(data);
		}
		m_internalFormat = format;
		m_imageFormat = format;
		Generate(width, height, data);
		stbi_image_free(data);
		return true;
	}
	else {
		std::cout << "Texture failed to load at path: " << filePath << std::endl;
		return false;
	}

}

std::unique_ptr<PetGame::Texture2D> PetGame::Texture2D::CreateTexture(const char* filePath)
{
	std::cout << "Creating unique" << std::endl;
	std::unique_ptr<PetGame::Texture2D> texture = std::make_unique<PetGame::Texture2D>();
	texture->Load(filePath);
	return texture;
}
