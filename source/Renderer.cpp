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

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float screenWidth{ static_cast<float>(m_Width) };
	float screenHeight{ static_cast<float>(m_Height) };
	float aspectRatio{ screenWidth / screenHeight };

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			float cX{ (((2 * (px + 0.5f)) / screenWidth) - 1) * aspectRatio };
			float cY{ 1 - (2 * py / screenHeight) };

			//ColorRGB finalColor{ gradient, gradient, gradient };

			Vector3 rayDirection{ cX, cY, 1 };
			#pragma region Gradient screens
			//Vector3 normalizedRayDirection{ rayDirection.Normalized() };

			//Vector3 cameraOrigin{ 0, 0, 0 };

			//Ray hitRay{ cameraOrigin, normalizedRayDirection };
			//ColorRGB finalColor{ normalizedRayDirection.x, normalizedRayDirection.y, normalizedRayDirection.z };
			#pragma endregion

			#pragma region Red sphere
			//Ray viewRay{ {0,0,0}, rayDirection };
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
			//Ray viewRay{ {0,0,0}, rayDirection };
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

			#pragma region 2 Spheres
			Ray viewRay{ {0, 0, 0}, rayDirection };
			ColorRGB finalColor{};
			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);
			if (closestHit.didHit) {
				finalColor = materials[closestHit.materialIndex]->Shade();
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
