/*************************************************************************************************************************/
// This source is provided for NON-COMMERCIAL RESEARCH PURPOSES only, and is provided “as is” WITHOUT ANY WARRANTY; 
// without even the implied warranty of fitness for a particular purpose. The redistribution of the code is not permitted.
//
// If you use the source or part of it in a publication, cite the following paper:
// 
// A. Brunton, A. Salazar, T. Bolkart, S. Wuhrer
// Review of Statistical Shape Spaces for 3D Data with Comparative Analysis for Human Faces.
// Computer Vision and Image Understanding, 128:1-17, 2014
//
// Copyright (c) 2016 Timo Bolkart, Stefanie Wuhrer
/*************************************************************************************************************************/

#ifndef DATACONTAINER_H
#define DATACONTAINER_H

#include <stdlib.h>
#include <vector>
#include <string>

template<typename DataType, size_t CoordsDim, typename TextureType, size_t TextureDim>
class ImportMesh
{
public:
	ImportMesh()
	: m_sstrTextureName("")
	{

	}

	ImportMesh(const ImportMesh& mesh)
	{
		*this = mesh;
	}

	~ImportMesh()
	{

	}

	ImportMesh& operator=(const ImportMesh& mesh)
	{
		clear();

		m_vertexList = mesh.getVertexList();
		m_vertexIndexList = mesh.getVertexIndexList();
		m_vertexColorList = mesh.getVertexColorList();

		m_textureList = mesh.getTextureList();
		m_textureIndexList = mesh.getTextureIndexList();
		m_sstrTextureName = mesh.getTextureName();
		return *this;
	}

	virtual void clear()
	{
		clearVertexData();
		clearTextureData();
	}

	virtual void clearVertexData()
	{
		m_vertexList.clear();
		m_vertexIndexList.clear();
		clearVertexColorList();
	}

	virtual void clearVertexColorList()
	{
		m_vertexColorList.clear();
	}

	virtual	void clearTextureData()
	{
		m_textureList.clear();
		m_textureIndexList.clear();
		m_sstrTextureName.clear();
	}

	size_t getNumVertices() const
	{
		return m_vertexList.size()/CoordsDim;
	}

	size_t getNumFaces() const
	{
		return m_vertexIndexList.size();
	}

	const std::vector<DataType>& getVertexList() const
	{
		return m_vertexList;
	}

	std::vector<DataType> getVertexList()
	{
		return m_vertexList;
	}

	const std::vector<std::vector<int>>& getVertexIndexList() const
	{
		return m_vertexIndexList;
	}

	std::vector<std::vector<int>> getVertexIndexList()
	{
		return m_vertexIndexList;
	}

	const std::vector<double>& getVertexColorList() const
	{
		return m_vertexColorList;
	}

	std::vector<double> getVertexColorList()
	{
		return m_vertexColorList;
	}

	const std::vector<TextureType>& getTextureList() const
	{
		return m_textureList;
	}

	std::vector<TextureType> getTextureList()
	{
		return m_textureList;
	}

	const std::vector<std::vector<int>>& getTextureIndexList() const
	{
		return m_textureIndexList;
	}

	std::vector<std::vector<int>> getTextureIndexList()
	{
		return m_textureIndexList;
	}

	const std::string& getTextureName() const
	{
		return m_sstrTextureName;
	}

	virtual bool setVertexList(const std::vector<DataType>& vertexList)
	{
		if(vertexList.size() % CoordsDim != 0)
		{
			return false;
		}

		m_vertexList.clear();
		m_vertexList = vertexList;
		return true;
	}

	virtual void setVertexIndexList(const std::vector<std::vector<int>>& vertexIndexList)
	{
		m_vertexIndexList.clear();
		m_vertexIndexList = vertexIndexList;
	}

	virtual void setVertexColorList(const std::vector<double>& vertexColorList)
	{
		m_vertexColorList.clear();
		m_vertexColorList = vertexColorList;
	}

	virtual bool setTextureList(const std::vector<TextureType>& textureList)
	{
		if(textureList.size() % TextureDim != 0)
		{
			return false;
		}

		m_textureList.clear();
		m_textureList = textureList;
		return true;
	}

	virtual void setTextureIndexList(const std::vector<std::vector<int>>& textureIndexList)
	{
		m_textureIndexList.clear();
		m_textureIndexList = textureIndexList;
	}

	virtual void setTextureName(const std::string& sstrTextureName)
	{
		m_sstrTextureName.clear();
		m_sstrTextureName = sstrTextureName;
	}

private:
	//Vertex data
	std::vector<DataType> m_vertexList;
	std::vector<std::vector<int>> m_vertexIndexList;
	std::vector<double> m_vertexColorList;

	//Texture data
	std::vector<TextureType> m_textureList;
	std::vector<std::vector<int>> m_textureIndexList;

	std::string m_sstrTextureName;
};

typedef ImportMesh<double, 3, double, 2> DataContainer;

#endif