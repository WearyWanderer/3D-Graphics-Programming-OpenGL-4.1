#pragma once

#include <SceneModel/SceneModel_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tygra\Image.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <chrono>
#include <map>

class MyView : public tygra::WindowViewDelegate
{
public:
	
    MyView();
	
    ~MyView();

    void
    setScene(std::shared_ptr<const SceneModel::Context> scene);

	void toggleExtraFeature(); //switch the feature on/off

	const bool getToggleState() const; //retrieve extra feature info

private:

	std::chrono::system_clock::time_point start_time_;

	//texture name
	GLuint amb_textures;
	GLuint amb_textures1;
	GLuint amb_textures2;

	//shader compiled name
	GLuint sponza_program;

	struct GLMesh
	{
		GLuint normal_vbo;
		GLuint position_vbo; // VBO for positions
		GLuint element_vbo; // VBO for elements (indices)
		GLuint texcoord_vbo;
		GLuint vao; // VAO for shape's vertex array settings
		int element_count; // Needed for drawing the vertex arrays

		GLMesh() : position_vbo(0),
			element_vbo(0),
			normal_vbo(0),
			texcoord_vbo(0),
			vao(0),
			element_count(0) {}
	};

	std::map<SceneModel::MeshId, GLMesh> _sponzaMesh;
	GLMesh _currentMesh;

	GLuint MyView::ShaderCompile(std::string shaderPath, int shaderType);

	tygra::Image MyView::GenerateTexture(std::string filepath, int numTex);

	void PerInstanceRender(SceneModel::Instance thisInstance, glm::mat4 projection_xform, glm::mat4 view_xform); //the drawing for each instance that occurs in WindowWIllRender function

    void
    windowViewWillStart(std::shared_ptr<tygra::Window> window) override;
	
    void
    windowViewDidReset(std::shared_ptr<tygra::Window> window,
                       int width,
                       int height) override;

    void
    windowViewDidStop(std::shared_ptr<tygra::Window> window) override;
    
    void
    windowViewRender(std::shared_ptr<tygra::Window> window) override;

    std::shared_ptr<const SceneModel::Context> scene_;

	bool extraToggle = false; //the toggle feature for extra shader
    
};
