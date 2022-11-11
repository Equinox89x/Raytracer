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

#include <future>
#include <ppl.h>
//#define ASYNC
#define PARALLEL_FOR

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

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRation, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const {
	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;

	float rx{ px + 0.5f };
	float ry{ py + 0.5f };

	float cx{ (2 * (rx / static_cast<float>(m_Width)) - 1) * aspectRation * fov };
	float cy{ (1-(2 * (ry / static_cast<float>(m_Height)))) * fov };

	Vector3 rayDirection{ cx, cy, 1 };

	Matrix camToWorld{ camera.CalculateCameraToWorld() };
	Vector3 transformedCamera{ camToWorld.TransformVector(rayDirection.Normalized()) };

	Ray viewRay{ camera.origin, transformedCamera };
	ColorRGB finalColor{};

	HitRecord closestHit{};

	int lightsSize{ static_cast<int>(lights.size()) };
	pScene->GetClosestHit(viewRay, closestHit);
	const auto material{ materials[closestHit.materialIndex] };

	if (closestHit.didHit) {

		for (const Light& light : lights)
		{
			//check if point we hit can see light
			//if not, go to next lightand skip light calculation
			Vector3 direction{ LightUtils::GetDirectionToLight(light,closestHit.origin) };
			Vector3 normalisedDirection{ direction.Normalized() };
			float LCL{ Vector3::Dot(closestHit.normal.Normalized(), normalisedDirection)};
			if (LCL < 0) {
				continue;
			}

			Ray lightRay{ closestHit.origin + (closestHit.normal * offset), normalisedDirection, offset, direction.Magnitude() };

			//shadow
			if (m_ShadowsEnabled) {
				if (pScene->DoesHit(lightRay))
				{
					continue;
				}
			}

			//render equation
			switch (m_CurrentLightingMode)
			{
			case LightingMode::BRDF: {
				ColorRGB BRDFrgb{ material->Shade(closestHit, lightRay.direction, viewRay.direction) };
				finalColor += BRDFrgb;
				break;
			}
			case LightingMode::Radiance: {
				ColorRGB eRGB{ LightUtils::GetRadiance(light, closestHit.origin) };
				finalColor += eRGB;
				break;
			}
			case LightingMode::ObservedArea: {
				finalColor += ColorRGB(LCL, LCL, LCL);
				break;
			}
			case LightingMode::Combined: {
				ColorRGB BRDFrgb{ material->Shade(closestHit, lightRay.direction, viewRay.direction) };
				ColorRGB eRGB{ LightUtils::GetRadiance(light, closestHit.origin) };
				finalColor += eRGB * BRDFrgb * LCL;
				break;
			}
			default: {
				ColorRGB BRDFrgb{ material->Shade(closestHit, lightRay.direction, viewRay.direction) };
				ColorRGB eRGB{ LightUtils::GetRadiance(light, closestHit.origin) };
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

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	const std::vector<Material*>& materials = pScene->GetMaterials();
	const std::vector<Light>& lights = pScene->GetLights();

	const float screenWidth{ static_cast<float>(m_Width) };
	const float screenHeight{ static_cast<float>(m_Height) };
	const float aspectRatio{ screenWidth / screenHeight };

	const float fov{ tanf(camera.fovAngle * TO_RADIANS / 2.f) };

	const uint32_t numPixels = m_Width * m_Height ;

	#if defined(ASYNC)
	const uint32_t numCores{ std::thread::hardware_concurrency() };
	std::vector<std::future<void>> async_futures{};
	const uint32_t numPixelsPerTask{ numPixels / numCores };
	uint32_t numUnassignedPixels{ numPixels % numCores };
	uint32_t currPixelIndex{ 0 };
	for (uint32_t coreId{ 0 }; coreId < numCores; coreId++)
	{
		uint32_t taskSize{ numPixelsPerTask };
		if (numUnassignedPixels > 0) {
			++taskSize;
			--numUnassignedPixels;
		}

		async_futures.push_back(std::async(std::launch::async, [=, this] {
				const uint32_t pixelIndexEnd{ currPixelIndex + taskSize };
				for (uint32_t pixelIndex{ currPixelIndex }; pixelIndex < pixelIndexEnd; ++pixelIndex)
				{
					RenderPixel(pScene, pixelIndex, fov, aspectRatio, pScene->GetCamera(), lights, materials);
				}
			}
		));
		currPixelIndex += taskSize;
	}

	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}

	#elif defined(PARALLEL_FOR)
	concurrency::parallel_for(0u, numPixels, [=, this](int i) {
		RenderPixel(pScene, i, fov, aspectRatio, pScene->GetCamera(), lights, materials);
	});
	#else
	for (uint32_t i{ 0 }; i < numPixels; i++)
	{
		RenderPixel(pScene, i, fov, aspectRatio, pScene->GetCamera(), lights, materials);
	}
	#endif

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
