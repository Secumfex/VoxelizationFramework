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
				m_voxelGrid[i][j][k] = new GridCell(false, m_cellSize, i, j, k);
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

// retrieve grid cell corresponding to this world position
GridCell* Grid::AxisAlignedVoxelGrid::getGridCell(const glm::vec3& position)
{
	glm::vec3 gridPos =( position - glm::vec3(m_x,m_y,m_z) ) / m_cellSize;
	return VoxelGrid::getGridCell( (int) gridPos.x, (int) gridPos.y, (int) gridPos.z);
}

std::vector < std::pair < GridCell* , glm::vec3 > > Grid::AxisAlignedVoxelGrid::getGridCellsForTriangle(const std::vector < glm::vec3 >& trianglePositions)
{
	std::vector<std::pair< GridCell*, glm::vec3 > > result;

	if ( trianglePositions.size() != 3)
	{
		DEBUGLOG->log("ERROR : Triangle positions were not sufficient. Aborting intersection testing");
		return result;
	}

	// TODO build an Axis Aligned Bounding Box around the Triangle ( find min / max values )
	const glm::vec3& pos1 = trianglePositions[0];
	const glm::vec3& pos2 = trianglePositions[1];
	const glm::vec3& pos3 = trianglePositions[2];

	glm::vec3 min = glm::min ( pos3, glm::min ( pos1, pos2 ) );
	glm::vec3 max = glm::max ( pos3, glm::max ( pos1, pos2 ) );

//	DEBUGLOG->log("pos1 : " , pos1);
//	DEBUGLOG->log("pos2 : " , pos2);
//	DEBUGLOG->log("pos3 : " , pos3);
//	DEBUGLOG->log("min  : " , min);
//	DEBUGLOG->log("max  : " , max);

	// compute amount of voxels to test in each direction
	glm::vec3 minToMax = max - min;
	int xSteps = ( (int) minToMax.x / m_cellSize ) + 1;
	int ySteps = ( (int) minToMax.y / m_cellSize ) + 1;
	int zSteps = ( (int) minToMax.z / m_cellSize ) + 1;

	// indices of min - voxel
	int indexMinX = ( min.x - m_x ) / m_cellSize;
	int indexMinY = ( min.y - m_y ) / m_cellSize;
	int indexMinZ = ( min.z - m_z ) / m_cellSize;

	glm::vec3 minCellCenter = getGridCellCenter( min );

	// test voxels against polygon
	for ( int x = 0; x < xSteps; x++ )
	{
		for ( int y = 0; y < ySteps; y++ )
		{
			for ( int z = 0; z < zSteps; z++ )
			{
				glm::vec3 currentSamplePoint = minCellCenter + glm::vec3(x * m_cellSize, y * m_cellSize, z * m_cellSize ) + 0.5f * m_cellSize;

				// retrieve grid Cell to be tested against
				GridCell* gridCellCandidate = VoxelGrid::getGridCell(indexMinX + x, indexMinY + y, indexMinZ + z );

				if (gridCellCandidate != 0)
				{
					// perform the test
					bool intersectsPolygon = testIntersection( currentSamplePoint, m_cellSize, trianglePositions );

					// if intersects push back
					if ( intersectsPolygon )
					{
						bool alreadyInList = false;

						// check whether this cell is already in the list
						for (unsigned int i = 0; i < result.size(); i++)
						{
							if ( result[i].first == gridCellCandidate )
							{
								alreadyInList = true;
							}
						}

						if( !alreadyInList )
						{
							result.push_back( std::pair <GridCell*, glm::vec3 >( gridCellCandidate, currentSamplePoint ) );
						}
					}
				}
			}
		}
	}
	return result;
}

#include <cmath>

// return the center of the affected grid cell
glm::vec3 AxisAlignedVoxelGrid::getGridCellCenter(const glm::vec3& position)
{
	// position in grid space
	glm::vec3 gridPos = ( position - glm::vec3(m_x,m_y,m_z) ) / m_cellSize;

	// round to bottom left front corner of cell and add half a cell and compute world position
	float x = trunc( gridPos.x ) * m_cellSize + ( m_cellSize / 2.0f ) + m_x;
	float y = trunc( gridPos.y ) * m_cellSize + ( m_cellSize / 2.0f ) + m_y;
	float z = trunc( gridPos.z ) * m_cellSize + ( m_cellSize / 2.0f ) + m_z;

	return glm::vec3 ( x , y, z );
}

// return the world position of the center of the grid cell
glm::vec3 AxisAlignedVoxelGrid::getGridCellCenter(const GridCell& gridCell)
{
	// position in grid space
	glm::vec3 gridPos = glm::vec3 ( gridCell.getX() * m_cellSize, gridCell.getY() * m_cellSize, gridCell.getZ() * m_cellSize );

	// round to bottom left front corner of cell and add half a cell and compute world position
	float x = trunc( gridPos.x ) * m_cellSize + ( m_cellSize / 2.0f ) + m_x;
	float y = trunc( gridPos.y ) * m_cellSize + ( m_cellSize / 2.0f ) + m_y;
	float z = trunc( gridPos.z ) * m_cellSize + ( m_cellSize / 2.0f ) + m_z;

	return glm::vec3 ( x , y, z );
}

GridCell::GridCell(bool occupied, float size, int x, int y, int z)
{
	m_occupied = occupied;
	m_size = size;
	m_x = x;
	m_y = y;
	m_z = z;
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

int GridCell::getX() const {
return m_x;
}

void GridCell::setX(int x) {
m_x = x;
}

int GridCell::getY() const {
return m_y;
}

void GridCell::setY(int y) {
m_y = y;
}

int GridCell::getZ() const {
return m_z;
}

void GridCell::setZ(int z) {
m_z = z;
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

/**
 * as stated by Fast 3D Triangle-Box Overlap Testing
 * @param center
 * @param cellSize
 * @param positions
 * @return
 */
bool Grid::testIntersection(const glm::vec3& center, float cellSize,
		const std::vector<glm::vec3>& positions) {

	// move triangle as if box was in origin
	glm::vec3 v0 = positions[0] - center;
	glm::vec3 v1 = positions[1] - center;
	glm::vec3 v2 = positions[2] - center;

	// AABB of triangle
	glm::vec3 min = glm::min ( v0, glm::min( v1, v2 ) );
	glm::vec3 max = glm::max ( v0, glm::max( v1, v2 ) );

	// compute edge vectors
	glm::vec3 f0 = v1 - v0;
	glm::vec3 f1 = v2 - v1;
	glm::vec3 f2 = v0 - v2;

	// compute normal of triangle, whatever edge
	glm::vec3 n_t = glm::normalize( glm::cross( f0, f1 ) );

	float halfExtent = cellSize / 2.0f;

	// use Separating Axis Theorem to test

	// 1st : 3 tests - normals of AABB against minmal AABB of triangle

	if ( min.x > halfExtent || max.x < - halfExtent )
	{
		return false;
	}

	if ( min.y > halfExtent || max.y < - halfExtent )
	{
		return false;
	}

	if ( min.z > halfExtent || max.z < - halfExtent )
	{
		return false;
	}

	// if program reaches this point, all axis overlap
	// 2nd : 1 test - normal of Triangle
	bool overlapPlane = triangleOverlapsPlane(n_t, halfExtent, v0);
	if (!overlapPlane)
	{
		return false;
	}

//	// 3rd : 9 tests - cross products of edges
//	glm::vec3 e0(1.0f, 0.0f, 0.0f);
//	glm::vec3 e1(0.0f, 1.0f, 0.0f);
//	glm::vec3 e2(0.0f, 0.0f, 1.0f);
//
//	glm::vec3 cross = glm::cross(e0, f0);
//	bool overlaps = triangleOverlapsCross ( cross, halfExtent, v0, v1, v2 );
//	if ( !overlaps)
//	{
//		return false;
//	}
//
//	cross = glm::cross(e0, f1);
//	overlaps = triangleOverlapsCross ( cross, halfExtent, v0, v1, v2 );
//	if ( !overlaps)
//	{
//		return false;
//	}
//
//	cross = glm::cross(e0, f2);
//	overlaps = triangleOverlapsCross ( cross, halfExtent, v0, v1, v2 );
//	if ( !overlaps)
//	{
//		return false;
//	}
//
//	cross = glm::cross(e1, f0);
//	overlaps = triangleOverlapsCross ( cross, halfExtent, v0, v1, v2 );
//	if ( !overlaps)
//	{
//		return false;
//	}
//
//	cross = glm::cross(e1, f1);
//	overlaps = triangleOverlapsCross ( cross, halfExtent, v0, v1, v2 );
//	if ( !overlaps)
//	{
//		return false;
//	}
//
//	cross = glm::cross(e1, f2);
//	overlaps = triangleOverlapsCross ( cross, halfExtent, v0, v1, v2 );
//	if ( !overlaps)
//	{
//		return false;
//	}
//
//	cross = glm::cross(e2, f0);
//	overlaps = triangleOverlapsCross ( cross, halfExtent, v0, v1, v2 );
//	if ( !overlaps)
//	{
//		return false;
//	}
//
//	cross = glm::cross(e2, f1);
//	overlaps = triangleOverlapsCross ( cross, halfExtent, v0, v1, v2 );
//	if ( !overlaps)
//	{
//		return false;
//	}
//
//	cross = glm::cross(e2, f2);
//	overlaps = triangleOverlapsCross ( cross, halfExtent, v0, v1, v2 );
//	if ( !overlaps)
//	{
//		return false;
//	}

	// if program reaches this pont, all failed to find a separating axis
	return true;
}

bool Grid::triangleOverlapsPlane( const glm::vec3& n_t, float halfExtent, const glm::vec3& v0)
{
	float d = glm::dot( n_t, v0 );
	float negD = -d;

	glm::vec3 diagMax;	// diagonal vector with maximal matching with normal
	glm::vec3 diagMin;	// diagonal vector with minimal matching with normal ( i.e. opposite direction )

	if ( n_t.x > 0.0f )
	{
		diagMin.x = - halfExtent;
		diagMax.x = halfExtent;
	}
	else
	{
		diagMin.x = halfExtent;
		diagMax.x = - halfExtent;
	}

	if ( n_t.y > 0.0f )
	{
		diagMin.y = - halfExtent;
		diagMax.y = halfExtent;
	}
	else
	{
		diagMin.y = halfExtent;
		diagMax.y = - halfExtent;
	}

	if ( n_t.z > 0.0f )
	{
		diagMin.z = - halfExtent;
		diagMax.z = halfExtent;
	}
	else
	{
		diagMin.z = halfExtent;
		diagMax.z = - halfExtent;
	}

	float dotMax = glm::dot ( n_t, diagMax );
	float dotMin = glm::dot( n_t, diagMin);

	// SOME ASSERTIONS CAN BE MADE :

	// dotMin ALWAYS <= 0.0   cuz always facing in the opposite direction of n
	// dotMax ALWAYS >= 0.0   cuz always facing in the same direction of n
	// d negative if normal direction and vertex vector facing in opposite directions
	// d positive if normal direction and vertex vector facing in same directions

	if ( dotMin + negD > 0.0f )
	{
		return false;	// thus, there must be a seperating plane here
	}

	// dunno what this means for anyone
	if ( dotMax + negD >= 0.0f)
	{
		return true;
	}

	// only possible thing now
	return false;
}

bool Grid::triangleOverlapsCross ( const glm::vec3& cross, float halfExtent, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2 )
{
	float neg_x,neg_y,neg_z;

	( cross.x < 0.0f ) ? neg_x = -1.0f : neg_x = 1.0f;
	( cross.y < 0.0f ) ? neg_y = -1.0f : neg_y = 1.0f;
	( cross.z < 0.0f ) ? neg_z = -1.0f : neg_z = 1.0f;

	float radius = halfExtent * glm::abs( cross.x ) + halfExtent * glm::abs( cross.y ) + halfExtent * glm::abs( cross.z );

	float dotV0		 = glm::dot( v0, cross );
	float dotV1		 = glm::dot( v1, cross );
	float dotV2		 = glm::dot( v2, cross );

	float minTriangle = glm::min( dotV0, glm::min ( dotV1, dotV2) );
	float maxTriangle = glm::max( dotV0, glm::max ( dotV1, dotV2) );

	return ( minTriangle < radius && maxTriangle >= - radius);
}
