#include "VoxelGrid.h"

#include <Utility/DebugLog.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace Grid;

double trunc(double d){ return (d>0) ? floor(d) : ceil(d) ; }
float trunc(float d){ return (d>0) ? floor(d) : ceil(d) ; }

VoxelGrid::VoxelGrid(int width, int height, int depth, float cellSize)
	: Object(new Model(), new Material())
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
			m_voxelGrid[i][j].resize(depth);
			for (unsigned int k = 0; k < m_voxelGrid[i][j].size(); k++)
			{
				m_voxelGrid[i][j][k] = new GridCell(false, m_cellSize);
			}
		}
	}

	setRenderMode(GL_LINES);
}

VoxelGrid::~VoxelGrid() {
	for (unsigned int i = 0; i < m_voxelGrid.size(); i++)
	{
		for (unsigned j = 0; j < m_voxelGrid[i].size(); j++)
		{
			for (unsigned int k = 0; k < m_voxelGrid[i][j].size(); k++)
			{
				delete m_voxelGrid[i][j][k];
			}
		}
	}
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

#include <Utility/DebugLog.h>

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

float Grid::AxisAlignedVoxelGrid::getX() const {
	return m_x;
}

void Grid::AxisAlignedVoxelGrid::setX(float x) {
	m_x = x;
}

float Grid::AxisAlignedVoxelGrid::getY() const {
	return m_y;
}

void Grid::AxisAlignedVoxelGrid::setY(float y) {
	m_y = y;
}

float Grid::AxisAlignedVoxelGrid::getZ() const {
	return m_z;
}

void Grid::AxisAlignedVoxelGrid::setZ(float z) {
	m_z = z;
}

GridCell* AxisAlignedVoxelGrid::getGridCell(glm::vec3 position)
{
	glm::vec3 gridPos =( position - glm::vec3(m_x,m_y,m_z) ) / m_cellSize;
	return VoxelGrid::getGridCell( (int) gridPos.x, (int) gridPos.y, (int) gridPos.z);
}

std::vector < std::pair < GridCell* , glm::vec3 > > AxisAlignedVoxelGrid::getGridCellsForFace(std::vector < glm::vec3 > facePositions)
{
	std::vector<std::pair< GridCell*, glm::vec3 > > result;

	// for every edge, interpolate and check sample points.. (oh god this is horrible) TODO
	for (float t = 0.0f; t < 1.0f ; t+=0.05f)
	{
		// TODO find a better way instead of this...
		glm::vec3 currentSamplePoint = facePositions[0] + ( t * ( facePositions[1] - facePositions[0] ) );

		GridCell* intersectedGridCell =  getGridCell( currentSamplePoint );
		if (intersectedGridCell != 0)
		{
			bool alreadyInList = false;
			// check whether this cell is already in the list
			for (unsigned int i = 0; i < result.size(); i++)
			{
				if ( result[i].first == intersectedGridCell )
				{
					alreadyInList = true;
				}
			}

			if( !alreadyInList )
			{
				result.push_back( std::pair <GridCell*, glm::vec3 >(intersectedGridCell, currentSamplePoint ) );

			}
		}
	}

	for (float t = 0.0f; t < 1.0f ; t+=0.05f)
	{
		// TODO find a better way instead of this...
		glm::vec3 currentSamplePoint = facePositions[1] + ( t * ( facePositions[2] - facePositions[1] ) );

		GridCell* intersectedGridCell =  getGridCell( currentSamplePoint );
		if (intersectedGridCell != 0)
		{
			bool alreadyInList = false;
			// check whether this cell is already in the list
			for (unsigned int i = 0; i < result.size(); i++)
			{
				if ( result[i].first == intersectedGridCell )
				{
					alreadyInList = true;
				}
			}

			if( !alreadyInList )
			{
				result.push_back( std::pair <GridCell*, glm::vec3 >(intersectedGridCell, currentSamplePoint ) );

			}
		}
	}

	for (float t = 0.0f; t < 1.0f ; t+=0.05f)
	{
		// TODO find a better way instead of this...
		glm::vec3 currentSamplePoint = facePositions[2] + ( t * ( facePositions[0] - facePositions[2] ) );

		GridCell* intersectedGridCell =  getGridCell( currentSamplePoint );
		if (intersectedGridCell != 0)
		{
			bool alreadyInList = false;
			// check whether this cell is already in the list
			for (unsigned int i = 0; i < result.size(); i++)
			{
				if ( result[i].first == intersectedGridCell )
				{
					alreadyInList = true;
				}
			}

			if( !alreadyInList )
			{
				result.push_back( std::pair <GridCell*, glm::vec3 >(intersectedGridCell, currentSamplePoint ) );

			}
		}
	}

	return result;
}

#include <cmath>

// return the center of the affected grid cell
glm::vec3 AxisAlignedVoxelGrid::getGridCellCenter(glm::vec3 position)
{
	// position in grid space
	glm::vec3 gridPos = ( position - glm::vec3(m_x,m_y,m_z) ) / m_cellSize;

	// round to bottom left front corner of cell and add half a cell and compute world position
	float x = trunc( gridPos.x ) * m_cellSize + ( m_cellSize / 2.0f ) + m_x;
	float y = trunc( gridPos.y ) * m_cellSize + ( m_cellSize / 2.0f ) + m_y;
	float z = trunc( gridPos.z ) * m_cellSize + ( m_cellSize / 2.0f ) + m_z;

	return glm::vec3 ( x , y, z );
}

GridCell::GridCell(bool occupied, float size)
{
	m_occupied = occupied;
	m_size = size;
	m_x = 0;
	m_y = 0;
	m_z = 0;
}

bool GridCell::isOccupied() const {
	return m_occupied;
}

void GridCell::setOccupied(bool occupied) {
	m_occupied = occupied;
}

float GridCell::getSize() const {
	return m_size;
}

void GridCell::setSize(float size) {
	m_size = size;
}

GridCell::~GridCell()
{

}

/*******   RENDERING METHODS   *********/

void GridCell::uploadUniforms(Shader* shader)
{
	shader->uploadUniform( isOccupied(), "uniformOccupied");	// cuz why not
}

void GridCell::render()
{
	//TODO render a Cube
}


