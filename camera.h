#pragma once


struct SCamera
{
	enum Camera_Movement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;

	glm::vec3 WorldUp;

	float Yaw;
	float Pitch;

	const float MovementSpeed = .2f;
	float MouseSensitivity = 1.f;

	float FOV;


};


void InitCamera(SCamera &in, float y, float p)
{
	in.Front = glm::vec3(0.544861f,-0.224951f,0.807789f);
	in.Position = glm::vec3(-17.0f, 10.0f, -23.0f);
	//in.Position = glm::vec3(1.5515f,6.8016f,3.5634f);
	in.Up = glm::vec3(0.0f, 1.0f, 0.0f);
	in.WorldUp = in.Up;
	in.Right = glm::normalize(glm::cross(in.Front, in.WorldUp));

	in.Yaw = y;
	in.Pitch = p;

	//in.FOV = 45.0f;
}
