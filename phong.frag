#version 330 core

in vec3 col;
in vec3 nor;
in vec3 FragPos;
in vec2 tex;

uniform vec3 lightColour;
uniform vec3 lightDirection;
uniform vec3 lightPos;

uniform vec3 camPos;

//uniform vec3 SunDirection;
uniform vec3 SunPos;
uniform vec3 SunColour;

uniform sampler2D Texture;


out vec4 fragColour;

float CalculateSunlightIllumination()
{
    float amb = 0.1f;

	vec3 Nnor = normalize(nor);
	vec3 NToLight = normalize(SunPos - FragPos);
	float diff = max(dot(Nnor, NToLight), 0.f);

	vec3 NFromLight = -NToLight;
	vec3 refDir = reflect(NFromLight, Nnor);
	vec3 NcamDir = normalize(camPos - FragPos);
	float spec = pow(max(dot(NcamDir, refDir), 0.f), 128);

	float i = (amb + diff + spec);

	return i;
}

float CalculateSpotIllumination()
{
	float amb = 0.1f;

	vec3 Nnor = normalize(nor);
	vec3 NToLight = normalize(lightPos - FragPos);
	float diff = max(dot(Nnor, NToLight), 0.f);

	vec3 NFromLight = -NToLight;
	vec3 refDir = reflect(NFromLight, Nnor);
	vec3 NcamDir = normalize(camPos - FragPos);
	float spec = pow(max(dot(NcamDir, refDir), 0.f), 128);

	float d = length(lightPos - FragPos);
	float c = 1.5f;
	float l = .05f;
	float q = .02f;
	float att = 1.f / (c + (l*d) + (q*(d*d)));

	float phi = cos(radians(15.f));
	vec3 NSpotDir = normalize(lightDirection);
	float theta = dot(NFromLight, NSpotDir);

	float i;
	if(theta > phi)
	{
		i = (amb + diff + spec) * att;
	}
	else
	{
		i = (amb) * att;
	}

	return i;
}

void main()
{
    // Calculate illumination from the first light source
    float phong1 = CalculateSpotIllumination();
    vec3 lightEffect1 = phong1 * col * lightColour; // Color effect of the first light

    // Calculate illumination from the Sun
    float phongSun = CalculateSunlightIllumination();
    vec3 sunEffect = phongSun * col * SunColour; // Color effect of the sunlight

    // Combine the color contributions from both lights
    vec3 combinedColor = lightEffect1 + sunEffect;

    // Apply the combined color to the texture color
    vec4 texColor = texture(Texture, tex);
    vec4 finalColor = texColor * vec4(combinedColor, 1.f);
    fragColour = finalColor;
}
 