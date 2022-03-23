#pragma once

class MathHelper
{
public:
	// Returns random float in [0, 1).
	static float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	static float RandF(float a, float b)
	{
		return a + RandF()*(b-a);
	}

    static int Rand(int a, int b)
    {
        return a + rand() % ((b - a) + 1);
    }

	template<typename T>
	static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	static T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}
	 
	template<typename T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b-a)*t;
	}

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x); 
	}

	// Returns the polar angle of the point (x,y) in [0, 2*PI).
	static float AngleFromXY(float x, float y);

	static glm::mat4 Identity4x4()
	{
		static glm::mat4 I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}

	static glm::vec4 Vector3TransformCoord(glm::vec3 V, glm::mat4 M)
	{
		glm::vec4 z(V.z);
		glm::vec4 y(V.y);
		glm::vec4 x(V.x);

		glm::vec4 fResult = z * M[2] + M[3];
		fResult = y * M[1] + fResult;
		fResult = x * M[0] + fResult;

		glm::vec4 w(fResult.w);
		return fResult / w;
	}

	static glm::vec4 RandUnitVec3();
	static glm::vec4 RandHemisphereUnitVec3(glm::vec4 n);

	static const float Infinity;
	static const float Pi;


};

