#pragma once

#include <stdio.h>
#include <glm/glm.hpp>
#include "camera.h"

float cam_dist;
bool lockedCamera = true;

void MoveAndOrientCamera(SCamera& in, glm::vec3 target, float distance, float xoffset, float yoffset)
{
	if (lockedCamera)
	{
		in.Yaw -= xoffset * in.MovementSpeed;
		in.Pitch -= yoffset * in.MovementSpeed;

		if (in.Pitch > 89.0f)                in.Pitch = 89.0f;
		if (in.Pitch < -89.0f)               in.Pitch = -89.0f;

		float radianYaw = glm::radians(in.Yaw);
		float radianPitch = glm::radians(in.Pitch);

		in.Position.x = target.x - cos(radianYaw) * cos(radianPitch) * cam_dist;
		in.Position.y = target.y - sin(radianPitch) * cam_dist;
		in.Position.z = target.z - sin(radianYaw) * cos(radianPitch) * cam_dist;

		in.Front = glm::normalize(target - in.Position);
		in.Right = glm::normalize(glm::cross(in.Front, in.WorldUp));
		in.Up = glm::normalize(glm::cross(in.Right, in.Front));
	}
	else
	{
		in.Yaw += xoffset * in.MovementSpeed;
		in.Pitch += yoffset * in.MovementSpeed;

		if (in.Pitch > 89.0f)                in.Pitch = 89.0f;
		if (in.Pitch < -89.0f)               in.Pitch = -89.0f;

		cam_dist = glm::length(target - in.Position);

		glm::vec3 front;
		front.x = cos(glm::radians(in.Yaw)) * cos(glm::radians(in.Pitch));
		front.y = sin(glm::radians(in.Pitch));
		front.z = sin(glm::radians(in.Yaw)) * cos(glm::radians(in.Pitch));
		in.Front = glm::normalize(front);

		in.Right = glm::normalize(glm::cross(in.Front, in.WorldUp));
		in.Up = glm::normalize(glm::cross(in.Right, in.Front));;
	}
}
