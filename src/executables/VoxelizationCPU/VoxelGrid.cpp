#include "VoxelGrid.h"

using namespace Grid;

VoxelGrid::VoxelGrid(int width, int height, int depth, float cellSize)
{
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_cellSize = cellSize;

	m_voxelGrid.resize(width);
	for (unsigned int i = 0; i < m_voxelGrid.size(); i++)
	{
		m_voxelGrid[i].resize(height);
		for (unsigned j = 0; j < m_voxelGrid[i].size(); j++)
		{
			m_voxelGrid[i][j].resize(depth,0);		}
	}
}

VoxelGrid::~VoxelGrid() {
}

AxisAlignedVoxelGrid::AxisAlignedVoxelGrid(float x, float y, float z,int width, int height, int depth, float cellSize)
	: VoxelGrid(width,height,depth,cellSize)
{
	m_x = x;
	m_y = y;
	m_z = z;
}

AxisAlignedVoxelGrid::~AxisAlignedVoxelGrid()
{

}

void VoxelGrid::setGridCell(int x, int y, int z, GridCell* gridCell)
{
	if ( checkCoordinates(x,y,z) )
	{
		m_voxelGrid[x][y][z] = gridCell;
	}
}

float VoxelGrid::getCellSize() const {
	return m_cellSize;
}

void VoxelGrid::setCellSize(float cellSize) {
	m_cellSize = cellSize;
}

int VoxelGrid::getDepth() const {
	return m_depth;
}

void VoxelGrid::setDepth(int depth) {
	m_depth = depth;
}

int VoxelGrid::getHeight() const {
	return m_height;
}

void VoxelGrid::setHeight(int height) {
	m_height = height;
}

int VoxelGrid::getWidth() const {
	return m_width;
}

void VoxelGrid::setWidth(int width) {
	m_width = width;
}

GridCell* VoxelGrid::getGridCell(int x, int y, int z)
{

	if ( checkCoordinates(x,y,z) )
	{
		return m_voxelGrid[x][y][z];
	}
	else{
		 return 0;
	}
}

bool VoxelGrid::checkCoordinates(int x, int y, int z)
{
	return ( ( x > 0 && x < m_width ) && ( y > 0 && y < m_height ) && ( z > 0 && z < m_depth ) );
}

GridCell* AxisAlignedVoxelGrid::getGridCell(glm::vec3 position)
{
	glm::vec3 gridPos =( position - glm::vec3(m_x,m_y,m_z) ) * 1.0f / m_cellSize;
	return VoxelGrid::getGridCell(gridPos.x, gridPos.y, gridPos.z);
}
