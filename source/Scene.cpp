#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene():
		m_Materials({ new Material_SolidColor({1,0,0})})
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_TriangleGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for(auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		HitRecord smallestRecord{ };
		smallestRecord.t = FLT_MAX;

		HitRecord temp{};
		for (int i = 0; i < m_SphereGeometries.size(); i++)
		{
			GeometryUtils::HitTest_Sphere(m_SphereGeometries[i], ray, temp);
			if (temp.t < smallestRecord.t) {
				smallestRecord = temp;
			}
		}
		
		for (int i = 0; i < m_PlaneGeometries.size(); i++)
		{
			GeometryUtils::HitTest_Plane(m_PlaneGeometries[i], ray, temp);
			if (temp.t < smallestRecord.t) {
				smallestRecord = temp;
			}
		}

		for (int i = 0; i < m_TriangleGeometries.size(); i++)
		{
			GeometryUtils::HitTest_Triangle(m_TriangleGeometries[i], ray, temp);
			if (temp.t < smallestRecord.t) {
				smallestRecord = temp;
			}
		}

		for (int i = 0; i < m_TriangleMeshGeometries.size(); i++)
		{
			GeometryUtils::HitTest_TriangleMesh(m_TriangleMeshGeometries[i], ray, temp);
			if (temp.t < smallestRecord.t) {
				smallestRecord = temp;
			}
		}

		closestHit = smallestRecord;
	}

	bool Scene::DoesHit(const Ray& ray) const {

		for (int i{ 0 }; i < m_SphereGeometries.size(); i++)
		{
			HitRecord temp{};
			if (GeometryUtils::HitTest_Sphere(m_SphereGeometries[i], ray, temp, true)) {
				return true;
			}
		}

		for (int i{ 0 }; i < m_PlaneGeometries.size(); i++)
		{
			HitRecord temp{};
			if(GeometryUtils::HitTest_Plane(m_PlaneGeometries[i], ray, temp, true)) {
				return true;
			}
		}

		for (int i{ 0 }; i < m_TriangleGeometries.size(); i++)
		{
			HitRecord temp{};
			if(GeometryUtils::HitTest_Triangle(m_TriangleGeometries[i], ray, temp, true)) {
				return true;
			}
		}

		for (int i{ 0 }; i < m_TriangleMeshGeometries.size(); i++)
		{
			HitRecord temp{};
			if(GeometryUtils::HitTest_TriangleMesh(m_TriangleMeshGeometries[i], ray, temp, true)) {
				return true;
			}
		}

		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}
#pragma endregion

#pragma region SCENE W2
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f,3.f,-9.f };
		m_Camera.fovAngle = 45.f;

		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });
		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Plane
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f,1.f,0.f }, .75f, matId_Solid_Red);
		AddSphere({ 0.f,1.f,0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 1.75f,1.f,0.f }, .75f, matId_Solid_Red);
		AddSphere({ -1.75f,3.f,0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 0.f,3.f,0.f }, .75f, matId_Solid_Red);
		AddSphere({ 1.75f,3.f,0.f }, .75f, matId_Solid_Blue);

		//Light
		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
	}
#pragma endregion

#pragma region SCENE W3
	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.f,3.f,-9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .95f, .93f, .88f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .95f, .93f, .88f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .95f, .93f, .88f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .95f, .93f, .88f }, 0.f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .95f, .93f, .88f }, 0.f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .95f, .93f, .88f }, 0.f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		
		//Plane
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matLambert_GrayBlue); //back
		AddPlane({ 0.f, 0.f, 0.f  }, { 0.f, 1.f, 0.f  }, matLambert_GrayBlue); //bottom
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matLambert_GrayBlue); //top
		AddPlane({ 5.f, 0.f, 0.f  }, { -1.f, 0.f, 0.f }, matLambert_GrayBlue); //right
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f  }, matLambert_GrayBlue); //left

		//Spheres
		/*const auto matLambertPhong1 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 3.f));
		const auto matLambertPhong2 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 15.f));
		const auto matLambertPhong3 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 50.f));
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matLambertPhong1);
		AddSphere({ 0.f,    1.f, 0.f }, .75f, matLambertPhong2);
		AddSphere({ 1.75f,  1.f, 0.f }, .75f, matLambertPhong3);*/

		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f,    1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f,  1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f,    3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f,  3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		//Light
		AddPointLight({ 0.f, 5.f, 5.f    }, 50.f, ColorRGB{ 1.f, .61f, .45f  }); //baklight
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f   }); //front light left
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}
#pragma endregion

#pragma region SCENE W3 TEST
	void Scene_W3_Test::Initialize() {
		m_Camera.origin = { 0.f,1.f,-5.f };
		m_Camera.fovAngle = 45.f;

		const unsigned char matLambert_Red = AddMaterial(new Material_Lambert(colors::Red, 1.f));
		const unsigned char matLambert_Yellow = AddMaterial(new Material_Lambert(colors::Yellow, 1.f));
		const auto matLambertPhong_Blue = AddMaterial(new Material_LambertPhong(colors::Blue, 1.f, 1.f, 60.f));

		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .95f, .93f, .88f }, 0.f, .1f));


		//spheres
		AddSphere({ -.75f, 1.f, .0f }, 1.f, matCT_GraySmoothPlastic);
		AddSphere({ .75f, 1.f, .0f }, 1.f, matLambertPhong_Blue);

		//plane
		AddPlane({ 0.f,0.f,0.f }, { 0.f,1.f,0.f }, matLambert_Yellow);

		//light
		AddPointLight({ 0.f,5.f,5.f }, 25.f, colors::White);
		AddPointLight({ 0.f,2.5f,-5.f }, 25.f, colors::White);
	}
#pragma endregion
	
#pragma region SCENE W4 TEST
	void Scene_W4_Test::Initialize() {
		m_Camera.origin = { 0.f,1.f,-5.f };
		m_Camera.totalYaw = { 0 };

		//m_Camera.origin = { .0f, 1.f, 4.f };
		//m_Camera.totalYaw = { PI };

		m_Camera.fovAngle = 45.f;

		const unsigned char matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const unsigned char matLambert_White = AddMaterial(new Material_Lambert(colors::Gray, 1.f));

		//plane
		AddPlane({ 0.f,0.f,10.f }, { 0.f,0.f,-1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f,0.f,0.f }, { 0.f,1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f,10.f,0.f }, { 0.f,-1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f,0.f,0.f }, { -1.f,0.f,0.f }, matLambert_GrayBlue);
		AddPlane({ -5.f,0.f,0.f }, { 1.f,0.f,0.f }, matLambert_GrayBlue);

		//triangle
		/*auto triangle{ Triangle{{-.75f,.5f,.0f},{-.75f,2.f,.0f}, {.75f, .5f, 0.f}} };
		triangle.cullMode = TriangleCullMode::BackFaceCulling;
		triangle.materialIndex = matLambert_White;

		m_TriangleGeometries.emplace_back(triangle);*/

		//2 triangles
		pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		pMesh->positions = {
			{ -.75f,  -1.f, .0f},
			{ -.75f,  1.f,  .0f},
			{  .75f,  1.f,  1.f},
			{  .75f, -1.f,  0.f} 
		};

		pMesh->indices = {
			0,1,2,
			0,2,3
		};
		pMesh->normals.reserve(pMesh->indices.size());
		pMesh->CalculateNormals();
		pMesh->Translate({ 0.f, 1.5f, 0.f });
		pMesh->RotateY(45);

		//box
		//pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		//Utils::ParseOBJ("Resources/simple_cube.obj", pMesh->positions, pMesh->normals, pMesh->indices);
		//pMesh->normals.reserve(pMesh->indices.size());

		//pMesh->Scale({ .7f, .7f, .7f });
		//pMesh->Translate({ .0f, 1.f, 0.f });

		pMesh->UpdateAABB();
		pMesh->UpdateTransforms();

		//light
		AddPointLight({ 0.f,5.f,5.f }, 50.f, ColorRGB(1.f, .61f, .45f));
		AddPointLight({ -2.5f,5.f,-5.f }, 70.f, ColorRGB(1.f, .8f, .45f));
		AddPointLight({ 2.5f,2.5f,-5.f }, 50.f, ColorRGB(.34f, .47f, .68f));
	}

	void Scene_W4_Test::Update(Timer* pTimer) {
		Scene::Update(pTimer);

		//pMesh->RotateY(PI_DIV_2 * pTimer->GetTotal());
		pMesh->UpdateAABB();
		pMesh->UpdateTransforms();
	}
#pragma endregion

#pragma region W4 Refference Scene
	void Scene_W4_ReferenceScene::Initialize() {
		sceneName = "Reference Scene";
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, 0.f, .1f));

		const unsigned char matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const unsigned char matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//plane
		AddPlane({ 0.f,0.f,10.f }, { 0.f,0.f,-1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f,0.f,0.f }, { 0.f,1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f,10.f,0.f }, { 0.f,-1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f,0.f,0.f }, { -1.f,0.f,0.f }, matLambert_GrayBlue);
		AddPlane({ -5.f,0.f,0.f }, { 1.f,0.f,0.f }, matLambert_GrayBlue);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere({ 0.f,    1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere({ 1.75f,  1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere({ 0.f,    3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere({ 1.75f,  3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		//meshes
		const Triangle baseTriangle = { Vector3( -.75f, 1.5f, 0.f ), Vector3( .75f, 0.f, 0.f ), Vector3( -.75f, 0.f, 0.f ) };
		m_Meshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		m_Meshes[0]->AppendTriangle(baseTriangle, true);
		m_Meshes[0]->normals.reserve(m_Meshes[0]->indices.size());
		m_Meshes[0]->Translate({ -1.75f, 4.5f, 0.f });
		m_Meshes[0]->UpdateAABB();
		m_Meshes[0]->UpdateTransforms();

		m_Meshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		m_Meshes[1]->AppendTriangle(baseTriangle, true);
		m_Meshes[1]->normals.reserve(m_Meshes[0]->indices.size());
		m_Meshes[1]->Translate({ 0.f, 4.5f, 0.f });
		m_Meshes[1]->UpdateAABB();
		m_Meshes[1]->UpdateTransforms();

		m_Meshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		m_Meshes[2]->AppendTriangle(baseTriangle, true);
		m_Meshes[2]->normals.reserve(m_Meshes[0]->indices.size());
		m_Meshes[2]->Translate({ 1.75f, 4.5f, 0.f });
		m_Meshes[2]->UpdateAABB();
		m_Meshes[2]->UpdateTransforms();

		//light
		AddPointLight({ 0.f,5.f,5.f }, 50.f, ColorRGB(1.f, .61f, .45f));
		AddPointLight({ -2.5f,5.f,-5.f }, 70.f, ColorRGB(1.f, .8f, .45f));
		AddPointLight({ 2.5f,2.5f,-5.f }, 50.f, ColorRGB(.34f, .47f, .68f));
	}

	void Scene_W4_ReferenceScene::Update(Timer* pTimer) {
		Scene::Update(pTimer);

		float yawAngle{ (cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2 };
		for (const auto& m: m_Meshes)
		{
			m->RotateY(yawAngle);
			m->UpdateAABB();
			m->UpdateTransforms();
		}
	}
#pragma endregion

#pragma region W4 Bunny
	void Scene_W4_Bunny::Initialize()
	{
		sceneName = "Bunny Scene";
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const unsigned char matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));
		const unsigned char matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//plane
		AddPlane({ 0.f,0.f,10.f }, { 0.f,0.f,-1.f }, matLambert_GrayBlue);
		AddPlane({ 0.f,0.f,0.f }, { 0.f,1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 0.f,10.f,0.f }, { 0.f,-1.f,0.f }, matLambert_GrayBlue);
		AddPlane({ 5.f,0.f,0.f }, { -1.f,0.f,0.f }, matLambert_GrayBlue);
		AddPlane({ -5.f,0.f,0.f }, { 1.f,0.f,0.f }, matLambert_GrayBlue);


		//bunny
		m_pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/lowpoly_bunny2.obj",
			m_pMesh->positions,
			m_pMesh->normals,
			m_pMesh->indices
		);

		m_pMesh->Scale({ 2.f, 2.f, 2.f });

		m_pMesh->UpdateAABB();
		m_pMesh->UpdateTransforms();

		//light
		AddPointLight({ 0.f,5.f,5.f }, 50.f, ColorRGB(1.f, .61f, .45f));
		AddPointLight({ -2.5f,5.f,-5.f }, 70.f, ColorRGB(1.f, .8f, .45f));
		AddPointLight({ 2.5f,2.5f,-5.f }, 50.f, ColorRGB(.34f, .47f, .68f));
	}

	void Scene_W4_Bunny::Update(Timer* pTimer) {
		Scene::Update(pTimer);

		float yawAngle{ (cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2 };
		m_pMesh->RotateY(yawAngle);
		m_pMesh->UpdateAABB();
		m_pMesh->UpdateTransforms();
	}
#pragma endregion


}
