#include "MyView.hpp"
#include <SceneModel/SceneModel.hpp>
#include <tygra/FileHelper.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cassert>

MyView::
MyView() : sponza_program(0)
{


}

MyView::
~MyView() {
}

void MyView::
setScene(std::shared_ptr<const SceneModel::Context> scene)
{
    scene_ = scene;
}

void MyView::
windowViewWillStart(std::shared_ptr<tygra::Window> window)
{
    assert(scene_ != nullptr);

	start_time_ = std::chrono::system_clock::now();

	GLint compile_status = 0;

	GLuint vertex_shader = ShaderCompile("sponza_vs.glsl", 1);
	GLuint fragment_shader = ShaderCompile("sponza_fs.glsl", 2);

	sponza_program = glCreateProgram();

	glAttachShader(sponza_program, vertex_shader);
	glBindAttribLocation(sponza_program, 0, "vertex_position");
	glBindAttribLocation(sponza_program, 1, "vertex_normal");
	glBindAttribLocation(sponza_program, 2, "vertex_texcoord");

	glDeleteShader(vertex_shader);

	glAttachShader(sponza_program, fragment_shader);
	glBindFragDataLocation(sponza_program, 0, "fragment_colour");
	glDeleteShader(fragment_shader);

	glLinkProgram(sponza_program);

	//error catch if the link has failed and show the reason why
	GLint link_status = 0;
	glGetProgramiv(sponza_program, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE) {
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(sponza_program, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	//set up the map to loop through all meshes

	SceneModel::GeometryBuilder builder;

	const auto& meshMap = builder.getAllMeshes();

	for (size_t i = 0; i < meshMap.size(); i++)
	{
		const auto& source_mesh = meshMap[i];

		const auto& positions = source_mesh.getPositionArray();
		const auto& elements = source_mesh.getElementArray();
		const auto& normals = source_mesh.getNormalArray();
		const auto& texcoords = source_mesh.getTextureCoordinateArray();
		

		//gen the buffers
		glGenBuffers(1, &_currentMesh.position_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _currentMesh.position_vbo);
		glBufferData(GL_ARRAY_BUFFER,
			positions.size() * sizeof(glm::vec3), // size of data in bytes
			positions.data(), // pointer to the data
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &_currentMesh.normal_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _currentMesh.normal_vbo);
		glBufferData(GL_ARRAY_BUFFER,
			normals.size() * sizeof(glm::vec3),
			normals.data(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &_currentMesh.texcoord_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _currentMesh.texcoord_vbo);
		glBufferData(GL_ARRAY_BUFFER,
			texcoords.size() * sizeof(glm::vec2),
			texcoords.data(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &_currentMesh.element_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _currentMesh.element_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			elements.size() * sizeof(unsigned int),
			elements.data(),
			GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		_currentMesh.element_count = elements.size();

		//bind to the vao
		glGenVertexArrays(1, &_currentMesh.vao);
		glBindVertexArray(_currentMesh.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _currentMesh.element_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _currentMesh.position_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, _currentMesh.normal_vbo);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, _currentMesh.texcoord_vbo);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
			sizeof(glm::vec2), TGL_BUFFER_OFFSET(0));

		//clear the buffer to empty
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		_sponzaMesh[source_mesh.getId()] = _currentMesh;
	}

	//align our pixel info in case of non-4-byte alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	tygra::Image texture_image = GenerateTexture("amb1.png", 1);
	tygra::Image texture_image2 = GenerateTexture("amb2.png", 2);
	tygra::Image texture_image3 = GenerateTexture("amb3.png", 3);
}

void MyView::
windowViewDidReset(std::shared_ptr<tygra::Window> window,
											   int width,
											   int height)
{
    glViewport(0, 0, width, height);
}

void MyView::
windowViewDidStop(std::shared_ptr<tygra::Window> window)
{
	//run through the deletes here
	glDeleteProgram(sponza_program);
	glDeleteBuffers(1, &_currentMesh.position_vbo);
	glDeleteBuffers(1, &_currentMesh.element_vbo);
	glDeleteBuffers(1, &_currentMesh.normal_vbo);
	glDeleteVertexArrays(1, &_currentMesh.vao);
}

void MyView::
windowViewRender(std::shared_ptr<tygra::Window> window)
{
    assert(scene_ != nullptr);

	//current_time_ = std::chrono::system_clock::now();

	//depth test enable
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.f, 0.f, 0.25f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto& lights = scene_->getAllLights(); //get all scene lights
	glm::vec3 ambient_intensity = scene_->getAmbientLightIntensity(); //link this later after use program

	SceneModel::Camera camera = scene_->getCamera();
	glm::vec3 camera_pos = camera.getPosition();
	glm::vec3 camera_at_pos = camera.getDirection();

	GLint viewport_size[4];
	glGetIntegerv(GL_VIEWPORT, viewport_size);
	const float aspect_ratio = viewport_size[2] / (float)viewport_size[3];

	//wide angled viewpoint (could swap to 45.f for shortened range, more real first person?)
	glm::mat4 projection_xform = glm::perspective(75.f,
		aspect_ratio,
		1.0f, 1000.f);

	//need to define camera pos and camera at pos
	glm::mat4 view_xform = glm::lookAt(camera_pos,
		camera_pos + camera_at_pos, glm::vec3(0, 1, 0));

	//model matrix needs to pass in the vertex info
	glm::mat4 model_xform = glm::translate(glm::mat4(1.0),
		glm::vec3(0.0f, 0.0f, 0.0f));

	//multiply matrices to get the correct view frustum
	glm::mat4 combined_xform = projection_xform
		* view_xform
		* model_xform;

	glUseProgram(sponza_program);

	for (unsigned int i = 0; i < lights.size(); i++)
	{
		GLuint lights_vector_uni1 = glGetUniformLocation(sponza_program, ("lights_data[" + std::to_string(i) + "].position").c_str()); //get the uniform address for first field
		GLuint lights_vector_uni2 = glGetUniformLocation(sponza_program, ("lights_data[" + std::to_string(i) + "].direction").c_str()); //get the uniform address for second field
		GLuint lights_vector_uni3 = glGetUniformLocation(sponza_program, ("lights_data[" + std::to_string(i) + "].field_of_view").c_str()); //get the uniform address etc
		GLuint lights_vector_uni5 = glGetUniformLocation(sponza_program, ("lights_data[" + std::to_string(i) + "].c_dist_co").c_str());
		GLuint lights_vector_uni6 = glGetUniformLocation(sponza_program, ("lights_data[" + std::to_string(i) + "].q_dist_co").c_str());
		glUniform3fv(lights_vector_uni1, 1, glm::value_ptr(lights[i].getPosition())); // position info
		glUniform3fv(lights_vector_uni2, 1, glm::value_ptr(lights[i].getDirection())); // direction info
		glUniform1f(lights_vector_uni3, lights[i].getConeAngleDegrees()); // cone_angle_degrees    
		glUniform1f(lights_vector_uni5, lights[i].getConstantDistanceAttenuationCoefficient());
		glUniform1f(lights_vector_uni6, lights[i].getQuadraticDistanceAttenuationCoefficient());
	}

	GLuint ambient_intensity_uni = glGetUniformLocation(sponza_program, "ambient_intensity");
	glUniform3fv(ambient_intensity_uni, 1, glm::value_ptr(ambient_intensity));

	GLuint camera_pos_uni = glGetUniformLocation(sponza_program, "camera_position"); //camera position for specular, only needs to be sent once
	glUniform3fv(camera_pos_uni, 1, glm::value_ptr(camera_pos));
	//get all instances
	const auto& instance = scene_->getAllInstances();
	//loop through instances
	for (unsigned int i = 0; i < instance.size(); i++)
	{
		const SceneModel::Instance thisInstance = instance[i];	
		PerInstanceRender(thisInstance, projection_xform, view_xform);
	}
}

#pragma region REFACTORED_INTO_FUNCTIONS //code of above functions stripped into smaller functions for refactoring

//Function in order to shorten the shader compilation, pass in shader to compile
GLuint MyView::ShaderCompile(std::string shaderPath, int shaderType) 
{
	GLint compile_status = 0;
	GLuint result;

	if (shaderType == 1)
	{
		GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		std::string vertex_shader_string = tygra::stringFromFile(shaderPath);
		const char* vertex_shader_code = vertex_shader_string.c_str();
		glShaderSource(vertex_shader, 1, (const GLchar **)&vertex_shader_code, NULL);
		glCompileShader(vertex_shader);
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
		if (compile_status != GL_TRUE)
		{
			const int string_length = 1024;
			GLchar log[string_length] = "";
			glGetShaderInfoLog(vertex_shader, string_length, NULL, log);
			std::cerr << log << std::endl;
		}

		result = vertex_shader;
	}

	if (shaderType == 2)
	{
		GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		std::string fragment_shader_string = tygra::stringFromFile(shaderPath);
		const char* fragment_shader_code = fragment_shader_string.c_str();
		glShaderSource(fragment_shader, 1, (const GLchar **)&fragment_shader_code, NULL);
		glCompileShader(fragment_shader);
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);

		if (compile_status != GL_TRUE)
		{
			const int string_length = 1024;
			GLchar log[string_length] = "";
			glGetShaderInfoLog(fragment_shader, string_length, NULL, log);
			std::cerr << log << std::endl;
		}

		result = fragment_shader;
	}

	return result;
}

tygra::Image MyView::GenerateTexture(std::string filepath, int numTex)
{
	tygra::Image texture_imageTemp = tygra::imageFromPNG(filepath);
	if (texture_imageTemp.containsData()) {
		switch (numTex)
		{
		case 1:
			glGenTextures(1, &amb_textures);
			glBindTexture(GL_TEXTURE_2D, amb_textures);
			break;
		case 2:
			glGenTextures(1, &amb_textures1);
			glBindTexture(GL_TEXTURE_2D, amb_textures1);
			break;
		case 3:
			glGenTextures(1, &amb_textures2);
			glBindTexture(GL_TEXTURE_2D, amb_textures2);
			break;
		default:
			break;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
						GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			texture_imageTemp.width(),
			texture_imageTemp.height(),
			0,
			pixel_formats[texture_imageTemp.componentsPerPixel()],
			texture_imageTemp.bytesPerComponent() == 1 ? GL_UNSIGNED_BYTE
			: GL_UNSIGNED_SHORT,
			texture_imageTemp.pixels());
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, numTex);
	}
	
	return texture_imageTemp;
}

void MyView::PerInstanceRender(SceneModel::Instance thisInstance, glm::mat4 projection_xform, glm::mat4 view_xform)
{
	//mesh via ID
	const GLMesh& meshDraw = _sponzaMesh[thisInstance.getMeshId()];

	//model matrix needs to pass in the vertex info
	glm::mat4 model_xform = glm::translate(glm::mat4(thisInstance.getTransformationMatrix()),
		glm::vec3(0.0f, 0.0f, 0.0f));

	glm::mat4 combined_xform = projection_xform
		* view_xform
		* model_xform;

	glm::vec3 material_diffuse = scene_->getMaterialById(thisInstance.getMaterialId()).getDiffuseColour(); //get the diffuse from this surface
	glm::vec3 material_specular = scene_->getMaterialById(thisInstance.getMaterialId()).getSpecularColour(); //specular
	float material_shininess = scene_->getMaterialById(thisInstance.getMaterialId()).getShininess(); //shine factor

	GLuint model_xform_uni = glGetUniformLocation(sponza_program, "model_xform");
	glUniformMatrix4fv(model_xform_uni, 1, GL_FALSE, glm::value_ptr(model_xform));

	GLuint model_xform_id = glGetUniformLocation(sponza_program, "projection_view_model_xform");
	glUniformMatrix4fv(model_xform_id, 1, GL_FALSE, glm::value_ptr(combined_xform));

	GLuint material_diffuse_uni = glGetUniformLocation(sponza_program, "material.diffuse_colour");
	glUniform3fv(material_diffuse_uni, 1, glm::value_ptr(material_diffuse));
	GLuint material_specular_uni = glGetUniformLocation(sponza_program, "material.specular_colour");
	glUniform3fv(material_specular_uni, 1, glm::value_ptr(material_specular));
	GLuint material_shininess_uni = glGetUniformLocation(sponza_program, "material.shininess");
	glUniform1f(material_shininess_uni, material_shininess);
	GLuint material_ambient_uni = glGetUniformLocation(sponza_program, "material.ambientID");

	std::string thisAmb = scene_->getMaterialById(thisInstance.getMaterialId()).getAmbientMap(); //check all the ambient string potential strings

	glActiveTexture(GL_TEXTURE0);
	if (thisAmb.compare("amb1.png") == 0) //check for possible combinations for texture strings and load correct texture
	{
		glBindTexture(GL_TEXTURE_2D, amb_textures);
		glUniform1i(material_ambient_uni, 1);
		GLuint texture_sample_id = glGetUniformLocation(sponza_program, "texture_sample");
		glUniform1i(texture_sample_id, 0);
	}
	if (thisAmb.compare("amb2.png") == 0)
	{
		glBindTexture(GL_TEXTURE_2D, amb_textures1);
		glUniform1i(material_ambient_uni, 2);
		GLuint texture_sample_id = glGetUniformLocation(sponza_program, "texture_sample");
		glUniform1i(texture_sample_id, 0);
	}
	if (thisAmb.compare("amb3.png") == 0)
	{
		glBindTexture(GL_TEXTURE_2D, amb_textures2);
		glUniform1i(material_ambient_uni, 3);
		GLuint texture_sample_id = glGetUniformLocation(sponza_program, "texture_sample");
		glUniform1i(texture_sample_id, 0);
	}

	if (thisAmb.length() == 0)//if there is no ambience string
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(material_ambient_uni, 0);
		GLuint texture_sample_id = glGetUniformLocation(sponza_program, "texture_sample");
		glUniform1i(texture_sample_id, 0);
	}

	glBindVertexArray(meshDraw.vao);


	if (getToggleState())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //use standard mode
		GLuint shader_emiss_uni = glGetUniformLocation(sponza_program, "emissive_colour"); //set emmissive colour
		glUniform3f(shader_emiss_uni, 0.0f, 0.2f, 0.0f);
		GLuint shaderBool_uni = glGetUniformLocation(sponza_program, "wireframeDraw");
		glUniform1i(shaderBool_uni, 0);
		glDrawElements(GL_TRIANGLES, meshDraw.element_count, GL_UNSIGNED_INT, 0); //just draw scene normal

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //switch to wireframe
		glUniform1i(shaderBool_uni, 1);
		glDrawElements(GL_TRIANGLES, meshDraw.element_count, GL_UNSIGNED_INT, 0); //augment on top of scene and use second type of shading in shader
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		GLuint shaderBool_uni = glGetUniformLocation(sponza_program, "wireframeDraw");
		glUniform1i(shaderBool_uni, 0);
		GLuint shader_emiss_uni = glGetUniformLocation(sponza_program, "emissive_colour"); //reset to no emissive colour
		glUniform3f(shader_emiss_uni, 0.0, 0.0, 0.0);
		glDrawElements(GL_TRIANGLES, meshDraw.element_count, GL_UNSIGNED_INT, 0); //just draw scene normal
	}
}

void MyView::toggleExtraFeature()
{
	extraToggle = !extraToggle;
}

const bool MyView::getToggleState() const
{
	return extraToggle;
}

#pragma endregion

