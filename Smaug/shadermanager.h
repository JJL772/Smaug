#pragma once
#include <bgfx/bgfx.h>
#include <glm/vec4.hpp>
// Probably a WAAAAAY better way to do this.

enum class Shader
{
	NONE,
	
	WORLD_PREVIEW_SHADER,
	EDIT_VIEW_SHADER,
	CURSOR_SHADER,
	MODEL_SHADER,
	ERROR_MODEL_SHADER,
	GRID,
	LINE,

	COUNT,
};

class CShaderManager
{
public:
	void Init();
	void Shutdown();

	void SetColor(glm::vec4 color);
	bgfx::ProgramHandle GetShaderProgram(Shader shader);
private:
	bgfx::UniformHandle m_colorUniform;
};

CShaderManager& ShaderManager();