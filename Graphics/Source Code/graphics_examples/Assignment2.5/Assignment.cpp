/*
 Assignment.cpp for Assignment2.5
 I updated my Assignment.cpp from the first assignment and added model improting, texturing, mouse controls
 Iain Martin October 2018 updated by John Harrow 12/22
*/

/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#else
#pragma comment(lib, "glfw3.lib")
#endif
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>
#include <stack>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "tiny_loader_texture.h"
#include "cube.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
using namespace glm;

const int NUM_PROGRAMS = 3;

GLuint program[NUM_PROGRAMS];		/* Identifiers for the shader prgorams */
GLuint current_program;
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */
GLuint emitmode;
GLuint attenuationmode;

/* Position and view globals */
GLfloat model_scale, x, z, y, vx, vy, vz, droid_x, droid_z, droid_rotation;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere object

//Variable for mouse control
GLfloat light_x, light_y, light_z, view_x, view_y, view_z, yaw7, pitch7, lastX, lastY;

/* Uniforms*/
GLuint modelID[NUM_PROGRAMS], viewID[NUM_PROGRAMS], projectionID[NUM_PROGRAMS], lightposID[NUM_PROGRAMS], normalmatrixID[NUM_PROGRAMS];
GLuint colourmodeID[NUM_PROGRAMS], emitmodeID[NUM_PROGRAMS], attenuationmodeID[NUM_PROGRAMS];

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numspherevertices;

//Variable for objects and textures
TinyObjLoader droid, garage, ship, chair, ladder, droid2, droid3, light;
GLuint texID1, texID2, texID3, texID4, texID5, texID6, texID7;

Cube aCube;
vec3 cameraPos, cameraFront, cameraUp;
GLboolean firstMouse;




bool load_texture(const char* filename, GLuint& texID, bool bGenMipmaps)
{
	glGenTextures(1, &texID);
	// local image parameters
	int width, height, nrChannels;

	/* load an image file using stb_image */
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

	// check for an error during the load process
	if (data)
	{
		// Note: this is not a full check of all pixel format types, just the most common two!
		int pixel_format = 0;
		if (nrChannels == 3)
			pixel_format = GL_RGB;
		else
			pixel_format = GL_RGBA;

		// Bind the texture ID before the call to create the texture.
			// texID[i] will now be the identifier for this specific texture
		glBindTexture(GL_TEXTURE_2D, texID);

		// Create the texture, passing in the pointer to the loaded image pixel data
		glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);

		// Generate Mip Maps
		if (bGenMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			// If mipmaps are not used then ensure that the min filter is defined
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
	}
	else
	{
		printf("stb_image  loading error: filename=%s", filename);
		return false;
	}
	stbi_image_free(data);
	return true;
}



/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	//Set the object transformation controls to their initial values 
	x = 0;
	y = 0;
	z = 0;
	vx = 0; vy = 0, vz = 4.f;
	light_x = 0; light_y = 3.25; light_z = 0;

	//Mouse controls
	yaw7 = -90.f;
	pitch7 = 0.0f;
	firstMouse = true;
	lastX = 800.0f / 2.0;
	lastY = 600.0f / 2.0;

	//droid
	droid_x = 2; droid_z = 0;
	droid_rotation = 0;

	const float roughness = 0.8;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 1; 
	emitmode = 0;
	attenuationmode = 1; // Attenuation is on by default

	//Camera movement
	cameraPos = vec3(0.0f, 1.0f, 1.0f);
	cameraFront = vec3(0.0f, 0.0f, -1.0f);
	cameraUp = vec3(0.0f, 1.0f, 0.0f);

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and create our object*/
	garage.load_obj("..\\..\\obj\\garage1.obj");
	droid.load_obj("..\\..\\obj\\untitled1.obj");
	ship.load_obj("..\\..\\obj\\tie_fighter.obj");
	chair.load_obj("..\\..\\obj\\Folding_Chair.obj");
	ladder.load_obj("..\\..\\obj\\ladder_1.obj");
	droid2.load_obj("..\\..\\obj\\droidb1.obj");
	droid3.load_obj("..\\..\\obj\\droid3.obj");
	light.load_obj("..\\..\\obj\\light.obj");



	//Currently only using program[1] fraglight
	try
	{
		program[0] = glw->LoadShader("poslight_A.vert", "poslight_A.frag");
		program[1] = glw->LoadShader("fraglight_A.vert", "fraglight_A.frag");
		program[2] = glw->LoadShader("fraglight_A.vert", "fraglight_oren_nayar_A.frag");
	}
	catch (exception &e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	//Load all the textures
	stbi_set_flip_vertically_on_load(true);
	if (!load_texture("..//..//images//droid//d_o_material_Base_Color.png", texID1, true))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}
	if (!load_texture("..//..//images//garage_texture.png", texID2, true))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}
	if (!load_texture("..//..//images//tie_texture.png", texID3, true))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}
	if (!load_texture("..//..//images//gray.jpg", texID4, true))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}
	if (!load_texture("..//..//images//droidb1-3_Base_BaseColor.png", texID5, true))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}
	if (!load_texture("..//..//images//Droid_albedo.jpeg", texID6, true))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}
	if (!load_texture("..//..//images//Material_39_baseColor.png", texID7, true))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}
	

	/* Define the same uniforms to send to both shaders */
	for (int i = 0; i < NUM_PROGRAMS; i++)
	{
		glUseProgram(program[i]);
		modelID[i] = glGetUniformLocation(program[i], "model");
		colourmodeID[i] = glGetUniformLocation(program[i], "colourmode");
		emitmodeID[i] = glGetUniformLocation(program[i], "emitmode");
		attenuationmodeID[i] = glGetUniformLocation(program[i], "attenuationmode");
		viewID[i] = glGetUniformLocation(program[i], "view");
		projectionID[i] = glGetUniformLocation(program[i], "projection");
		lightposID[i] = glGetUniformLocation(program[i], "lightpos");
		normalmatrixID[i] = glGetUniformLocation(program[i], "normalmatrix");
	}
	// Define the index which represents the current shader
	current_program = 1;

	int loc = glGetUniformLocation(program[1], "tex1");
	if (loc >= 0) glUniform1i(loc, 0);

	aCube.makeCube();
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(program[current_program]);

	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	// Apply rotations to the view position. This wil get applied to the whole scene
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view *  vec4(light_x, light_y, light_z, 1.0);

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniform1ui(colourmodeID[current_program], colourmode);
	glUniformMatrix4fv(viewID[current_program], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID[current_program], 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID[current_program], 1, &lightpos[0]);
	glUniform1ui(attenuationmodeID[current_program], attenuationmode);

	
	//Light Scouce Cube
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		model.top() = scale(model.top(), vec3(0.9f, 0.125f, 0.25f)); // make a small sphere
																	 // Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		// Draw our lightposition sphere  with emit mode on
		emitmode = 1;
		glUniform1ui(emitmodeID[current_program], emitmode);
		aCube.drawCube(drawmode);
		emitmode = 0;
		glUniform1ui(emitmodeID[current_program], emitmode);
	}
	model.pop();
	
	

	// Define the global model transformations (rotate and scale). Note, we're not modifying the light source position

	model.top() = translate(model.top(), vec3(x, y, z));
	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	
	// Droid
	model.push(model.top());
	{
		// Define the model transformations for the droid
		model.top() = translate(model.top(), vec3(droid_x, y + 0.135, droid_z));
		model.top() = scale(model.top(), vec3(0.5f, 0.5f, 0.5f));
		model.top() = rotate(model.top(), -radians(droid_rotation), glm::vec3(0, 1, 0)); //rotating around y-axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		//Bind the texture and draw
		glBindTexture(GL_TEXTURE_2D, texID1);
		droid.drawObject(drawmode);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	model.pop();
	
	//Garage
	model.push(model.top());
	{
		// Define the model transformations for the garage
		model.top() = translate(model.top(), vec3(x + 1, y, z)); 
		model.top() = scale(model.top(), vec3(0.75f, 0.75f, 0.75f));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		//Bind the texture and draw
		glBindTexture(GL_TEXTURE_2D, texID2);
		garage.drawObject(drawmode);
		glBindTexture(GL_TEXTURE_2D, 0);
		
	}
	model.pop();
	
	//Ship
	model.push(model.top());
	{
		// Define the model transformations for the ship
		model.top() = translate(model.top(), vec3(x, y + 0.455, z - 1));
		model.top() = scale(model.top(), vec3(1.5f, 1.5f, 1.5f));
		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(0, 1, 0)); //rotating around y-axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		//Bind the texture and draw
		glBindTexture(GL_TEXTURE_2D, texID3);
		ship.drawObject(drawmode);
		glBindTexture(GL_TEXTURE_2D, 0);

	}
	model.pop();

	//Ladder
	model.push(model.top());
	{
		// Define the model transformations for the ladder
		model.top() = translate(model.top(), vec3(x, y + 0.01, z + 1.5));
		model.top() = scale(model.top(), vec3(0.1f, 0.1f, 0.1f));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		//Bind the texture and draw
		glBindTexture(GL_TEXTURE_2D, texID4);
		ladder.drawObject(drawmode);
		glBindTexture(GL_TEXTURE_2D, 0);

	}
	model.pop();

	//Chair
	model.push(model.top());
	{
		// Define the model transformations for the chair
		model.top() = translate(model.top(), vec3(x + 1.5, y + 0.05, z + 2.25));
		model.top() = scale(model.top(), vec3(0.3f, 0.3f, 0.3f));
		model.top() = rotate(model.top(), -radians(135.f), glm::vec3(0, 1, 0)); //rotating around y-axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		//Bind the texture and draw
		glBindTexture(GL_TEXTURE_2D, texID4);
		chair.drawObject(drawmode);
		glBindTexture(GL_TEXTURE_2D, 0);

	}
	model.pop();

	//Droid2
	model.push(model.top());
	{
		// Define the model transformations for the droid
		model.top() = translate(model.top(), vec3(x + 1.5, y + 0.8, z - 2.25));
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f));
		model.top() = rotate(model.top(), -radians(45.f), glm::vec3(0, 1, 0)); //rotating around y-axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		//Bind the texture and draw
		glBindTexture(GL_TEXTURE_2D, texID5);
		droid2.drawObject(drawmode);
		glBindTexture(GL_TEXTURE_2D, 0);

	}
	model.pop();

	//Droid3
	model.push(model.top());
	{
		// Define the model transformations for the droid
		model.top() = translate(model.top(), vec3(x - 0.5, y + 0.23, z - 2));
		model.top() = scale(model.top(), vec3(20.f, 20.f, 20.f));
		model.top() = rotate(model.top(), -radians(45.f), glm::vec3(0, 1, 0)); //rotating around y-axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		//Bind the texture and draw
		glBindTexture(GL_TEXTURE_2D, texID6);
		droid3.drawObject(drawmode);
		glBindTexture(GL_TEXTURE_2D, 0);

	}
	model.pop();

	//Light Fitting
	model.push(model.top());
	{
		// Define the model transformations for the light
		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		model.top() = scale(model.top(), vec3(0.005f, 0.005f, 0.005f));
		model.top() = rotate(model.top(), -radians(90.f), glm::vec3(0, 1, 0)); //rotating around y-axis
		model.top() = rotate(model.top(), -radians(180.f), glm::vec3(1, 0, 0)); //rotating around y-axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID[current_program], 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID[current_program], 1, GL_FALSE, &normalmatrix[0][0]);

		//Bind the texture and draw
		glBindTexture(GL_TEXTURE_2D, texID7);
		light.drawObject(drawmode);
		glBindTexture(GL_TEXTURE_2D, 0);

	}
	model.pop();
	
	glDisableVertexAttribArray(0);
	glUseProgram(0);

	
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f*4.f) / ((float)h / 480.f*3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
//	if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//camera
	const float cameraSpeed = 0.05f;
	if (key == 'W') cameraPos += cameraSpeed * cameraFront;
	if (key == 'A') cameraPos -= normalize(cross(cameraFront, cameraUp)) * cameraSpeed;;
	if (key == 'S') cameraPos -= cameraSpeed * cameraFront;
	if (key == 'D') cameraPos += normalize(cross(cameraFront, cameraUp)) * cameraSpeed;;

	//Droid movement
	if (key == 'T') {
		droid_x += 0.05f;
		droid_rotation = -90.f;
	}
	if (key == 'G') {
		droid_x -= 0.05f;
		droid_rotation = 90.f;
	}
	if (key == 'H') {
		droid_z += 0.05f;
		droid_rotation = 0.f;
	}
	if (key == 'F') {
		droid_z -= 0.05f;
		droid_rotation = -180.f;
	}

	if (key == '1') light_x -= 0.05f;
	if (key == '2') light_x += 0.05f;
	if (key == '3') light_y -= 0.05f;
	if (key == '4') light_y += 0.05f;
	if (key == '5') light_z -= 0.05f;
	if (key == '6') light_z += 0.05f;

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}

	
}


//I used a tutorial to get this working: https://learnopengl.com/Getting-started/Camera
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw7 += xoffset;
	pitch7 += yoffset;

	if (pitch7 > 89.0f)
		pitch7 = 89.0f;
	if (pitch7 < -89.0f)
		pitch7 = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw7)) * cos(glm::radians(pitch7));
	direction.y = sin(glm::radians(pitch7));
	direction.z = sin(glm::radians(yaw7)) * cos(glm::radians(pitch7));
	cameraFront = glm::normalize(direction);
}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Lab2: Hello 3D");;

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD. Exiting." << endl;
		return -1;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setMouseCallback(mouse_callback);
	glw->setReshapeCallback(reshape);

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}





