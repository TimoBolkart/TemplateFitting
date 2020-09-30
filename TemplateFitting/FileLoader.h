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

#ifndef FILELOADER_H
#define FILELOADER_H

#include <stdlib.h>
#include "DataContainer.h"

#include <iostream>
#include <assert.h>
#include <cstring>

class FileLoader
{
	enum Type
	{
		FLOATING_POINT,
		INTEGER
	};

public:
	static std::string getFileExtension(const std::string& sstrFileName);

	static bool fileExist(const std::string& sstrFileName);

	static std::string getFileName(const std::string& sstrFullFileName);

	static std::string getFilePath(const std::string& sstrFileName);

	bool loadFile(const std::string& sstrFileName, DataContainer& outData);

	bool loadDataFile(const std::string& sstrDataFileName, std::vector<double>& data);

private:
	bool loadWRL(const std::string& sstrFileName, DataContainer& outData);

	bool loadOFF(const std::string& sstrFileName, DataContainer& outData);

	bool loadObj(const std::string& sstrFileName, DataContainer& outData);

	bool readNextNode(FILE* pFile, char* cstrOutput);

	//! return true if successful, false if not successful or eof is reached
	template<typename Unit>
	bool readNextNumber(FILE* pFile, const Type type, char* strOutput, Unit& number)
	{
		readNextNode(pFile, strOutput);
		return convertToNumber(strOutput, type, number);
	}

	bool processShapeNode(FILE* pFile, char* cstrOutput, DataContainer& outPoly);

	bool processImageTexture(FILE* pFile, char* cstrOutput, char* cstrTextureName);

	void processBlockStructure(FILE* pFile, char* strOutput, std::vector<double>& values);

	template<typename Unit>
	bool convertToNumber(const char* strIn, const Type& type, Unit& out)
	{
		char strBuffer[20];
		for(size_t i = 0; i < 20; ++i)
		{
			if(strIn[i] == ',')
			{
				strBuffer[i] = '\0';
				break;
			}
			else
			{
				strBuffer[i] = strIn[i];
			}
		}

		const char* cstrFormat = type==FLOATING_POINT ? "%lf%*c" : "%ld%*c";
		return sscanf(strBuffer, cstrFormat, &out)==1; 
	}

	template<typename Unit>
	void processCoordinates(FILE* pFile, char* cstrOutput, const size_t dim, std::vector<Unit>& vertexList)
	{

		bool bEnd = readNodeBlocksUntil(pFile, "[", cstrOutput);
		assert(!bEnd && !isEqual(cstrOutput, "}"));

		do
		{
			std::vector<Unit> tmpData;
			if(!readDataBlock(pFile, FLOATING_POINT, cstrOutput, dim, tmpData))
			{
				break;
			}

			for(size_t i = 0; i < tmpData.size(); ++i)
			{
				vertexList.push_back(tmpData[i]);
			}
		}
		while(!isEqual(cstrOutput, "}"));
	}

	void processIndices(FILE* pFile, char* /*cstrOutput*/, std::vector<std::vector<int>>& indexList)
	{
		std::vector<int> endChars;
		endChars.push_back(91); // [
		endChars.push_back(93); // ]
		//endChars.push_back(123); // {
		endChars.push_back(125); // }
		endChars.push_back(10); // new line

		char tmpOutput[1000] = "";
		bool bEnd = readBlockUntil(pFile, endChars, tmpOutput);
		assert(!bEnd && !isEqual(tmpOutput, "}"));

		while(!bEnd)
		{
			std::vector<int> values;
			bEnd = readIndexBlock(pFile, values);
			if(bEnd)
			{
				break;
			}

			indexList.push_back(values);
		}
	}

	template<typename Unit>
	bool readDataBlock(FILE* pFile, const Type& type, char* cstrOutput, const size_t dim, std::vector<Unit>& data)
	{
		data.resize(dim);
		for(size_t i = 0; i < dim; ++i)
		{
			Unit value = static_cast<Unit>(0.0);
			if(readNextNode(pFile, cstrOutput) || !convertToNumber(cstrOutput, type, value)
				|| isEqual(cstrOutput, "}") || isEqual(cstrOutput, "]"))
			{
				return false;
			}
			
			data[i] = value;
		}

		return true;
	}

	bool readIndexBlock(FILE* pFile, std::vector<int>& values)
	{
		bool bEnd(false);
		while(!bEnd)
		{
			char cstrTmpOutput[20] = {0};

			bEnd = getIndexNumberString(pFile, cstrTmpOutput);

			size_t len = strlen(cstrTmpOutput);
			if(len>0)
			{
				int value(0);
				if(!convertToNumber(cstrTmpOutput, INTEGER, value))
				{
					return true;
				}

				if(value==-1)
				{
					return false;
				}

				values.push_back(value);
			}
			
			if(bEnd)
			{
				return true;
			}
		}

		return bEnd;
	}

	bool getIndexNumberString(FILE* pFile, char* cstrOutput)
	{
		char cstrTmpOutput[20] = {0};

		size_t currSize(0);
		bool bEnd(false);
		while(!bEnd)
		{
		  int currChar = fgetc(pFile);
		  
		  if(currChar == EOF)
		  {
		    std::cout << "Reached end of file" << std::endl;
		    bEnd = true;
		    break;
		  }
		  
		  if((currChar >= 48 && currChar <= 57) || currChar == 45 || currChar == 43) 
		  {
		    //Character must be a number 0-9 (char 48 - char 57) or a sign (- 45, + 43)
		    cstrTmpOutput[currSize] = (char)currChar;
		    ++currSize;  
		  }
		  else if(currChar == 125 || currChar == 93)
		  {
		    bEnd = true;
		    break; 
		  }
		  else
		  {
		    if(currSize == 0)
		    {
		      continue;
		    }
		    else
		    {
		      break;
		    } 
		  }
		}

		for(size_t i = 0; i < currSize; ++i)
		{
		  cstrOutput[i] = cstrTmpOutput[i];
		}

		return bEnd;
	}

	//! Reads next block until it reaches the specified character.
	//! Stops reading if the end of file is reached.
	//! Stops reading if "}" or "]" is reached.
	bool readNodeBlocksUntil(FILE* pFile, const char* cstrEnd, char* cstrOutput)
	{
		bool bEnd(false);
		do
		{
			bEnd = readNextNode(pFile, cstrOutput);
		}
		while(!bEnd && !isEqual(cstrOutput, cstrEnd) 
				&& !isEqual(cstrOutput, "}") && !isEqual(cstrOutput, "]"));

		return bEnd;
	}

	bool readBlockUntil(FILE* pFile, const std::vector<int>& stopChars, char* cstrOutput)
	{
		if(stopChars.empty())
		{
			return false;
		}

		memset(cstrOutput, 0, 1000);

		const size_t numStopChars = stopChars.size();

		bool bStop(false);
		size_t currSize(0);
		while(!bStop && currSize < 1000) //One nod is probably not longer than 1000 characters
		{
			const int currChar = fgetc(pFile);
			if(currChar == EOF)
			{
				return true;
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

			for(size_t i = 0; i < numStopChars; ++i)
			{
				if(currChar == stopChars[i])
				{
					bStop = true;
					break;
				}
			}

			if(!bStop)
			{
				cstrOutput[currSize] = (char)currChar;
				++currSize;
			}
		}

		return false;
	}

	bool isEqual(const char* cstrS1, const char* cstrS2)
	{
		return strcmp(cstrS1, cstrS2) == 0;
	}
};

#endif