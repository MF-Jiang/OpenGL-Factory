#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>

#include "window.h"
#include "texture.h"
#include "camera.h"
#include "ModelViewerCamera.h"
#include "shader.h"

// Button Control
bool LRefresh = true;
bool TRefresh = true;

// Control scenario inversion
bool invertObjects = false;

// Start Control Box
enum ControlBoxState
{
	off,
	AllGreen,
	AllRed,
	HalfHalf,
};
ControlBoxState CurrentBox = off;

// Car Speed
float CurrentSpeed = 0.005f;
float CarSpeed;
glm::vec3 CarPosition = glm::vec3(0.f,0.f,0.f);

// Blower Fan
float rotationAngle = 0.0f;

// Pipe Scale Factor
float initialScaleFactor = 1.0f;
float scaleFactor = initialScaleFactor;
float scaleFactorAmplitude = 0.02f;

// Straight light variable:
glm::vec3 lightDirection = glm::vec3(0.1f, -.81f, -.61f);
glm::vec3 lightPos = glm::vec3(2.f, 6.f, 7.f);
// Sun light variable:
glm::vec3 SunPos = glm::vec3(-3.84888f, 15.3776f, -21.5044f);
glm::vec3 SunColour = glm::vec3(0.5f, 0.f, 0.f); 

SCamera Camera;

// Control Box Model Vector
std::vector<float> CBoxVector;
std::vector<float> CBoxSign;
std::vector<float> CBoxBlue;
std::vector<float> CBoxBlack;
std::vector<float> CBoxRed;
std::vector<float> CBoxGreen;
std::vector<float> CBoxFace;

// Heater Model Vector
std::vector<float> heaterVector;
std::vector<float> heaterTrailer;
std::vector<float> heaterBase;
std::vector<float> heaterEdge;
std::vector<float> heaterHandle;
std::vector<float> heaterDoor;

// Pipe Model Vector
std::vector<float> Pipe;
std::vector<float> PipeAirOut;
std::vector<float> PipeNail;

// Pump Model Vector
std::vector<float> pumpVector;
std::vector<float> pumpBase;
std::vector<float> pumpOutAir;

// Car Model Vector
std::vector<float> CarTerrface;
std::vector<float> CarVector;
std::vector<float> CarWheel;

// Blower Model Vector
std::vector<float> BlowerVector;
std::vector<float> BlowerBase;
std::vector<float> BlowerFan;

/**
 * @brief Reads an OBJ file and extracts vertex, normal, and UV data.
 *
 * @param filePath The path to the OBJ file.
 * @return A vector containing the vertex data (interleaved with color, normal, and UV).
 */
std::vector<float> ReadObjFile(const std::string& filePath) 
{
	// Open the OBJ file
	std::ifstream objFile(filePath);
	// Vector to store the final vertex data
	std::vector<float> vertices;
	std::string line;

	// Separate vectors to store indices for vertices, UVs, and normals
	std::vector<int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices; // Temporary storage for vertices
	std::vector<glm::vec2> temp_uvs; // Temporary storage for UVs
	std::vector<glm::vec3> temp_normals; // Temporary storage for normals

	// Check if the file is open
	if (!objFile.is_open()) {
		std::cerr << "Failed to open file: " << filePath << std::endl;
		return vertices; // Return an empty vector if the file cannot be opened
	}
	// Read each line of the OBJ file
	while (getline(objFile, line)) 
	{
		std::istringstream iss(line);
		std::string lineHeader;
		iss >> lineHeader;
		// Process vertex data
		if (lineHeader == "v") 
		{

			glm::vec3 vertex;
			if (iss >> vertex.x >> vertex.y >> vertex.z) 
			{
				temp_vertices.push_back(vertex);
			}
		}
		// Process UV data
		else if (lineHeader == "vt") 
		{ 
			glm::vec2 uv;
			if (iss >> uv.x >> uv.y) 
			{
				temp_uvs.push_back(uv);
			}
		}
		// Process normal data
		else if (lineHeader == "vn") 
		{
			glm::vec3 normal;
			if (iss >> normal.x >> normal.y >> normal.z) 
			{
				temp_normals.push_back(normal);
			}
		}
		// Process face data
		else if (lineHeader == "f") 
		{
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			char slash;
			for (int i = 0; i < 3; i++) 
			{
				iss >> vertexIndex[i] >> slash >> uvIndex[i] >> slash >> normalIndex[i];
				vertexIndices.push_back(vertexIndex[i]);
				uvIndices.push_back(uvIndex[i]);
				normalIndices.push_back(normalIndex[i]);
			}
		}
	}
	// Set a default color
	glm::vec3 defaultColor(1.0f, 1.0f, 1.0f);
	// Process the indices to construct the final vertex data
	for (size_t i = 0; i < vertexIndices.size(); i++) 
	{
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the vertex, uv, and normal data using the indices
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Add vertex data to the vertices vector
		vertices.push_back(vertex.x);
		vertices.push_back(vertex.y);
		vertices.push_back(vertex.z);

		// Add color data
		vertices.push_back(defaultColor.x);
		vertices.push_back(defaultColor.y);
		vertices.push_back(defaultColor.z);

		// Add normal data
		vertices.push_back(normal.x);
		vertices.push_back(normal.y);
		vertices.push_back(normal.z);

		vertices.push_back(uv.x);
		vertices.push_back(uv.y);
	}
	return vertices; // Return the final vector containing vertex data
}
/**
 * @brief Processes keyboard input to control camera movement and other actions.
 *
 * @param window The GLFW window object.
 */
void processKeyboard(GLFWwindow* window)
{
	glfwPollEvents();
	// Check if the Escape key is pressed to close the window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	// Set light direction and position to camera's front and position when Space key is pressed
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		lightDirection = Camera.Front;
		lightPos = Camera.Position;
	}

	bool cam_changed = false;
	float x = 0.f, y = 0.f;
	// Check if the camera is locked for manual control
	if (lockedCamera)
	{
		// Check arrow keys for camera movement
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			x = 0.f;
			y = 1.f;
			cam_changed = true;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			x = 0.f;
			y = -1.f;
			cam_changed = true;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			x = -1.f;
			y = 0.f;
			cam_changed = true;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			x = 1.f;
			y = 0.f;
			cam_changed = true;
		}
		// Adjust camera distance with R and F keys
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		{
			cam_dist -= 0.1 * Camera.MovementSpeed;
			cam_changed = true;
		}
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		{
			cam_dist += 0.1 * Camera.MovementSpeed;
			cam_changed = true;
		}
	}
	// Toggle locked camera mode with the L key
	if (LRefresh && glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		if (lockedCamera)
		{
			lockedCamera = false;
		}
		else
		{
			lockedCamera = true;
		}
		LRefresh = false;
		//std::cout << lockedCamera << std::endl;
		InitCamera(Camera, 56, -13);
		cam_dist = 24.8545f;
		MoveAndOrientCamera(Camera, glm::vec3(0.621314f, 03.25153f, -6.82311f), cam_dist, 0.f, 0.f);
		//std::cout << Camera.Yaw << std::endl;
	}
	if (LRefresh == false && glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
	{
		LRefresh = true;
	}
	// Toggle object inversion with the T key
	if (TRefresh && glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		TRefresh = false;
		invertObjects = !invertObjects;
	}
	if (TRefresh == false && glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
	{
		TRefresh = true;
	}

	float deltaTime = 0.1f;  // Adjust movement speed, modify as needed
	// Control camera movement when not in locked mode
	if (lockedCamera == false)
	{
		// Check W key for forward movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			Camera.Position += deltaTime * Camera.MovementSpeed * Camera.Front;
			//cam_changed = true;
		}
		// Check S key for backward movement
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			Camera.Position -= deltaTime * Camera.MovementSpeed * Camera.Front;
			//cam_changed = true;
		}
		// Check A key for leftward movement
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			Camera.Position -= deltaTime * Camera.MovementSpeed * Camera.Right;
			//cam_changed = true;
		}
		// Check D key for rightward movement
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			Camera.Position += deltaTime * Camera.MovementSpeed * Camera.Right;
			//cam_changed = true;
		}
	}
	// Apply camera movement and orientation changes if needed
	if (cam_changed)
	{
		MoveAndOrientCamera(Camera, glm::vec3(0.621314f, 03.25153f, -6.82311f), cam_dist, x, y);
	}
}
/**
 * @brief Callback function for mouse movement, controlling camera orientation.
 *
 * @param window The GLFW window object.
 * @param xpos The X position of the mouse cursor.
 * @param ypos The Y position of the mouse cursor.
 */
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	// Check if the camera is not locked for manual control
	if (lockedCamera == false)
	{
		static bool firstMouse = true;
		static float lastX = 1920.0f / 2, lastY = 1080.0f / 2;
		// Initialize last mouse position on the first call
		if (firstMouse)
		{
			lastX = static_cast<float>(xpos);
			lastY = static_cast<float>(ypos);
			firstMouse = false;
		}
		// Calculate mouse offset
		float xoffset = static_cast<float>(xpos) - lastX;
		float yoffset = lastY - static_cast<float>(ypos);

		lastX = static_cast<float>(xpos);
		lastY = static_cast<float>(ypos);

		// Call the camera movement and orientation function
		MoveAndOrientCamera(Camera, glm::vec3(0.621314f, 03.25153f, -6.82311f), cam_dist, xoffset, yoffset);
	}
}
/**
 * @brief Callback function for mouse button events, handling interaction with control boxes.
 *
 * @param window The GLFW window object.
 * @param button The mouse button that triggered the event.
 * @param action The action performed (press or release).
 * @param mods Modifier keys held down during the action.
 */
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	// Check if the left mouse button is pressed
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		// Toggle through different control box states based on the current state
		if (CurrentBox == off)
		{
			CurrentBox = HalfHalf;
			std::cout << "Control box on" << std::endl;
		}
		else if (CurrentBox == HalfHalf)
		{
			CurrentBox = AllGreen;
			std::cout << "Green Moudle" << std::endl;
		}
		else if (CurrentBox == AllGreen)
		{
			CurrentBox = AllRed;
			std::cout << "Red Moudle" << std::endl;
		}
		else 
		{
			CurrentBox = HalfHalf;
			std::cout << "Middle Moudle" << std::endl;
		}
	}
	// Check if the right mouse button is pressed
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		// Turn off the control box if it is currently active
		if (CurrentBox != off)
		{
			CurrentBox = off;
			std::cout << "Control box off" << std::endl;
		}
	}
}

int main(int argc, char** argv)
{
	// Create a GLFW window
	GLFWwindow* window = CreateGLFWWindow(1920, 1080, "20320552");
	// Load OpenGL function pointers
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	// Load shader program
	unsigned int shaderProgram = LoadShader("phong.vert", "phong.frag");
	// Initialize camera
	InitCamera(Camera, 56, -13);
	cam_dist = 24.8545f;
	MoveAndOrientCamera(Camera, glm::vec3(0.621314f, 03.25153f, -6.82311f), cam_dist, 0.f, 0.f);
	lightDirection = Camera.Front;
	lightPos = Camera.Position;

	// Control Box Model
	CBoxVector = ReadObjFile("resources/Box.obj");
	CBoxSign = ReadObjFile("resources/BoxSign.obj");
	CBoxBlue = ReadObjFile("resources/BoxBlue.obj");
	CBoxBlack = ReadObjFile("resources/BoxBlack.obj");
	CBoxRed = ReadObjFile("resources/BoxRed.obj");
	CBoxGreen = ReadObjFile("resources/BoxGreen.obj");
	CBoxFace = ReadObjFile("resources/BoxFace.obj");
	// Control Box Texture
	GLuint CBoxtexture = setup_texture("resources/bmp/Box.bmp");
	GLuint CBoxSigntexture = setup_texture("resources/bmp/Pump.bmp");
	GLuint CBoxBluetexture = setup_texture("resources/bmp/BoxBlue.bmp");
	GLuint CBoxBlacktexture = setup_texture("resources/bmp/Black.bmp");
	GLuint CBoxRedtexture = setup_texture("resources/bmp/Red.bmp");
	GLuint CBoxGreentexture = setup_texture("resources/bmp/Green.bmp");
	GLuint CBoxFacetexture = setup_texture("resources/bmp/BoxFace.bmp");

	// Heater Model
	heaterVector = ReadObjFile("resources/Heater.obj");
	heaterTrailer = ReadObjFile("resources/HeaterTrailer.obj");
	heaterBase = ReadObjFile("resources/HeaterBase.obj");
	heaterEdge = ReadObjFile("resources/HeaterEdge.obj");
	heaterHandle = ReadObjFile("resources/HeaterHandle.obj");
	heaterDoor = ReadObjFile("resources/HeaterDoor.obj");
	// Heater Texture
	GLuint heaterTexture = setup_texture("resources/bmp/Pump.bmp");
	GLuint heaterTrailerTexture = setup_texture("resources/bmp/BlowerBase.bmp");
	GLuint heaterBaseTexture = setup_texture("resources/bmp/HeaterBase.bmp");
	GLuint heaterEdgeTexture = setup_texture("resources/bmp/Edge.bmp");
	GLuint heaterHandleTexture = setup_texture("resources/bmp/Blower.bmp");
	GLuint heaterDoorTexture = setup_texture("resources/bmp/Box.bmp");

	// Pipe Model
	Pipe = ReadObjFile("resources/Pipe.obj");
	PipeAirOut = ReadObjFile("resources/PipeAirOut.obj");
	PipeNail = ReadObjFile("resources/PipeNail.obj");
	// Pipe Texture
	GLuint PipeTexture = setup_texture("resources/bmp/Pipe.bmp");
	GLuint PipeAirOutTexture = setup_texture("resources/bmp/White.bmp");
	GLuint PipeNailTexture = setup_texture("resources/bmp/Black.bmp");

	// Pump Model
	pumpVector = ReadObjFile("resources/pump.obj");
	pumpBase = ReadObjFile("resources/pumpBase.obj");
	pumpOutAir = ReadObjFile("resources/pumpOutAir.obj");
	// Pump Texture
	GLuint pumpTexture = setup_texture("resources/bmp/Pump.bmp");
	GLuint pumpBaseTexture = setup_texture("resources/bmp/Black.bmp");
	GLuint pumpOutAirTexture = setup_texture("resources/bmp/White.bmp");

	// Car Model
	CarTerrface = ReadObjFile("resources/CarTerrface.obj");
	CarVector = ReadObjFile("resources/Car.obj");
	CarWheel = ReadObjFile("resources/CarWheel.obj");
	// Car Texture
	GLuint CarTerrfaceTexture = setup_texture("resources/bmp/CarTerrface.bmp");
	GLuint CarTexture = setup_texture("resources/bmp/CarBase.bmp");
	GLuint CarWheelTexture = setup_texture("resources/bmp/Wheel.bmp");

	// Blower Model
	BlowerVector = ReadObjFile("resources/Blower.obj");
	BlowerBase = ReadObjFile("resources/BlowerBase.obj");
	BlowerFan = ReadObjFile("resources/BlowerFan.obj");
	// Blower Texture
	GLuint BlowerTexture = setup_texture("resources/bmp/Blower.bmp");
	GLuint BlowerBaseTexture = setup_texture("resources/bmp/BlowerBase.bmp");
	GLuint BlowerFanTexture = setup_texture("resources/bmp/Wheel.bmp");

	// Generate Vertex Array Objects and Vertex Buffer Objects
	GLuint VAOs[25], VBOs[25];
	glGenVertexArrays(25, VAOs);
	glGenBuffers(25, VBOs);

	// Setup pump model
	// pump Part
	glBindVertexArray(VAOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	glBufferData(GL_ARRAY_BUFFER, pumpVector.size() * sizeof(float), pumpVector.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// pump Base Part
	glBindVertexArray(VAOs[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
	glBufferData(GL_ARRAY_BUFFER, pumpBase.size() * sizeof(float), pumpBase.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// pump OutAir Part
	glBindVertexArray(VAOs[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
	glBufferData(GL_ARRAY_BUFFER, pumpOutAir.size() * sizeof(float), pumpOutAir.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	// Setup heater model
	// heater Part
	glBindVertexArray(VAOs[3]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
	glBufferData(GL_ARRAY_BUFFER, heaterVector.size() * sizeof(float), heaterVector.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// heater Trailer Part
	glBindVertexArray(VAOs[4]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[4]);
	glBufferData(GL_ARRAY_BUFFER, heaterTrailer.size() * sizeof(float), heaterTrailer.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// heater Base Part
	glBindVertexArray(VAOs[5]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[5]);
	glBufferData(GL_ARRAY_BUFFER, heaterBase.size() * sizeof(float), heaterBase.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// heater Edge Part
	glBindVertexArray(VAOs[6]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[6]);
	glBufferData(GL_ARRAY_BUFFER, heaterEdge.size() * sizeof(float), heaterEdge.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// heater Handle Part
	glBindVertexArray(VAOs[7]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[7]);
	glBufferData(GL_ARRAY_BUFFER, heaterHandle.size() * sizeof(float), heaterHandle.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// heater Door Part
	glBindVertexArray(VAOs[8]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[8]);
	glBufferData(GL_ARRAY_BUFFER, heaterDoor.size() * sizeof(float), heaterDoor.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	// Setup Blower model
	// Blower Part
	glBindVertexArray(VAOs[9]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[9]);
	glBufferData(GL_ARRAY_BUFFER, BlowerVector.size() * sizeof(float), BlowerVector.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	//Blower Base Part
	glBindVertexArray(VAOs[10]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[10]);
	glBufferData(GL_ARRAY_BUFFER, BlowerBase.size() * sizeof(float), BlowerBase.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	//Blower Fan Part
	glBindVertexArray(VAOs[24]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[24]);
	glBufferData(GL_ARRAY_BUFFER, BlowerFan.size() * sizeof(float), BlowerFan.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	// Setup Car model
	// CarBase Part
	glBindVertexArray(VAOs[11]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[11]);
	glBufferData(GL_ARRAY_BUFFER, CarVector.size() * sizeof(float), CarVector.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Car Terrface Part
	glBindVertexArray(VAOs[12]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[12]);
	glBufferData(GL_ARRAY_BUFFER, CarTerrface.size() * sizeof(float), CarTerrface.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Car Wheel Part
	glBindVertexArray(VAOs[13]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[13]);
	glBufferData(GL_ARRAY_BUFFER, CarWheel.size() * sizeof(float), CarWheel.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	// Setup Control Box model
	// Box Part
	glBindVertexArray(VAOs[14]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[14]);
	glBufferData(GL_ARRAY_BUFFER, CBoxVector.size() * sizeof(float), CBoxVector.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Box Sign Part
	glBindVertexArray(VAOs[15]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[15]);
	glBufferData(GL_ARRAY_BUFFER, CBoxSign.size() * sizeof(float), CBoxSign.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Box Blue Part
	glBindVertexArray(VAOs[16]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[16]);
	glBufferData(GL_ARRAY_BUFFER, CBoxBlue.size() * sizeof(float), CBoxBlue.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Box Black Part
	glBindVertexArray(VAOs[17]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[17]);
	glBufferData(GL_ARRAY_BUFFER, CBoxBlack.size() * sizeof(float), CBoxBlack.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Box Red Part
	glBindVertexArray(VAOs[18]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[18]);
	glBufferData(GL_ARRAY_BUFFER, CBoxRed.size() * sizeof(float), CBoxRed.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Box Green Part
	glBindVertexArray(VAOs[19]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[19]);
	glBufferData(GL_ARRAY_BUFFER, CBoxGreen.size() * sizeof(float), CBoxGreen.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Box Face Part
	glBindVertexArray(VAOs[20]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[20]);
	glBufferData(GL_ARRAY_BUFFER, CBoxFace.size() * sizeof(float), CBoxFace.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);


	// Setup Pipe model
	// Pipe Part
	glBindVertexArray(VAOs[21]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[21]);
	glBufferData(GL_ARRAY_BUFFER, Pipe.size() * sizeof(float), Pipe.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Pipe AirOut Part
	glBindVertexArray(VAOs[22]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[22]);
	glBufferData(GL_ARRAY_BUFFER, PipeAirOut.size() * sizeof(float), PipeAirOut.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// Pipe Nail Part
	glBindVertexArray(VAOs[23]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[23]);
	glBufferData(GL_ARRAY_BUFFER, PipeNail.size() * sizeof(float), PipeNail.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	// Enable depth testing
	glEnable(GL_DEPTH_TEST);
	// Use the shader program
	glUseProgram(shaderProgram);
	//Anti aliasing
	glEnable(GL_MULTISAMPLE);

	while (!glfwWindowShouldClose(window))
	{
		// Process keyboard input
		processKeyboard(window);
		// Set callbacks for mouse movement and mouse button events
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		// Clear the screen
		glClearColor(0.15f, 0.15f, 0.15f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// Set uniform values for lighting and camera position
		glUniform3f(glGetUniformLocation(shaderProgram, "lightDirection"), lightDirection.x, lightDirection.y, lightDirection.z);
		glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shaderProgram, "lightColour"), 10.f, 10.f, 10.f);
		glUniform3f(glGetUniformLocation(shaderProgram, "camPos"), Camera.Position.x, Camera.Position.y, Camera.Position.z);
		glUniform3f(glGetUniformLocation(shaderProgram, "SunPos"), SunPos.x, SunPos.y, SunPos.z); 
		glUniform3f(glGetUniformLocation(shaderProgram, "SunColour"), SunColour.x, SunColour.y, SunColour.z);
		// Update car position and adjust speed based on the type of box
		CarPosition += glm::vec3(1.0f, 0.0f, 0.0f) * CarSpeed;
		if (CurrentBox == AllGreen)
		{
			float carDirection = CarSpeed / CurrentSpeed;
			CurrentSpeed = 0.01f;
			CarSpeed = CurrentSpeed * carDirection;
		}
		else if (CurrentBox == AllRed)
		{
			float carDirection = CarSpeed / CurrentSpeed;
			CurrentSpeed = 0.0025f;
			CarSpeed = CurrentSpeed * carDirection;
		}
		else
		{
			float carDirection = CarSpeed / CurrentSpeed;
			CurrentSpeed = 0.005f;
			CarSpeed = CurrentSpeed * carDirection;
		}
		// Handle car movement boundaries
		if (CarPosition.x >= 7.0f)
		{
			CarSpeed = -1 * CurrentSpeed;
		}
		if(CarPosition.x<=0)
		{
			CarSpeed = 1 * CurrentSpeed;
		}
		// Set view and projection matrices for rendering
		glm::mat4 view = glm::mat4(1.f);
		view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glm::mat4 projection = glm::mat4(1.f);
		projection = glm::perspective(glm::radians(45.f), (float)1920 / (float)1080, .1f, 100.f);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Rendering Pump Model
		glBindVertexArray(VAOs[0]);
		glBindTexture(GL_TEXTURE_2D, pumpTexture);
		glm::mat4 model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		if (CurrentBox == HalfHalf)
		{
			// Pump trembling
			float jitterX = 0.001f * (rand() % 100 - 50); 
			float jitterY = 0.001f * (rand() % 100 - 50);
			float jitterZ = 0.001f * (rand() % 100 - 50);
			model = glm::translate(model, glm::vec3(jitterX, jitterY, jitterZ));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, pumpVector.size());

		glBindVertexArray(VAOs[1]);
		glBindTexture(GL_TEXTURE_2D, pumpBaseTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		if (CurrentBox == HalfHalf)
		{
			// Pump trembling
			float jitterX = 0.001f * (rand() % 100 - 50);
			float jitterY = 0.001f * (rand() % 100 - 50);
			float jitterZ = 0.001f * (rand() % 100 - 50);
			model = glm::translate(model, glm::vec3(jitterX, jitterY, jitterZ));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, pumpBase.size());

		glBindVertexArray(VAOs[2]);
		glBindTexture(GL_TEXTURE_2D, pumpOutAirTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		if (CurrentBox == AllGreen)
		{
			// Pump out the air outlet
			float currentTime = glfwGetTime();
			float oscillation = sin(currentTime);
			scaleFactor = initialScaleFactor + oscillation * scaleFactorAmplitude;
			model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
		}
		if (CurrentBox == HalfHalf)
		{
			// Pump trembling
			float jitterX = 0.001f * (rand() % 100 - 50);
			float jitterY = 0.001f * (rand() % 100 - 50);
			float jitterZ = 0.001f * (rand() % 100 - 50);
			model = glm::translate(model, glm::vec3(jitterX, jitterY, jitterZ));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, pumpOutAir.size());

		// Rendering Heater Model
		glBindVertexArray(VAOs[3]);
		glBindTexture(GL_TEXTURE_2D, heaterTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, heaterVector.size());

		glBindVertexArray(VAOs[4]);
		glBindTexture(GL_TEXTURE_2D, heaterTrailerTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, heaterTrailer.size());

		glBindVertexArray(VAOs[5]);
		glBindTexture(GL_TEXTURE_2D, heaterBaseTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, heaterBase.size());

		glBindVertexArray(VAOs[6]);
		glBindTexture(GL_TEXTURE_2D, heaterEdgeTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, heaterEdge.size());

		glBindVertexArray(VAOs[7]);
		glBindTexture(GL_TEXTURE_2D, heaterHandleTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		if (CurrentBox == AllRed)
		{
			// Open the Heater
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.f, 1.f, 0.f));
			model = glm::translate(model, glm::vec3(10.f, 0.f, 6.f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, heaterHandle.size());

		glBindVertexArray(VAOs[8]);
		glBindTexture(GL_TEXTURE_2D, heaterDoorTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		if (CurrentBox == AllRed)
		{
			// Open the Heater
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.f, 1.f, 0.f));
			model = glm::translate(model, glm::vec3(9.5f, 0.f, 4.f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, heaterDoor.size());

		//Rendering Blower Model 
		glBindVertexArray(VAOs[9]);
		glBindTexture(GL_TEXTURE_2D, BlowerTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, BlowerVector.size());

		glBindVertexArray(VAOs[10]);
		glBindTexture(GL_TEXTURE_2D, BlowerBaseTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, BlowerBase.size());

		glBindVertexArray(VAOs[24]);
		glBindTexture(GL_TEXTURE_2D, BlowerFanTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		if (CurrentBox == AllGreen)
		{
			// Rotate the Fan
			model = glm::translate(model, glm::vec3(1.31855f, 1.46722f, 1.0938755f));
			float currentTime = glfwGetTime();
			rotationAngle += currentTime;
			if (rotationAngle > 360.0f)
			{
				rotationAngle -= 360.0f;
			}
			//std::cout << rotationAngle << std::endl;
			model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::translate(model, glm::vec3(-1.31855f, -1.46722f, -1.0938755f));
			
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, BlowerFan.size());

		//Rendering  Car Model 
		glBindVertexArray(VAOs[11]);
		glBindTexture(GL_TEXTURE_2D, CarTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
			// Continuous motion
			model = glm::translate(model, CarPosition);
			
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
			// Continuous motion
			model = glm::translate(model, CarPosition);
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, CarVector.size());

		glBindVertexArray(VAOs[12]);
		glBindTexture(GL_TEXTURE_2D, CarTerrfaceTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
			// Continuous motion
			model = glm::translate(model, CarPosition);
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
			// Continuous motion
			model = glm::translate(model, CarPosition);
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, CarTerrface.size());

		glBindVertexArray(VAOs[13]);
		glBindTexture(GL_TEXTURE_2D, CarWheelTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
			// Continuous motion
			model = glm::translate(model, CarPosition);
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
			// Continuous motion
			model = glm::translate(model, CarPosition);
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, CarWheel.size());

		//Rendering Control Box Model 
		glBindVertexArray(VAOs[14]);
		glBindTexture(GL_TEXTURE_2D, CBoxtexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, CBoxVector.size());

		glBindVertexArray(VAOs[15]);
		glBindTexture(GL_TEXTURE_2D, CBoxSigntexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, CBoxSign.size());

		glBindVertexArray(VAOs[16]);
		// Update texture based on current box state
		if (CurrentBox!=off)
		{
			glBindTexture(GL_TEXTURE_2D, CBoxBluetexture);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, CBoxBlacktexture);
		}	
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, CBoxBlue.size());

		glBindVertexArray(VAOs[17]);
		glBindTexture(GL_TEXTURE_2D, CBoxBlacktexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, CBoxBlack.size());

		glBindVertexArray(VAOs[18]);
		// Update texture based on current box state
		if (CurrentBox == AllRed || CurrentBox == HalfHalf)
		{
			glBindTexture(GL_TEXTURE_2D, CBoxRedtexture);
		}
		else if (CurrentBox == AllGreen)
		{
			glBindTexture(GL_TEXTURE_2D, CBoxGreentexture);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, CBoxBlacktexture);
		}
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, CBoxRed.size());

		glBindVertexArray(VAOs[19]);
		// Update texture based on current box state
		if (CurrentBox == AllGreen || CurrentBox == HalfHalf)
		{
			glBindTexture(GL_TEXTURE_2D, CBoxGreentexture);
		}
		else if (CurrentBox == AllRed)
		{
			glBindTexture(GL_TEXTURE_2D, CBoxRedtexture);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, CBoxBlacktexture);
		}
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, CBoxGreen.size());

		glBindVertexArray(VAOs[20]);
		glBindTexture(GL_TEXTURE_2D, CBoxFacetexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, CBoxFace.size());

		//Rendering Pipe Model 
		glBindVertexArray(VAOs[21]);
		glBindTexture(GL_TEXTURE_2D, PipeTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		if (CurrentBox == AllGreen)
		{
			// Pipe transport gas
			float currentTime = glfwGetTime();
			float oscillation = sin(currentTime);
			scaleFactor = initialScaleFactor + oscillation * scaleFactorAmplitude;
			model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, Pipe.size());

		glBindVertexArray(VAOs[22]);
		glBindTexture(GL_TEXTURE_2D, PipeAirOutTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		if (CurrentBox == AllGreen)
		{
			// Pipe transport gas
			float currentTime = glfwGetTime();
			float oscillation = sin(currentTime);
			scaleFactor = initialScaleFactor + oscillation * scaleFactorAmplitude;
			model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, PipeAirOut.size());

		glBindVertexArray(VAOs[23]);
		glBindTexture(GL_TEXTURE_2D, PipeNailTexture);
		model = glm::mat4(1.f);
		if (invertObjects)
		{
			// Scene inversion
			model = glm::scale(model, glm::vec3(1.0f, -1.0f, 1.0f));
		}
		else
		{
			// Scene alignment
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		}
		if (CurrentBox == AllGreen)
		{
			// Pipe transport gas
			float currentTime = glfwGetTime();
			float oscillation = sin(currentTime);
			scaleFactor = initialScaleFactor + oscillation * scaleFactorAmplitude;
			model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
		}
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, PipeNail.size());


		glBindVertexArray(0);

		glfwSwapBuffers(window);

	}

	glfwTerminate();

	return 0;
}

