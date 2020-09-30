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

#include "FileLoader.h"
#include "MathHelper.h"

#include <fstream>
#include <string>
#include <iostream>

std::string FileLoader::getFileExtension(const std::string& sstrFileName)
{
	if(sstrFileName.empty())
	{
		return "";
	}

	const size_t pos = sstrFileName.rfind(".");
	if(pos != std::string::npos)
	{
		return sstrFileName.substr(pos+1);
	}

	return "";
}

bool FileLoader::fileExist(const std::string& sstrFileName)
{
	if(sstrFileName.empty())
	{
		return false;
	}

	std::fstream inStream;
	inStream.open(sstrFileName, std::ios::in);

	if(inStream.is_open())
	{
		inStream.close();
		return true;
	}
	
	return false;
}

std::string FileLoader::getFileName(const std::string& sstrFullFileName)
{
	std::string sstrFileName = "";

	if(sstrFullFileName.empty())
	{
		return sstrFileName;
	}

	//if(!fileExist(sstrFullFileName))
	//{
	//	return sstrFileName;
	//}

	size_t posEnd = sstrFullFileName.rfind(".");
	if(posEnd == std::string::npos)
	{
		posEnd = sstrFullFileName.size();
	}

	size_t posStart = sstrFullFileName.rfind("/");
	if(posStart != std::string::npos)
	{
		sstrFileName = sstrFullFileName.substr(posStart+1, posEnd-posStart-1);
		return sstrFileName;
	}

	posStart = sstrFullFileName.rfind("\\");
	if(posStart != std::string::npos)
	{
		sstrFileName = sstrFullFileName.substr(posStart+1, posEnd-posStart-1);
		return sstrFileName;
	}
	else
	{
		sstrFileName = sstrFullFileName.substr(0, posEnd);
	}

	return sstrFileName;
}

std::string FileLoader::getFilePath(const std::string& sstrFileName)
{
	std::string sstrFilePath = "";

	if(sstrFileName.empty())
	{
		return sstrFilePath;
	}

	if(!fileExist(sstrFileName))
	{
		return sstrFilePath;
	}

	size_t pos = sstrFileName.rfind("/");
	if(pos != std::string::npos)
	{
		sstrFilePath = sstrFileName.substr(0, pos);
		return sstrFilePath;
	}

	pos = sstrFileName.rfind("\\");
	if(pos != std::string::npos)
	{
		sstrFilePath = sstrFileName.substr(0, pos);
		return sstrFilePath;
	}

	return sstrFilePath;
}

bool FileLoader::loadFile(const std::string& sstrFileName, DataContainer& outData)
{
	if(!fileExist(sstrFileName))
	{
		return false;
	}

	bool bReturn(false);

	const std::string sstrSuffix = FileLoader::getFileExtension(sstrFileName);
	if(sstrSuffix=="off")
	{
		bReturn = loadOFF(sstrFileName, outData);
		if(bReturn)
		{
			MathHelper::cleanMesh(outData);
		}
	}
	else if(sstrSuffix=="wrl")
	{
		bReturn = loadWRL(sstrFileName, outData);
		if(bReturn)
		{
			MathHelper::cleanMesh(outData);
		}
	}
	else if (sstrSuffix=="obj")
	{
		bReturn = loadObj(sstrFileName, outData);
		if(bReturn)
		{
			MathHelper::cleanMesh(outData);
		}
	}

	return bReturn;
}

bool FileLoader::loadDataFile(const std::string& sstrDataFileName, std::vector<double>& data)
{
	const char* cstrFileName = sstrDataFileName.c_str();
	if(cstrFileName==NULL)
	{
		return false;
	}

	FILE* pFile(NULL);
	errno_t err = fopen_s(&pFile, cstrFileName, "r");
	if(pFile==NULL || err!=0)
	{
		return false;
	}

	while(true)
	{
		double val(0.0);
		char strOutput[1000];
		if(!readNextNumber(pFile, FLOATING_POINT, strOutput, val))
		{
			break;
		}

		data.push_back(val);
	}

	return fclose(pFile)==0;
}

bool FileLoader::loadWRL(const std::string& sstrFileName, DataContainer& outData)
{
	const char* cstrFileName = sstrFileName.c_str();
	if(cstrFileName==NULL)
	{
		return false;
	}

	FILE* pFile = fopen(cstrFileName, "r");
	if(pFile==NULL)
	{
		return false;
	}

	bool bEnd(false);
	while(!bEnd)
	{
		char output[1000] = "";
		
		while(!bEnd && !isEqual(output, "Shape") && !isEqual(output, "Shape"))
		{
			bEnd = readNextNode(pFile, output);
		}

		if(!bEnd && isEqual(output, "Shape"))
		{
			bEnd = processShapeNode(pFile, output, outData);
		}

		if(!bEnd && isEqual(output, "Transform"))
		{
			//bEnd = processTransformNode(pFile, output, ...);
		}
	}

	return fclose(pFile)==0;
}

bool FileLoader::loadOFF(const std::string& sstrFileName, DataContainer& outData)
{
	const char* cstrFileName = sstrFileName.c_str();
	if(cstrFileName==NULL)
	{
		return false;
	}

	FILE* pFile = fopen(cstrFileName, "r");
	if(pFile==NULL)
	{
		return false;
	}

	char output[1000] = "";
	readNextNode(pFile, output);

	bool bColorOff(false);
	if(strcmp(output, "OFF") == 0)
	{
		bColorOff = false;
	}
	else if(strcmp(output, "COFF") == 0)
	{
		bColorOff = true;
	}
	else
	{
		return false;
	}

	int numVertices(0);
	readNextNumber(pFile, INTEGER, output, numVertices);

	int numFaces(0);
	readNextNumber(pFile, INTEGER, output, numFaces);

	int numEdges(0);
	readNextNumber(pFile, INTEGER, output, numEdges);

	if(numVertices < 1 /*|| numFaces < 1*/)
	{
		return false;
	}

	std::vector<double> vertexList;
	std::vector<std::vector<int>> vertexIndexList;
	std::vector<double> vertexColorList;

	bool bEnd(false);
	for(int vertex = 0; vertex < numVertices; ++vertex)
	{
		char tmpOutput[1000];
	
		for(int i = 0; i < 3; ++i)
		{
			double number(0.0);		
			if(!readNextNumber(pFile, FLOATING_POINT, tmpOutput, number))
			{
				bEnd = true;
				break;
			}

			vertexList.push_back(number);
		}

		if(bEnd || vertexList.size()%3 != 0)
		{
			std::cout << "Loaded vertex list of wrong dimension" << std::endl;
			return false;
		}

		if(bColorOff)
		{
			for(int i = 0; i < 3; ++i)
			{
				double colorValue(0);		
				if(!readNextNumber(pFile, FLOATING_POINT, tmpOutput, colorValue))
				{
					bEnd = true;
					break;
				}

				vertexColorList.push_back(colorValue);
			}

			if(bEnd || vertexColorList.size()%3 != 0)
			{
				std::cout << "Loaded vertex color list of wrong dimension" << std::endl;
				return false;
			}

			double alphaValue(0);		
			if(!readNextNumber(pFile, FLOATING_POINT, tmpOutput, alphaValue))
			{
				return false;
			}
		}
	}

	for(int face = 0; face < numFaces; ++face)
	{
		char tmpOutput[1000];

		int numPolyPoints(0);		
		if(!readNextNumber(pFile, INTEGER, tmpOutput, numPolyPoints))
		{
			break;
		}

		if(numPolyPoints!=3)
		{
#ifdef DEBUG_OUTPUT
			std::cout << "loadOff() - only triangles supported" << std::endl; 
#endif
			return false;
		}

		std::vector<int> polyIndices;
		for(int i = 0; i < 3; ++i)
		{
			int vertexIndex(0);		
			if(!readNextNumber(pFile, INTEGER, tmpOutput, vertexIndex))
			{
				bEnd = true;
				break;
			}

			polyIndices.push_back(vertexIndex);
		}

		if(bEnd || polyIndices.size() % 3 != 0) 
		{
			std::cout << "Polygon not considered" << std::endl;
			break;
		}

		vertexIndexList.push_back(polyIndices);
	}


	if(!outData.setVertexList(vertexList))
	{
		return false;
	}

	outData.setVertexIndexList(vertexIndexList);
	outData.setVertexColorList(vertexColorList); 

	return fclose(pFile)==0;
}

bool FileLoader::loadObj(const std::string& sstrFileName, DataContainer& outData)
{
	const char* cstrFileName = sstrFileName.c_str();
	if(cstrFileName==NULL)
	{
		return false;
	}

	FILE* pFile = fopen(cstrFileName, "r");
	if(pFile==NULL)
	{
		return false;
	}

	std::vector<double> vertexList;
	std::vector<std::vector<int>> vertexIndexList;
	std::vector<double> vertexColors;

	bool bEnd(false);

	char output[1000];
	while(strcmp(output, "v") != 0 
			&& strcmp(output, "vn") != 0 
			&& strcmp(output, "vt") != 0
			&& strcmp(output, "f") != 0 
			&& !bEnd)
	{
		bEnd = readNextNode(pFile, output);
	}

	//Read vertices
	while(strcmp(output, "v") == 0 && !bEnd)
	{
		char tmpOutput[1000];
	
		Vec3d* pVertex = new Vec3d(0.0,0.0,0.0);
		for(int i = 0; i < 3; ++i)
		{
			double number(0.0);		
			if(!readNextNumber(pFile, FLOATING_POINT, tmpOutput, number))
			{
				bEnd = true;
				break;
			}

			vertexList.push_back(number);
		}

		if(bEnd || vertexList.size()%3 != 0)
		{
			return false;
		}

		bEnd = readNextNode(pFile, output);
	}

	while(strcmp(output, "v") != 0 
			&& strcmp(output, "vn") != 0 
			&& strcmp(output, "vt") != 0
			&& strcmp(output, "f") != 0 
			&& !bEnd)
	{
		bEnd = readNextNode(pFile, output);
	}

	//Read normals
	while(strcmp(output, "vn") == 0 && !bEnd)
	{
		char tmpOutput[1000];

		for(int i = 0; i < 3; ++i)
		{
			double number(0.0);		
			if(!readNextNumber(pFile, FLOATING_POINT, tmpOutput, number))
			{
				bEnd = true;
				break;
			}
		}

		if(bEnd)
		{
			return false;
		}

		bEnd = readNextNode(pFile, output);
	}

	while(strcmp(output, "v") != 0 
			&& strcmp(output, "vn") != 0 
			&& strcmp(output, "vt") != 0
			&& strcmp(output, "f") != 0 
			&& !bEnd)
	{
		bEnd = readNextNode(pFile, output);
	}

	//Read texture coordinates
	while(strcmp(output, "vt") == 0 && !bEnd)
	{
		char tmpOutput[1000];

		for(int i = 0; i < 2; ++i)
		{
			double number(0.0);		
			if(!readNextNumber(pFile, FLOATING_POINT, tmpOutput, number))
			{
				bEnd = true;
				break;
			}
		}

		if(bEnd)
		{
			return false;
		}

		bEnd = readNextNode(pFile, output);
	}

	while(strcmp(output, "v") != 0 
			&& strcmp(output, "vn") != 0 
			&& strcmp(output, "vt") != 0
			&& strcmp(output, "f") != 0 
			&& !bEnd)
	{
		bEnd = readNextNode(pFile, output);
	}

	//Read faces
	while(strcmp(output, "f") == 0 && !bEnd)
	{
		char tmpOutput[1000];

		//Vec3i* pPoly = new Vec3i(0,0,0);
		std::vector<int> polygon;
		for(size_t i = 0; i < 3; ++i)
		{
			bEnd = readNextNode(pFile, tmpOutput);
			if(bEnd)
			{
				//delete pPoly;
				return false;
			}

			const std::string sstrTmpOutput(tmpOutput);

			std::string sstrVertexNum("");
			std::string sstrTexturNum("");
			std::string sstrNormalNum("");

			const size_t pos1 = sstrTmpOutput.find_first_of("/");
			if(pos1 != std::string::npos)
			{
				sstrVertexNum = sstrTmpOutput.substr(0, pos1);
			}
			else
			{
				sstrVertexNum = sstrTmpOutput;
			}

			//const size_t pos2 = sstrTmpOutput.find_first_of("/", pos1+1);
			//if(pos2 == std::string::npos)
			//{

			//}


			//sstrTexturNum = sstrTmpOutput.substr(pos1+1, pos2-pos1-1);
			//sstrNormalNum = sstrTmpOutput.substr(pos2+1);

			if(sstrVertexNum.empty())
			{
				//delete pPoly;
				return false;
			}

			const int vertexNum = atoi(sstrVertexNum.c_str());
			//const int num2 = atoi(sstrTexturNum.c_str());
			//const int num3 = atoi(sstrNormalNum.c_str());

			//(*pPoly)[i] = vertexNum-1;
			polygon.push_back(vertexNum-1);
		}

		//vertexIndexList.push_back(pPoly);
		vertexIndexList.push_back(polygon);
		bEnd = readNextNode(pFile, output);
	}

	outData.setVertexList(vertexList);
	outData.setVertexIndexList(vertexIndexList);
	outData.setVertexColorList(vertexColors);

	return fclose(pFile)==0;
}

bool FileLoader::readNextNode(FILE* pFile, char* cstrOutput)
{
	memset(cstrOutput, 0, 1000);

	//Read next word till end of the line, space, or any bracket
	//Ignore comments marked by # at the beginning of the line
	size_t currSize(0);
	while(currSize < 1000) //One nod is probably not longer than 1000 characters
	{
		int currChar = fgetc(pFile);

		if(currChar == EOF)
		{
			return true;
		}
	  
		// #
		if(currChar == 35)
		{
			//Ignore rest of the line
			// 10 = new line
			do
			{
				currChar = fgetc(pFile);
			}
			while(currChar != EOF && currChar != 10);

			//If we already read something stop here
			if(currSize > 0)
			{
				return currChar == EOF;
			}
		}

		//If last element was a bracket treat it special
		//  91 = [		92 = ]
		// 123 = {    125 = }
		if(currChar == 91 || currChar == 92 || currChar == 123 || currChar == 125)
		{
			if(currSize == 0)
			{
				//Return the bracket if it is the first character
				cstrOutput[currSize] = (char)currChar;
				++currSize;

				return false;
			}
			else
			{
				//Push back bracket to the stream if it is not the first character
				currChar = ungetc(currChar, pFile);
				if(currChar == EOF)
				{
					//Failed pushing bracket back into stream
					std::cout << "Failed pushing back bracket into stream" << std::endl;
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		
		//Special handling for space, tab and new line 
		// ( 32 = space, 9 = horizontal tab, 11 = vertical tab, 10 = new line, 12 = new page)
		// if first character: continue
		// if not first character: stop reading
		//if(currChar == 32 || currChar = 9 || currChar = 11 || currChar == 10 || currChar == 12)
		if(currChar >= 0 && currChar <= 32)
		{
			if(currSize == 0)
			{
				continue;
			}
			else
			{
				return false;
			}
		}

		cstrOutput[currSize] = (char)currChar;
		++currSize;
	}

	return false;
}

bool FileLoader::processShapeNode(FILE* pFile, char* cstrOutput, DataContainer& outPoly)
{
	bool bEnd(false);
	while(!bEnd)
	{
		bEnd = readNextNode(pFile, cstrOutput);

		if(bEnd)
		{
			break;
		}

		if(!bEnd && isEqual(cstrOutput, "ImageTexture"))
		{
			char cstrTextureName[1000] = "";
			bEnd &= processImageTexture(pFile, cstrOutput, cstrTextureName);
			
			std::string sstrTextureName(cstrTextureName);
			if(!sstrTextureName.empty())
			{
				outPoly.setTextureName(sstrTextureName);
			}
		}
		
		if(!bEnd && isEqual(cstrOutput, "coord"))
		{
			std::vector<double> vertexList;
			processCoordinates(pFile, cstrOutput, 3, vertexList);
			outPoly.setVertexList(vertexList);
		}
		
		if(!bEnd && isEqual(cstrOutput, "coordIndex"))
		{
			std::vector<std::vector<int>> vertexIndexList;
			processIndices(pFile, cstrOutput, vertexIndexList);
			outPoly.setVertexIndexList(vertexIndexList);
		}
		
		if(!bEnd && isEqual(cstrOutput, "TextureCoordinate"))
		{
			std::vector<double> textureList;
			processCoordinates(pFile, cstrOutput, 2, textureList);
			outPoly.setTextureList(textureList);
		}
		
		if(!bEnd && isEqual(cstrOutput, "texCoordIndex"))
		{
			std::vector<std::vector<int>> textureIndexList;
			processIndices(pFile, cstrOutput, textureIndexList);
			outPoly.setTextureIndexList(textureIndexList);
		}
	}

	return bEnd;
}

bool FileLoader::processImageTexture(FILE* pFile, char* cstrOutput, char* cstrTextureName)
{
	bool bEnd = readNodeBlocksUntil(pFile, "url", cstrOutput);

	assert(!bEnd);
	assert(!isEqual(cstrOutput, "}"));
	assert(isEqual(cstrOutput, "url"));

	bEnd = readNextNode(pFile, cstrOutput);
	
	if(isEqual(cstrOutput, "["))
	{
		bEnd = readNextNode(pFile, cstrOutput);
	}

	const size_t length = strlen(cstrOutput); //sizeof(cstrOutput)/sizeof(cstrOutput[0]);

	// Remove /, \ and " from the beginning of the name
	int nFirstLetter(0);
	for(size_t i = 0; i < length; ++i)
	{
		if(cstrOutput[i] != '\\' && cstrOutput[i] != '/'  && cstrOutput[i] != '\"')
		{
			break;
		}

		++nFirstLetter;
	}

	int nLastLetter = static_cast<int>(length)-1;
	for(size_t i = 0; i < length; ++i)
	{
		if(cstrOutput[length-i-1] != '\\' && cstrOutput[length-i-1] != '/'  && cstrOutput[length-i-1] != '\"')
		{
			break;
		}

		--nLastLetter;
	}

	strncpy(cstrTextureName, cstrOutput+nFirstLetter, nLastLetter-nFirstLetter+1);
	return bEnd;
}

void FileLoader::processBlockStructure(FILE* pFile, char* strOutput, std::vector<double>& values)
{
	bool bEnd = readNodeBlocksUntil(pFile, "{", strOutput);;
	while(!bEnd)
	{
		double number(0.0);
		if(!readNextNumber(pFile, FLOATING_POINT, strOutput, number))
		{
			break;
		}

		values.push_back(number);
	}
}