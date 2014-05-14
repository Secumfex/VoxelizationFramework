#ifndef VOXELGRID_H_
#define VOXELGRID_H_

#include <glm/glm.hpp>
#include <vector>
namespace Grid
{
	class GridCell
	{
	protected:
		bool m_occupied;
		float m_size;
	public:
		GridCell(bool occupied = false, float size = 1.0f);
		virtual ~GridCell();
	bool isOccupied() const;
	void setOccupied(bool occupied);
	float getSize() const;
	void setSize(float size);
};


	class VoxelGrid {
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

		GridCell* getGridCell(glm::vec3 position);
	};
}
#endif
