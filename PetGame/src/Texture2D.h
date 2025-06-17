#pragma once

#include <memory>
namespace PetGame {
	class Texture2D
	{
	public:
		Texture2D();
		~Texture2D();

		unsigned int ID;
		int m_width, m_height;
		unsigned int m_internalFormat;
		unsigned int m_imageFormat;

		unsigned int m_wrapS;
		unsigned int m_wrapT;
		unsigned int m_filterMin;
		unsigned int m_filterMax;


		void Bind() const;

		bool Load(const char* filePath);

		static std::unique_ptr<PetGame::Texture2D> CreateTexture(const char* filePath);
	private:
		void Generate(int width, int height, unsigned char* data);

	};

}

