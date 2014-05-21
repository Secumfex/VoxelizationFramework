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
		GridCell(bool occupied = false, float size = 1.0f);
		virtual ~GridCell();
		bool isOccupied() const;
		void setOccupied(bool occupied);
		float getSize() const;
		void setSize(float size);

		void render();
		void uploadUniforms(Shader* shader);
	};


	class VoxelGrid : public Object{
	protected:
		int m_width;
		int m_height;
		int m_depth;
		float m_cellSize;
		std::vector< std::vector< std::vector < GridCell* > > > m_voxelGrid;
	public:
		VoxelGrid(int width = 0, int height = 0, int depth = 0, float cellSize = 1.0f);
		virtual ~VoxelGrid();

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

	class AxisAlignedVoxelGrid : public VoxelGrid
	{
	protected:
		float m_x;
		float m_y;
		float m_z;
	public:
		/**
		 *
		 * @param x start coordinate x
		 * @param y start coordinate y
		 * @param z start coordinate z
		 * @param width amount of grid cells in x diraction
		 * @param height amount of grid cells in y direction
		 * @param depth amount of grid cells in z direction
		 * @param cellSize side length of a cell
		 */
		AxisAlignedVoxelGrid(float x, float y, float z, int width, int height, int depth, float cellSize);
		virtual ~AxisAlignedVoxelGrid();
		glm::vec3 getGridCellCenter(glm::vec3 position);
		GridCell* getGridCell(glm::vec3 position);
	float getX() const;
	void setX(float x);
	float getY() const;
	void setY(float y);
	float getZ() const;
	void setZ(float z);
};
}
#endif
