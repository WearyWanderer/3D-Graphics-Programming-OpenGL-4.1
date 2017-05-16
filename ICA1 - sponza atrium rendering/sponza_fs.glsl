#version 330

struct Light //structure for the lights
{
    vec3 position;
    float range;
    vec3 direction;
    float field_of_view;
	float c_dist_co;
	float q_dist_co;
    mat4 projection_view_xform; //unused until pixar uberlight
};

struct Material //structure for the surface material
{
	vec3 diffuse_colour;
	vec3 specular_colour;
	float shininess;
	int ambientID;
};

uniform sampler2D texture_sample;
uniform Light lights_data[5];
uniform Material material;
uniform vec3 camera_position;
uniform vec3 ambient_intensity;
uniform vec3 emissive_colour;
uniform bool wireframeDraw;

in vec2 varying_texcoord;
in vec3 varying_normal;
in vec3 varying_position;

out vec4 fragment_colour;

//--------------------------------FUNCTION PROTOTYPES-------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

vec4 returnPhongFragment();
vec4 returnLightWireframe();
vec4 returnCameraSphereWireframe();

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------

vec3 N = normalize(varying_normal); //only needs to be done once

void main(void)
{
	if(!wireframeDraw)
	{
	fragment_colour = returnPhongFragment();
	}
	else
	{
	fragment_colour = returnCameraSphereWireframe();
	}
}

vec4 returnPhongFragment()
{
	vec3 finalisedColour = vec3(0,0,0); //start an empty tally of all effective colours

	vec3 ambient_colour = texture(texture_sample, varying_texcoord).rgb; //ambience from texture
	vec3 baseColour = vec3(0, 0, 0);
	if (material.ambientID == 0)
	{
		baseColour = ambient_intensity * material.diffuse_colour; //base surface colour
	}
	else
	{
		baseColour = ambient_intensity * ambient_colour;
	}

	for(int i = 0; i < 5; i++) //for each light that is in the scene
	{
		vec3 L = normalize(lights_data[i].position - varying_position);
		float diffuse_intensity = max(0, dot(L, N));

		/*float attenuation = clamp(1.0 - length(varying_position - lights_data[i].position) / lights_data[i].range, 0.0, 1.0);
		attenuation *= attenuation; */

		float distance = distance(lights_data[i].position, varying_position);
		float attenuation = pow(max(dot(-lights_data[i].direction, L), 0), 1) / (lights_data[i].c_dist_co + distance * 0 + distance * distance * lights_data[i].q_dist_co); //linear not provided
		

		//below is spotlight functionality
		vec3 coneDirection = normalize(lights_data[i].direction);
		vec3 rayDirection = -L;
		float lightToSurfaceAngle = degrees(acos(dot(rayDirection, coneDirection))); //angle in radians then to degrees
		float coneattenuation = smoothstep(lights_data[i].field_of_view / 2, lights_data[i].field_of_view / 4, lightToSurfaceAngle);

		vec3 cameraToSurf = normalize(camera_position - varying_position); //eye position
		vec3 halfRadiusVec = reflect(-cameraToSurf, N); //half angle vector for lights 

		if (material.shininess == 0) //do not use specular
		{
			finalisedColour += (material.diffuse_colour * diffuse_intensity) * attenuation * coneattenuation; //add to the tally for fragment
		}
		else
		{
			float specularFactor = pow((max(0, dot(L, halfRadiusVec))), material.shininess);
			finalisedColour += ((material.diffuse_colour * diffuse_intensity) + (material.specular_colour * specularFactor)) * attenuation * coneattenuation;
		}		
		
	}

	vec4 phongResult;
	float attenFactor = distance(camera_position, varying_position);
	if(attenFactor >= 30)
	{
		phongResult =  vec4(baseColour, 1.0) + vec4(finalisedColour, 1.0);
	}
	else //remove this line (or the emissive component) and remove from if statement to exclude the green emissive light
	{
		phongResult = (vec4(emissive_colour, 1.0) * smoothstep(30, 20, attenFactor)) + vec4(baseColour, 1.0) + vec4(finalisedColour, 1.0); //add emissive additive colour to the fragment
	}
	
	//add this line to the above to add the emissive green sphereical area - (vec4(emissive_colour, 1.0) * smoothstep(30, 20, attenFactor)) +
	
	return phongResult; //return the end result of phong model calculations
}

vec4 returnCameraSphereWireframe()
{
	vec4 wire_colour = vec4(0,0,0, 1.0);
	float attenFactor = distance(camera_position, varying_position);
	if(attenFactor >= 30)
	{
		wire_colour = returnPhongFragment();
	}
	else
	{
		wire_colour = vec4(1,1,1,1.0) - ((vec4(1,1,1,1.0) - returnPhongFragment()) * smoothstep(20, 30, attenFactor));
	}
	
	return wire_colour;
}



//--------deprecated functions below----------

vec4 returnLightWireframe() //this was a function I was initially working on for using the scene's lights to create the wireframe effect
{
	float attenFactor = 1.0f;

	for(int i = 0; i < 5; i++) //for each light that is in the scene
		{
				vec3 L = normalize(lights_data[i].position - varying_position);
				float diffuse_intensity = max(0, dot(L, N));

				/*float attenuation = clamp(1.0 - length(varying_position - lights_data[i].position) / lights_data[i].range, 0.0, 1.0);
				attenuation *= attenuation; */

				float distance = distance(lights_data[i].position, varying_position);
				float attenuation = pow(max(dot(-lights_data[i].direction, L), 0), 20) / (lights_data[i].c_dist_co + distance * 0 + distance * distance * lights_data[i].q_dist_co); //linear not provided

				//below is spotlight functionality
				vec3 coneDirection = normalize(lights_data[i].direction);
				vec3 rayDirection = -L;
				float lightToSurfaceAngle = degrees(acos(dot(rayDirection, coneDirection))); //angle in radians then to degrees
				float coneattenuation = smoothstep(lights_data[i].field_of_view, lights_data[i].field_of_view - 5.0f, lightToSurfaceAngle);

				vec3 cameraToSurf = normalize(camera_position - varying_position); //eye position
				vec3 halfRadiusVec = reflect(-cameraToSurf, N); //half angle vector for lights

				attenFactor -= attenuation * coneattenuation; //add to the tally for fragment
		
		}
	vec4 wire_colour = vec4(0,0,0,1.0) + (returnPhongFragment() * attenFactor);
	return wire_colour;
}