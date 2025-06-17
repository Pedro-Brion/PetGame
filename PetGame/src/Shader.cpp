#include "Shader.h"
#include "stb_image.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
	std::string vertexSource;
	std::ifstream vShaderFile;

	std::string fragSource;
	std::ifstream fShaderFile;

	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;

		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vertexSource = vShaderStream.str();
		fragSource = fShaderStream.str();
	}
	catch (std::ifstream::failure err) {
		std::cerr << "ERROR::SHADER::FILE_NOT_READ: " << err.what() << std::endl;
		if (vShaderFile.fail()) std::cerr << "   Vertex shader file failed to open\n";
		if (fShaderFile.fail()) std::cerr << "   Fragment shader file failed to open\n";
	}

	const char* vShaderCode = vertexSource.c_str();
	const char* fShaderCode = fragSource.c_str();

	ID = glCreateProgram();
	unsigned int vertexID, fragmentID;

	vertexID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexID, 1, &vShaderCode, NULL);
	glCompileShader(vertexID);
	glAttachShader(ID, vertexID);
	checkShaderCompilation(vertexID, "VERTEX");

	fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentID, 1, &fShaderCode, NULL);
	glCompileShader(fragmentID);
	glAttachShader(ID, fragmentID);
	checkShaderCompilation(fragmentID, "FRAGMENT");

	glLinkProgram(ID);
	checkShaderProgramLinking();

	glDeleteShader(vertexID);
	glDeleteShader(fragmentID);
}

Shader::~Shader()
{
	std::cout << "Deleting Shader program" << std::endl;
	glDeleteProgram(ID);
}

void Shader::use() {
	glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setInt(const std::string& name, int value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat4)
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat4));
}

void Shader::setVec3(const std::string& name, const glm::vec3& vec3)
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(vec3));
}

void Shader::checkShaderCompilation(unsigned int shader, const char* shaderName = "SHADER") const {
	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::" << shaderName << "::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	else {
		std::cout << "SUCCESS::SHADER::" << shaderName << "::COMPILATION_DONE\n" << std::endl;
	}
}

void Shader::checkShaderProgramLinking() const {
	int  success;
	char infoLog[512];
	glGetProgramiv(ID, 0x8B81, &success);

	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER_PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	else {
		std::cout << "SUCCESS::SHADER_PROGRAM::LINKING_DONE\n" << std::endl;
	}
}

