#pragma once

#include "Mesh/Geometry.h"

namespace Egg {

	struct WireframeVertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 color;
	};

	class BasicGeometry {
		~BasicGeometry() = delete;
		BasicGeometry() = delete;

	public:


		/*
		* Creates a unit plane wireframe on the xz plane from the origin
		*/
		static Egg::Mesh::Geometry::P CreatePlaneWireframe(ID3D12Device * device, int gridSectionCount, const DirectX::XMFLOAT3 & color) {
			unsigned int verticesLength = 2 * 2 * (gridSectionCount + 1);
			WireframeVertex * vertices = new WireframeVertex[verticesLength];
			float g = (float)gridSectionCount;

			for(unsigned int i = 0; i <= gridSectionCount; ++i) {
				vertices[2 * i].position = DirectX::XMFLOAT3{ float(i) /g - 0.5f, 0, -0.5f };
				vertices[2 * i].color = color;
				vertices[2 * i + 1].position = DirectX::XMFLOAT3{  float(i) /g - 0.5f, 0, 0.5f };
				vertices[2 * i + 1].color = color;

				vertices[2 * i + (2 * gridSectionCount + 2)].position = DirectX::XMFLOAT3{ -0.5f, 0, float(i) / g  - 0.5f };
				vertices[2 * i + (2 * gridSectionCount + 2)].color = color;
				vertices[2 * i + (2 * gridSectionCount + 2) + 1].position = DirectX::XMFLOAT3{ 0.5f, 0, float(i) /g  - 0.5f };
				vertices[2 * i + (2 * gridSectionCount + 2) + 1].color = color;
			}
			


			Egg::Mesh::Geometry::P geom = Egg::Mesh::VertexStreamGeometry::Create(device, vertices, verticesLength * sizeof(WireframeVertex), sizeof(WireframeVertex));

			delete[] vertices;

			geom->SetTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
			geom->AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			geom->AddInputElement({ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

			return geom;
		}

		/*
		* Creates a capsule silhouette suitable for physics debugging and bounding box drawing as its a linelist has only position and color attributes,
		* use it with DebugPhysicsVS/PS
		*/
		static Egg::Mesh::Geometry::P CreateCapsuleSilhouette(ID3D12Device * device, float height, float radius, const DirectX::XMFLOAT3 & color) {

			WireframeVertex vertices[108];

			float h = height;
			float r = radius;
			constexpr static float PI = 3.1415925f;
			constexpr static float PI2 = 2.0f * PI;

			for(unsigned int i = 0; i < 12; ++i) {

				float arg = float(i) * PI2 / 12.0f;
				float argEnd = float(i + 1) * PI2 / 12.0f;

				DirectX::XMFLOAT3 lineStart { r * cosf(arg), h / 2.0f, r * sinf(arg) };
				DirectX::XMFLOAT3 lineEnd  { r * cosf(argEnd), h / 2.0f, r * sinf(argEnd) };

				vertices[2 * i].position = lineStart;
				vertices[2 * i].color = color;
				vertices[2 * i + 1].position = lineEnd;
				vertices[2 * i + 1].color = color;

				lineStart.y = -lineStart.y;
				lineEnd.y = -lineEnd.y;

				vertices[2 * i + 26].position = lineStart;
				vertices[2 * i + 26].color = color;
				vertices[2 * i + 27].position = lineEnd;
				vertices[2 * i + 27].color = color;

			}

			vertices[24] = vertices[23];
			vertices[25] = vertices[0];

			vertices[50] = vertices[49];
			vertices[51] = vertices[26];

			for(unsigned int i = 0; i < 6; ++i) {

				float arg = float(i) * PI / 6.0f;
				float argEnd = float(i + 1) * PI / 6.0f;

				DirectX::XMFLOAT3 lineStart{ 0, (h / 2.0f) + r * sinf(arg),  r * cosf(arg) };
				DirectX::XMFLOAT3 lineEnd{ 0, (h / 2.0f) + r * sinf(argEnd), r * cosf(argEnd) };

				/*
				vertices[i + 33] = { {0, (-h / 2.0f) - r * sinf(arg), r * cosf(arg) }, color };
				vertices[i + 40] = { {r * cosf(arg), (h / 2.0f) + r * sinf(arg), 0 }, color };
				vertices[i + 47] = { {r * cosf(arg), (-h / 2.0f) - r * sinf(arg), 0 }, color };*/

				vertices[2 * i + 52].position = lineStart;
				vertices[2 * i + 52].color = color;

				vertices[2 * i + 53].position = lineEnd;
				vertices[2 * i + 53].color = color;

				lineStart = DirectX::XMFLOAT3{ 0, (-h / 2.0f) - r * sinf(arg), r * cosf(arg) };
				lineEnd = DirectX::XMFLOAT3{ 0, (-h / 2.0f) - r * sinf(argEnd), r * cosf(argEnd) };

				vertices[2 * i + 64].position = lineStart;
				vertices[2 * i + 64].color = color;

				vertices[2 * i + 65].position = lineEnd;
				vertices[2 * i + 65].color = color;

				lineStart = DirectX::XMFLOAT3{ r * cosf(arg), (h / 2.0f) + r * sinf(arg), 0 };
				lineEnd = DirectX::XMFLOAT3{ r * cosf(argEnd), (h / 2.0f) + r * sinf(argEnd), 0 };


				vertices[2 * i + 76].position = lineStart;
				vertices[2 * i + 76].color = color;

				vertices[2 * i + 77].position = lineEnd;
				vertices[2 * i + 77].color = color;

				lineStart = DirectX::XMFLOAT3{ r * cosf(arg), (-h / 2.0f) - r * sinf(arg), 0 };
				lineEnd = DirectX::XMFLOAT3{ r * cosf(argEnd), (-h / 2.0f) - r * sinf(argEnd), 0 };


				vertices[2 * i + 88].position = lineStart;
				vertices[2 * i + 88].color = color;

				vertices[2 * i + 89].position = lineEnd;
				vertices[2 * i + 89].color = color;
			}

			DirectX::XMFLOAT3 lineSt{ 0, h / 2.0f, r };
			DirectX::XMFLOAT3 lineNd{ 0, -h / 2.0f, r };

			vertices[100].position = lineSt;
			vertices[100].color = color;

			vertices[101].position = lineNd;
			vertices[101].color = color;

			lineSt = DirectX::XMFLOAT3{ 0, h / 2.0f, -r };
			lineNd = DirectX::XMFLOAT3{ 0, -h / 2.0f, -r };


			vertices[102].position = lineSt;
			vertices[102].color = color;

			vertices[103].position = lineNd;
			vertices[103].color = color;

			lineSt = DirectX::XMFLOAT3{ r, h / 2.0f, 0 };
			lineNd = DirectX::XMFLOAT3{ r, -h / 2.0f, 0 };

			vertices[104].position = lineSt;
			vertices[104].color = color;

			vertices[105].position = lineNd;
			vertices[105].color = color;

			lineSt = DirectX::XMFLOAT3{ -r, h / 2.0f, 0 };
			lineNd = DirectX::XMFLOAT3{ -r, -h / 2.0f, 0 };

			vertices[106].position = lineSt;
			vertices[106].color = color;

			vertices[107].position = lineNd;
			vertices[107].color = color;

			Egg::Mesh::Geometry::P geom = Egg::Mesh::VertexStreamGeometry::Create(device, vertices, sizeof(vertices), sizeof(WireframeVertex));

			geom->SetTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
			geom->AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			geom->AddInputElement({ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		
			return geom;
		}

		/*
		* Create a Unit sized box instance, origin is (0,0,0),
		* each vertex has position, normal, texture
		*/
		Egg::Mesh::Geometry::P CreateBox(ID3D12Device* device) {
			PNT_Vertex vertices[] = {
				{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } },
				{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },
				{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
				{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } },
				{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } },
				{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } },

				{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f,  0.0f }, { 0.0f, 1.0f } },
				{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f,  0.0f }, { 1.0f, 0.0f } },
				{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f,  0.0f }, { 1.0f, 0.0f } },
				{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f,  0.0f }, { 0.0f, 1.0f } },
				{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f,  0.0f }, { 1.0f, 1.0f } },

				{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f } },
				{ {  0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f,  0.0f }, { 1.0f, 0.0f } },
				{ {  0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f,  0.0f }, { 1.0f, 0.0f } },
				{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f } },
				{ {  0.5f,  0.5f, 0.5f }, { 1.0f, 0.0f,  0.0f }, { 1.0f, 1.0f } },

				{ {  0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 0.0f } },
				{ {  0.5f,  0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 1.0f } },
				{ { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 1.0f, 0.0f } },
				{ { -0.5f, -0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 1.0f, 0.0f } },
				{ {  0.5f,  0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 0.0f, 1.0f } },
				{ { -0.5f,  0.5f, 0.5f }, { 0.0f, 0.0f,  1.0f }, { 1.0f, 1.0f } },

				{ { -0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
				{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } },
				{ {  0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } },
				{ {  0.5f, -0.5f, 0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } },
				{ { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } },
				{ {  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f } },

				{ {  -0.5f, -0.5f, 0.5f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 0.0f } },
				{ {  -0.5f,  0.5f, 0.5f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f } },
				{ {  -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f,  0.0f }, { 1.0f, 0.0f } },
				{ {  -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f,  0.0f }, { 1.0f, 0.0f } },
				{ {  -0.5f,  0.5f, 0.5f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f } },
				{ {  -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f,  0.0f }, { 1.0f, 1.0f } }
			};
			//ID3D12Device * device, void * data, unsigned int sizeInBytes, unsigned int stride
			Egg::Mesh::Geometry::P geom = Egg::Mesh::VertexStreamGeometry::Create(device, vertices, sizeof(vertices), sizeof(Egg::PNT_Vertex));

			geom->SetVertexType(Egg::PNT_Vertex::type);

			return geom;
		}


	};

}