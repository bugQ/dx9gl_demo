#pragma once

#include "../Graphics/Mesh.h"
#include "../Graphics/Wireframe.h"
#include "../Math/Triangle3.h"

#include <vector>
#include <queue>

namespace eae6320
{
namespace Physics
{
	struct Terrain
	{
		struct Octree
		{
			static const uint8_t MAX_DEPTH = 8;
			static const float FILL_DEPTH_RATIO;

			// invariant: branches are either all NULL (leaf node) or all allocated
			Octree * branch[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
			// invariant: children's bounds are strictly smaller than parent's
			AABB3 bounds;
			// invariant: children's max_depth is 1 less than parent's
			uint8_t max_depth;
			// object_ids are indices into the Terrain's triangles array
			std::vector<uint32_t> object_ids;


			Octree(AABB3 bounds, uint8_t max_depth = MAX_DEPTH) : bounds(bounds), max_depth(max_depth) {}
			~Octree()
			{
				if (!is_leaf())
					for (uint8_t i = 0; i < 8; ++i)
						delete branch[i];
			}

			// used to uphold invariants
			bool is_leaf() const { return branch[0] == NULL; }
			void branch_out()
			{
				if (max_depth > 0)
					for (uint8_t i = 0; i < 8; ++i)
						branch[i] = new Octree(bounds.octant(i), max_depth - 1);
			}

			void populate(const Triangle3 * triangles, uint32_t num_triangles);

			// used by populate:

			// put the triangle in the largest containing box
			void insert(uint32_t, const Triangle3 &);
			// push the triangle id to all intersecting leaves
			void propagate(uint32_t, const Triangle3 &);
			// push all triangles to leaves while leaving branch nodes empty
			void propagate_all(const Triangle3 * triangles, uint32_t num_triangles);
			// ensure 
			void optimize(const Triangle3 * triangles, uint32_t num_triangles);

			size_t intersect(Segment3 segment, std::queue<const Octree *> & hitboxes) const;
			size_t find(uint32_t id, std::queue<const Octree *> & hitboxes) const;

			void take_inventory(std::vector<bool> & inventory) const
#ifdef _DEBUG
			;
#else
			{}
#endif

			// draw debug cubes colored based on depth
			void draw(Graphics::Wireframe &) const
#ifdef _DEBUG
			;
#else
			{}
#endif
		};

		const Triangle3 * triangles;
		const uint32_t num_triangles;

		Octree octree;

		bool debug_octree = false;

		static Terrain * FromBinFile(const char * collision_mesh_path, Vector3 scale);
		Terrain(const Graphics::Mesh::Data &, Vector3 scale);
		~Terrain() { delete[] triangles; }

		void init_octree() { octree.populate(triangles, num_triangles); }

		void draw_octree(Graphics::Wireframe & wireframe)
#ifdef _DEBUG
		{ if (debug_octree) octree.draw(wireframe); }
#else
		{}
#endif

		void draw_raycast(Segment3 segment, Graphics::Wireframe & wireframe)
#ifdef _DEBUG
		;
#else
		{}
#endif

		void test_octree()
#ifdef _DEBUG
		;
#else
		{}
#endif


		float intersect_ray(Vector3 o, Vector3 dir, Vector3 * n) const;
	};
}
}