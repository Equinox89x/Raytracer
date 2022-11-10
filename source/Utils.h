#pragma once
#include <cassert>
#include <fstream>
#include <iostream>
#include "Math.h"
#include "DataTypes.h"
#include <string>  


namespace dae
{
	namespace GeometryUtils
	{
		#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			//is faster
			#pragma region Geometric way
			Vector3 TC{ sphere.origin - ray.origin };
			float dp{ Vector3::Dot(TC, ray.direction) };
			float tcl{ TC.SqrMagnitude() };
			float odSquare{ tcl - Square(dp) };

			float radiusSquared{ Square(sphere.radius) };
			if (odSquare > radiusSquared) {
				return false;
			}
			float tca{ sqrtf(radiusSquared - odSquare) };

			float t{ dp - tca };
			if (t > ray.min && t < ray.max) {
				if (ignoreHitRecord) return true;
				Vector3 I{ ray.origin + t * ray.direction };
				hitRecord.didHit = true;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.t = t;
				hitRecord.origin = I;
				//normalise is dividing by length, and the radius is the length
				hitRecord.normal = (I - sphere.origin)/sphere.radius;
				return true;

			}
			return false;
			#pragma endregion

			//is slower
			#pragma region Analytic way
			/*Vector3 raySphereOrigin{ ray.origin - sphere.origin };

			float A{ Vector3::Dot(ray.direction, ray.direction) };
			float B{ Vector3::Dot((2 * ray.direction), (raySphereOrigin)) };
			float C{ Vector3::Dot((raySphereOrigin), (raySphereOrigin)) - Square(sphere.radius) };

			float discriminant{ Square(B) - 4 * A * C };
			float discriminantsqrt{ sqrtfc(discriminant) };

			float tMax{ (-B + discriminantsqrt) / 2 * A };
			float tMin{ (-B - discriminantsqrt)/2*A };

			if (discriminant > 0) {
				if(tMin > ray.min && tMin < ray.max) {
					hitRecord.didHit = true;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.t = tMin;
					hitRecord.origin = ray.origin + tMin * ray.direction;
					return true;
				}	
				else if (tMax > ray.min && tMax < ray.max)
				{
					hitRecord.didHit = true;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.t = tMax;
					hitRecord.origin = ray.origin + tMax * ray.direction;
					return true;

				}
			}
			return false;*/
			#pragma endregion
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
		#pragma endregion

		#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			float t{ Vector3::Dot((plane.origin - ray.origin), plane.normal)/ Vector3::Dot(ray.direction, plane.normal) };

			if (t > ray.min && t < ray.max) {
				if (ignoreHitRecord) return true;
				Vector3 I{ ray.origin + t * ray.direction };
				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.t = t;
				hitRecord.origin = I;
				hitRecord.normal = plane.normal;
				return true;
			}

			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
		#pragma endregion
		
		#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//faster with bunny scene, same for referrence scene
			#pragma region Moller Trumbore
			Vector3 edge1{ triangle.v1 - triangle.v0 };
			Vector3 edge2{ triangle.v2 - triangle.v0 };

			Vector3 h{ Vector3::Cross(ray.direction, edge2) };
			float a{ Vector3::Dot(edge1, h) };

			float f{ 1.0f / a };
			Vector3 s{ ray.origin - triangle.v0 };
			float u{ f * Vector3::Dot(s, h) };
			if (u < 0.0f || u > 1.0f) {
				return false;
			}

			Vector3 q{ Vector3::Cross(s, edge1) };
			float v{ f * Vector3::Dot(ray.direction, q) };
			if (v < 0.0f || u + v > 1.0f) {
				return false;
			}

			// At this stage we can compute t to find out where the intersection point is on the line.
			float t{ f * Vector3::Dot(edge2, q) };
			// ray intersection
			if (t > ray.min && t < ray.max) 
			{
				//change cullmode if it's a shadow (ignorehitrecord = true)
				Vector3 normal = Vector3::Cross(edge1, edge2);
				TriangleCullMode currentCulling = triangle.cullMode;
				if (ignoreHitRecord) {
					if (currentCulling == TriangleCullMode::BackFaceCulling) {
						currentCulling = TriangleCullMode::FrontFaceCulling;
					}
					else if (currentCulling == TriangleCullMode::FrontFaceCulling) {
						currentCulling = TriangleCullMode::BackFaceCulling;
					}
				}

				switch (currentCulling)
				{
				case TriangleCullMode::BackFaceCulling:
					if (Vector3::Dot(normal, ray.direction) > 0)
					{
						return false;
					}
					break;
				case TriangleCullMode::FrontFaceCulling:
					if (Vector3::Dot(normal, ray.direction) < 0)
					{
						return false;
					}
					break;
				}

				if (ignoreHitRecord) return true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.normal = normal;
				hitRecord.origin = ray.origin + (t * ray.direction);
				hitRecord.didHit = true;
				hitRecord.t = t;
				return true;
			}

			// This means that there is a line intersection but not a ray intersection.
			return false;
			#pragma endregion

			#pragma region Badouel
			//Vector3 edge1{ triangle.v1 - triangle.v0 };
			//Vector3 edge2{ triangle.v2 - triangle.v0 };
			//Vector3 normal{ Vector3::Cross(edge1, edge2) };

			//if (Vector3::Dot(normal, ray.direction) == 0) {
			//	return false;
			//}

			//float d{ Vector3::Dot(-triangle.v0, normal) };
			//float t{ -(((d + Vector3::Dot(normal, ray.origin))) / Vector3::Dot(normal, ray.direction)) };
			//Vector3 intersectPoint{ ray.origin + (t * ray.direction) }; //P
			//if (t <= 0) {
			//	return false;
			//}

			//if (!(t > ray.min && t < ray.max)) {
			//	return false;
			//}

			//float u{ intersectPoint.x - triangle.v0.x };
			//float v{ intersectPoint.y - triangle.v0.y };			
			//
			//float u1{ triangle.v1.x - triangle.v0.x};
			//float u2{ triangle.v2.x - triangle.v0.x };
			//float v2{ triangle.v1.y - triangle.v0.y };
			//float v1{ triangle.v2.y - triangle.v0.y };

			//float beta{};
			//float alpha{};
			//if (u1 == 0) {
			//	beta = u / u2;
			//	if (beta >= 0 && beta <= 1) {
			//		alpha = (v - beta * v2) / v1;
			//	}
			//}
			//else {
			//	beta = (v * u1 - u * v1) / (v2 * u1 - u2 * v1);
			//	if (beta >= 0 && beta <= 1) {
			//		alpha = (u - beta * u2) / u1;
			//	}
			//}

			//if (alpha >= 0 && beta >= 0 && alpha + beta <= 1) {
			//	return true;
			//}
			//return false;
			#pragma endregion

			#pragma region Method from classes
			//Vector3 a = triangle.v1 - triangle.v0;
			//Vector3 b = triangle.v2 - triangle.v0;
			//Vector3 normal = Vector3::Cross(a, b);

			//if (Vector3::Dot(ray.direction, normal) == 0)
			//{
			//	return false;
			//}

			//Vector3 center{ (triangle.v0 + triangle.v1 + triangle.v2) / 3 };
			//Vector3 L{ center - ray.origin };
			//float t{ Vector3::Dot(L, normal) / Vector3::Dot(ray.direction, normal) };
			//if (t < ray.min || t > ray.max)
			//{
			//	return false;
			//}

			//Vector3 edgeA{ triangle.v1 - triangle.v0 };
			//Vector3 origin{ ray.origin + (t * ray.direction) };
			//Vector3 pointToSide{ origin - triangle.v0 };
			//if (Vector3::Dot(normal, Vector3::Cross(edgeA, pointToSide)) < 0)
			//{
			//	return false;
			//}

			//Vector3 edgeB{ triangle.v2 - triangle.v1 };
			//pointToSide = origin - triangle.v1;
			//if (Vector3::Dot(normal, Vector3::Cross(edgeB, pointToSide)) < 0)
			//{
			//	return false;
			//}

			//Vector3 edgeC{ triangle.v0 - triangle.v2 };
			//pointToSide = origin - triangle.v2;
			//if (Vector3::Dot(normal, Vector3::Cross(edgeC, pointToSide)) < 0)
			//{
			//	return false;
			//}

			//float result{ Vector3::Dot(normal, ray.direction) };
			//TriangleCullMode currentCulling = triangle.cullMode;
			//if (ignoreHitRecord) {
			//	//change cullmode if it's a shadow
			//	currentCulling == TriangleCullMode::BackFaceCulling ?
			//		currentCulling = TriangleCullMode::FrontFaceCulling :
			//		currentCulling = TriangleCullMode::BackFaceCulling;
			//}

			//switch (currentCulling)
			//{
			//case TriangleCullMode::BackFaceCulling:
			//	if (result > 0)
			//	{
			//		return false;
			//	}
			//	break;
			//case TriangleCullMode::FrontFaceCulling:
			//	if (result < 0)
			//	{
			//		return false;
			//	}
			//	break;
			//}

			//if (ignoreHitRecord) return true;
			//hitRecord.materialIndex = triangle.materialIndex;
			//hitRecord.normal = normal;
			//hitRecord.origin = origin;
			//hitRecord.didHit = true;
			//hitRecord.t = t;
			//return true;
			#pragma endregion
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
		#pragma endregion

		#pragma region TriangleMesh HitTest
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray) {
			float tx1{ (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x };
			float tx2{ (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x };

			float tMin{ std::min(tx1,tx2) };
			float tMax{ std::max(tx1,tx2) };

			float ty1{ (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y };
			float ty2{ (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y };

			tMin = std::max(tMin, std::min(ty1, ty2));
			tMax = std::min(tMax, std::max(ty1, ty2));

			float tz1{ (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z };
			float tz2{ (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z };

			tMin = std::max(tMin, std::min(tz1, tz2));
			tMax = std::min(tMax, std::max(tz1, tz2));

			return tMax > 0 && tMax >= tMin;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			if (!SlabTest_TriangleMesh(mesh, ray)) {
				return false;
			}

			float distance{ FLT_MAX };
			Triangle t;
			HitRecord temp;
			int size{ static_cast<int>(mesh.normals.size()) };
			for (int i = 0; i < size; i++)
			{
				int i2{ i * 3 };
				t.v0 = mesh.positions[mesh.indices[i2]];
				t.v1 = mesh.positions[mesh.indices[i2 + 1]];
				t.v2 = mesh.positions[mesh.indices[i2 + 2]];
					
				t.normal = mesh.normals[mesh.indices[i2]];
				t.materialIndex = mesh.materialIndex;
				t.cullMode = mesh.cullMode;

				if (HitTest_Triangle(t, ray, temp, ignoreHitRecord)) {
					if(ignoreHitRecord) return true;

					if (hitRecord.didHit && temp.t < distance) {
						distance = temp.t;
						hitRecord = temp;
					}
				}
			}
			
			return hitRecord.didHit;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
		#pragma endregion
	
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			if (light.type == dae::LightType::Directional) {
				return light.direction;
			}
			return light.origin - origin;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			return light.color * light.intensity/(light.origin - target).SqrMagnitude();
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}