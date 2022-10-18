//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	offset = 0.0001f;
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float screenWidth{ static_cast<float>(m_Width) };
	float screenHeight{ static_cast<float>(m_Height) };
	float aspectRatio{ screenWidth / screenHeight };

	float fov{ tan(camera.fovAngle / 2) };
	Matrix camToWorld{ camera.CalculateCameraToWorld() };

	size_t lightsSize{ lights.size() };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{

#pragma region Base
			/*float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;
			ColorRGB finalColor{ gradient, gradient, gradient };*/

			//Scene W1
			//float cX{ (((2 * (px + 0.5f)) / screenWidth) - 1) * aspectRatio };
			//float cY{ 1 - (2 * py / screenHeight) };
			//Vector3 rayDirection{ cX, cY, 1 };
#pragma endregion

//Scene W2
			float cX{ (((2 * (px + 0.5f)) / screenWidth) - 1) * (aspectRatio * fov) };
			float cY{ (1 - (2 * py / screenHeight)) * fov };

			Vector3 rayDirection{ cX, cY, 1 };
			Vector3 rayDirectionNormalised{ rayDirection.Normalized() };

			Vector3 transformedCamera{ camToWorld.TransformVector(rayDirectionNormalised) };

#pragma region unused
#pragma region Gradient screens
			//Vector3 cameraOrigin{ 0, 0, 0 };

			//Ray hitRay{ cameraOrigin, rayDirectionNormalised };
			//ColorRGB finalColor{ rayDirectionNormalised.x, rayDirectionNormalised.y, rayDirectionNormalised.z };
#pragma endregion

#pragma region Red sphere
//Ray viewRay{ {0,0,0}, rayDirectionNormalised };
//ColorRGB finalColor{};
//HitRecord closestHit{};
//Sphere testSphere{ {0.f,0.f,100.f}, 50.f, 0 };

//GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);
////if (closestHit.didHit) {
////	finalColor = materials[closestHit.materialIndex]->Shade();
////}
//if (closestHit.didHit) {
//	const float scaled_t = (closestHit.t - 50.f) / 40.f;
//	finalColor = { scaled_t, scaled_t, scaled_t };
//}
#pragma endregion

#pragma region Plane
//Ray viewRay{ {0,0,0}, rayDirectionNormalised };
//ColorRGB finalColor{};
//HitRecord closestHit{};
//Plane testPlane{ {0.f,-50.f,0.f}, {0.f,1.f,0.f}, 0 };
//GeometryUtils::HitTest_Plane(testPlane, viewRay, closestHit);
///*if (closestHit.didHit) {
//	finalColor = materials[closestHit.materialIndex]->Shade();
//}*/
//if (closestHit.didHit) {
//	const float scaled_t = (closestHit.t - 50.f) / 40.f;
//	finalColor = { scaled_t, scaled_t, scaled_t };
//}
#pragma endregion
#pragma endregion

#pragma region lab weeks
		//W1
		//Ray viewRay{ camera.origin, rayDirectionNormalised };
		//W2
			Ray viewRay{ camera.origin, transformedCamera };
			ColorRGB finalColor{};
			HitRecord closestHit{};

			pScene->GetClosestHit(viewRay, closestHit);
			auto material{ materials[closestHit.materialIndex] };

			if (closestHit.didHit) {
				for (size_t i = 0; i < lightsSize; i++)
				{
					Vector3 direction{ LightUtils::GetDirectionToLight(lights[i],closestHit.origin) };
					Vector3 nornalisedDirection{ direction.Normalized() };
					float LCL{ Vector3::Dot(closestHit.normal, nornalisedDirection) };
					if (LCL < 0) {
						continue;
					}

					Ray lightRay{ closestHit.origin + (closestHit.normal * offset), nornalisedDirection, offset, direction.Magnitude() };

					//shadow
					if (m_ShadowsEnabled) {
						if (pScene->DoesHit(lightRay))
						{
							continue;
						}
					}

					ColorRGB eRGB{ LightUtils::GetRadiance(lights[i], closestHit.origin) };
					ColorRGB BRDFrgb{ material->Shade(closestHit, lightRay.direction, viewRay.direction) };
					switch (m_CurrentLightingMode)
					{
						case LightingMode::BRDF: {
							finalColor += BRDFrgb;
							break;
						}
						case LightingMode::Radiance: {
							finalColor += eRGB;
							break;
						}
						case LightingMode::ObservedArea: {
							finalColor += ColorRGB(LCL, LCL, LCL);
							break;
						}
						case LightingMode::Combined: {
							finalColor += eRGB * BRDFrgb * LCL;
							break;
						}
						default: {
							finalColor += eRGB * BRDFrgb * LCL;
							break;
						}
					}
				}				
			}
			else {
				finalColor = colors::Black;
			}
#pragma endregion

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255)
			);

		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode() {
	m_CurrentLightingMode == LightingMode::Combined ?
		m_CurrentLightingMode = LightingMode(0) :
		m_CurrentLightingMode = LightingMode(static_cast<int>(m_CurrentLightingMode) + 1);
}
