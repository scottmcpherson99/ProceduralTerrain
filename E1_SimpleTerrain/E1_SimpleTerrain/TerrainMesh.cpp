#include "TerrainMesh.h"

TerrainMesh::TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution ) :
	PlaneMesh( device, deviceContext, lresolution ) 
{
	Resize( resolution );
	Regenerate( device, deviceContext );
}

//Cleanup the heightMap
TerrainMesh::~TerrainMesh() {
	delete[] heightMap;
	heightMap = 0;
}


//Fill an array of floats that represent the height values at each grid point.
//Here we are producing a Sine wave along the X-axis
void TerrainMesh::BuildHeightMap() {
	float height = 0.0f;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale =  terrainSize / (float)resolution;

	for( int j = 0; j < ( resolution ); j++ ) {
		for( int i = 0; i < ( resolution ); i++ ) {
			height = ( sin( (float)i * 0.1f * scale ) ) * 10.0f;
			height += ( cos( (float)j * 0.033f * scale ) ) * 10.0f;
			heightMap[( j * resolution ) + i] = 0;
		}
	}	
}

void TerrainMesh::Resize( int newResolution ) {
	resolution = newResolution;
	heightMap = new float[resolution * resolution];
	if( vertexBuffer != NULL ) {
		vertexBuffer->Release();
	}
	vertexBuffer = NULL;
}

// Set up the heightmap and create or update the appropriate buffers
void TerrainMesh::Regenerate( ID3D11Device * device, ID3D11DeviceContext * deviceContext ) {

	VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	float positionX, height, positionZ, u, v, increment;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	
	//Calculate and store the height values
	//BuildHeightMap();
	Fault();
	// Calculate the number of vertices in the terrain mesh.
	// We share vertices in this mesh, so the vertex count is simply the terrain 'resolution'
	// and the index count is the number of resulting triangles * 3 OR the number of quads * 6
	vertexCount = resolution * resolution;

	indexCount = ( ( resolution - 1 ) * ( resolution - 1 ) ) * 6;
	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	index = 0;

	// UV coords.
	u = 0;
	v = 0;
	increment = m_UVscale / resolution;

	//Scale everything so that the look is consistent across terrain resolutions
	const float scale = terrainSize / (float)resolution;

	//Set up vertices
	for( j = 0; j < ( resolution ); j++ ) {
		for( i = 0; i < ( resolution ); i++ ) {
			positionX = (float)i * scale;
			positionZ = (float)( j ) * scale;

			height = heightMap[index];
			vertices[index].position = XMFLOAT3( positionX, height, positionZ );
			vertices[index].texture = XMFLOAT2( u, v );

			u += increment;
			index++;
		}
		u = 0;
		v += increment;
	}

	//Set up index list
	index = 0;
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {

			//Build index array
			indices[index] = ( j*resolution ) + i;
			indices[index + 1] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			indices[index + 2] = ( ( j + 1 ) * resolution ) + i;

			indices[index + 3] = ( j * resolution ) + i;
			indices[index + 4] = ( j * resolution ) + ( i + 1 );
			indices[index + 5] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			index += 6;
		}
	}

	//Set up normals
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {
			//Calculate the plane normals
			XMFLOAT3 a, b, c;	//Three corner vertices
			a = vertices[j * resolution + i].position;
			b = vertices[j * resolution + i + 1].position;
			c = vertices[( j + 1 ) * resolution + i].position;

			//Two edges
			XMFLOAT3 ab( c.x - a.x, c.y - a.y, c.z - a.z );
			XMFLOAT3 ac( b.x - a.x, b.y - a.y, b.z - a.z );
			
			//Calculate the cross product
			XMFLOAT3 cross;
			cross.x = ab.y * ac.z - ab.z * ac.y;
			cross.y = ab.z * ac.x - ab.x * ac.z;
			cross.z = ab.x * ac.y - ab.y * ac.x;
			float mag = ( cross.x * cross.x ) + ( cross.y * cross.y ) + ( cross.z * cross.z );
			mag = sqrtf( mag );
			cross.x/= mag;
			cross.y /= mag;
			cross.z /= mag;
			vertices[j * resolution + i].normal = cross;
		}
	}

	//Smooth the normals by averaging the normals from the surrounding planes
	XMFLOAT3 smoothedNormal( 0, 1, 0 );
	for( j = 0; j < resolution; j++ ) {
		for( i = 0; i < resolution; i++ ) {
			smoothedNormal.x = 0;
			smoothedNormal.y = 0;
			smoothedNormal.z = 0;
			float count = 0;
			//Left planes
			if( ( i - 1 ) >= 0 ) {
				//Top planes
				if( ( j ) < ( resolution - 1 ) ) {
					smoothedNormal.x += vertices[j * resolution + ( i - 1 )].normal.x;
					smoothedNormal.y += vertices[j * resolution + ( i - 1 )].normal.y;
					smoothedNormal.z += vertices[j * resolution + ( i - 1 )].normal.z;
					count++;
				}
				//Bottom planes
				if( ( j - 1 ) >= 0 ) {
					smoothedNormal.x += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.x;
					smoothedNormal.y += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.y;
					smoothedNormal.z += vertices[( j - 1 ) * resolution + ( i - 1 )].normal.z;
					count++;
				}
			}
			//right planes
			if( ( i ) <( resolution - 1 ) ) {

				//Top planes
				if( ( j ) < ( resolution - 1 ) ) {
					smoothedNormal.x += vertices[j * resolution + i].normal.x;
					smoothedNormal.y += vertices[j * resolution + i].normal.y;
					smoothedNormal.z += vertices[j * resolution + i].normal.z;
					count++;
				}
				//Bottom planes
				if( ( j - 1 ) >= 0 ) {
					smoothedNormal.x += vertices[( j - 1 ) * resolution + i].normal.x;
					smoothedNormal.y += vertices[( j - 1 ) * resolution + i].normal.y;
					smoothedNormal.z += vertices[( j - 1 ) * resolution + i].normal.z;
					count++;
				}
			}
			smoothedNormal.x /= count;
			smoothedNormal.y /= count;
			smoothedNormal.z /= count;

			float mag = sqrt( ( smoothedNormal.x * smoothedNormal.x ) + ( smoothedNormal.y * smoothedNormal.y ) + ( smoothedNormal.z * smoothedNormal.z ) );
			smoothedNormal.x /= mag;
			smoothedNormal.y /= mag;
			smoothedNormal.z /= mag;

			vertices[j * resolution + i].normal = smoothedNormal;
		}
	}
	//If we've not yet created our dyanmic Vertex and Index buffers, do that now
	if( vertexBuffer == NULL ) {
		CreateBuffers( device, vertices, indices );
	}
	else {
		//If we've already made our buffers, update the information
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory( &mappedResource, sizeof( D3D11_MAPPED_SUBRESOURCE ) );

		//  Disable GPU access to the vertex buffer data.
		deviceContext->Map( vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		//  Update the vertex buffer here.
		memcpy( mappedResource.pData, vertices, sizeof( VertexType ) * vertexCount );
		//  Reenable GPU access to the vertex buffer data.
		deviceContext->Unmap( vertexBuffer, 0 );
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}

//Create the vertex and index buffers that will be passed along to the graphics card for rendering
//For CMP305, you don't need to worry so much about how or why yet, but notice the Vertex buffer is DYNAMIC here as we are changing the values often
void TerrainMesh::CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices ) {

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the dyanmic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof( VertexType ) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer( &vertexBufferDesc, &vertexData, &vertexBuffer );

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof( unsigned long ) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer( &indexBufferDesc, &indexData, &indexBuffer );
}

void TerrainMesh::Fault()
{
	TerrainPoint edge_[3];
	TerrainPoint currentPoint;
	int axis;
	float newPointSide;
	int chooseSide;

	//pick two random points on the edge of the terrain
	for (int i = 0; i < 2; i++)
	{
		axis = rand() % 4;

		switch (axis)
		{
		case 0:
			edge_[i].X = 0;
			edge_[i].Y = rand() % 129;
			break;

		case 1:
			edge_[i].X = 128;
			edge_[i].Y = rand() % 129;
			break;

		case 2:
			edge_[i].X = rand() % 129;
			edge_[i].Y = 0;
			break;

		case 3:
			edge_[i].X = rand() % 129;
			edge_[i].Y = 128;
		}
	}
		//get vector of the fault line
		edge_[2].X = edge_[1].X - edge_[0].X;
		edge_[2].Y = edge_[1].Y - edge_[0].Y;
		
		chooseSide = rand() % 2;

		//check each point in the terrain to see which side of the fault line it is and apply the fault
		for (int j = 0; j < (resolution); j++) {
			for (int i = 0; i < (resolution); i++) {

				//get the vector of the current point and the fault line
				currentPoint.X = j - edge_[0].X;
				currentPoint.Y = i - edge_[0].Y;

				//perform cross product between the two vectors
				newPointSide = -((edge_[2].X * currentPoint.Y) - (currentPoint.X * edge_[2].Y));

				//raise or lower each point depending if the value of the cross product is negative or positive
				if (newPointSide < 0)
				{
					if (chooseSide == 0)
					{
						heightMap[(j * resolution) + i] = 5;
					}
					else if (chooseSide == 1)
					{
						heightMap[(j * resolution) + i] = -5;
					}
				}
				else if (newPointSide >= 0)
				{
					if (chooseSide == 0)
					{
						heightMap[(j * resolution) + i] = -5;
					}
					else if (chooseSide == 1)
					{
						heightMap[(j * resolution) + i] = 5;
					}
				}
			}
		}
	




}