#ifndef VOXELGRID_H_
#define VOXELGRID_H_

#include <Resources/Object.h>
#include <Rendering/Renderable.h>

#include <glm/glm.hpp>
#include <vector>
namespace Grid
{
	class GridCell : public Renderable
	{
	protected:
		int m_x,m_y,m_z;
		bool m_occupied;
		float m_size;
	public:
		GridCell(bool occupied = false, float size = 1.0f, int x = 0, int y = 0, int z = 0);
		virtual ~GridCell();
		bool isOccupied() const;
		void setOccupied(bool occupied);
		float getSize() const;
		void setSize(float size);

		void render();
		void uploadUniforms(Shader* shader);
	int getX() const;
	void setX(int x);
	int getY() const;
	void setY(int y);
	int getZ() const;
	void setZ(int z);
};


	class VoxelGridCPU : public Object{
	protected:
		int m_width;
		int m_height;
		int m_depth;
		float m_cellSize;
		std::vector< std::vector< std::vector < GridCell* > > > m_voxelGrid;
	public:
		VoxelGridCPU(int width = 0, int height = 0, int depth = 0, float cellSize = 1.0f);
		virtual ~VoxelGridCPU();

		inline bool checkCoordinates(int x, int y, int z);

		void setGridCell(int x, int y, int z, GridCell* gridCell);
		GridCell* getGridCell(int x, int y, int z);

		float getCellSize() const;
		void setCellSize(float cellSize);
		int getDepth() const;
		void setDepth(int depth);
		int getHeight() const;
		void setHeight(int height);
		int getWidth() const;
		void setWidth(int width);
	};

	class AxisAlignedVoxelGrid : public VoxelGridCPU
	{
	protected:
		float m_x;
		float m_y;
		float m_z;
	public:
		/**
		 *
		 * @param x start world coordinate x
		 * @param y start world coordinate y
		 * @param z start world coordinate z
		 * @param width amount of grid cells in x diraction
		 * @param height amount of grid cells in y direction
		 * @param depth amount of grid cells in z direction
		 * @param cellSize side length of a cell
		 */
		AxisAlignedVoxelGrid(float x, float y, float z, int width, int height, int depth, float cellSize);
		virtual ~AxisAlignedVoxelGrid();
		glm::vec3 getGridCellCenter(const glm::vec3& position);
		glm::vec3 getGridCellCenter(const GridCell& gridCell);
		GridCell* getGridCell(const glm::vec3& position);
		std::vector < std::pair < GridCell* , glm::vec3 > > getGridCellsForFace(std::vector < glm::vec3 > facePositions);
		std::vector < std::pair < GridCell* , glm::vec3 > > getGridCellsForTriangle(const std::vector < glm::vec3 >& trianglePositions);
	float getX() const;
	void setX(float x);
	float getY() const;
	void setY(float y);
	float getZ() const;
	void setZ(float z);
	};

	bool static testIntersection( const glm::vec3& center, float cellSize, const std::vector< glm::vec3 >& positions );
	bool static triangleOverlapsCross( const glm::vec3& axis, const glm::vec3& edge, float halfExtent, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2 );
	bool static boxOverlapsPlane( const glm::vec3& n_t, float halfExtent, const glm::vec3& v0);
}
#endif
