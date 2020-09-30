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

#include "FileWriter.h"
#include "FileLoader.h"

#ifdef _WIN32
#include <direct.h> 
#else
#include <sys/stat.h>
#endif

#include <vector>
#include <fstream>

void FileWriter::makeDirectory(const std::string& sstrPath)
{
#ifdef _WIN32
  _mkdir(sstrPath.c_str());
#else
  mkdir(sstrPath.c_str(), 0777);
#endif
}

std::string FileWriter::getFileExtension(const std::string& sstrFileName)
{
	if(sstrFileName.empty())
	{
		return "";
	}

	const size_t pos = sstrFileName.find_last_of(".");
	if(pos != std::string::npos)
	{
		return sstrFileName.substr(pos+1);
	}

	return "";
}

bool FileWriter::saveFile(const std::string& sstrFileName, const DataContainer& data)
{
	if(sstrFileName.empty())
	{
		return false;
	}

	const std::string suffix = FileWriter::getFileExtension(sstrFileName);
	if(suffix==std::string("off"))
	{
		return FileWriter::writeOff(sstrFileName, data);
	}
	else if(suffix==std::string("wrl"))
	{
		return FileWriter::writeWrl(sstrFileName, data);
	}
	else if(suffix==std::string("pset"))
	{
		return false;
	}

	return false;
}

bool FileWriter::saveFile(const std::string& sstrFileName, const DataContainer& data, const std::vector<double>& addPoints, const std::vector<double>& addColors)
{
	if(sstrFileName.empty())
	{
		return false;
	}

	const std::string suffix = FileWriter::getFileExtension(sstrFileName);
	if(suffix==std::string("off"))
	{
		return FileWriter::writeOff(sstrFileName, data);
	}
	else if(suffix==std::string("wrl"))
	{
		return FileWriter::writeWrl(sstrFileName, data, addPoints, addColors);
	}
	else if(suffix==std::string("pset"))
	{

	}

	return false;
}

bool FileWriter::saveLandmarks(const std::string& sstrFileName, const std::vector<double>& landmarks, const std::vector<bool>& valid)
{
	if(sstrFileName.empty())
	{
		return false;
	}

	const std::string suffix = FileLoader::getFileExtension(sstrFileName);
	if(suffix==std::string("lm3"))
	{
		return saveBosphorusLandmarks(sstrFileName, landmarks, valid);
	}
	else if(suffix==std::string("tlmk"))
	{
		return saveSpecifiedTLandmarks(sstrFileName, landmarks, valid);
	}

	return false;
}

bool FileWriter::saveMultilinearModel(const std::string& sstrFileName, std::vector<size_t>& modeDims, std::vector<size_t>& truncModeDims, std::vector<double>& multModel
												  , std::vector<double>& uMatrices, std::vector<double>& sVectors, std::vector<double>& mean)
{
	if(sstrFileName.empty())
	{
		return false;
	}

	if(modeDims.empty() || modeDims.size() != truncModeDims.size())
	{
		return false;
	}

	const char* cstrFileName = sstrFileName.c_str();
	if(cstrFileName == NULL)
	{
		return false;
	}

	std::fstream output;
	output.open(cstrFileName, std::ios::out);

	const size_t numModes = modeDims.size();
	output << numModes << std::endl;

	for(size_t i = 0; i < numModes; ++i)
	{
		output << modeDims[i] << " ";
	}
	output << "#Mode dimensions" << std::endl;

	for(size_t i = 0; i < numModes; ++i)
	{
		output << truncModeDims[i] << " ";
	}
	output << "#Truncated mode dimensions" << std::endl;
	output << std::endl;

	output << "MultilinearModel" << std::endl;
	output << "{" << std::endl;

	std::vector<double>::const_iterator currModelIter = multModel.begin();
	std::vector<double>::const_iterator endModelIter = multModel.end();
	for(; currModelIter != endModelIter; ++currModelIter)
	{
		output << *currModelIter << " ";
	}

	output << std::endl;
	output << "}" << std::endl;
	output << std::endl;
	
	for(size_t i = 1; i < numModes; ++i)
	{
		output << "UMatrix" << std::endl;
		output << "{" << std::endl;

		const size_t numEntries = modeDims[i]*truncModeDims[i];
		size_t startIndex = 0;
		for(size_t j = 1; j < i; ++j)
		{
			startIndex += modeDims[j]*truncModeDims[j];
		}


		for(size_t j = 0; j < numEntries; ++j)
		{
			output << uMatrices[j+startIndex] << " ";
		}

		output << std::endl;
		output << "}" << std::endl;
		output << std::endl;
	}

	for(size_t i = 1; i < numModes; ++i)
	{
		output << "SVector" << std::endl;
		output << "{" << std::endl;

		const size_t numEntries = truncModeDims[i];
		size_t startIndex = 0;
		for(size_t j = 1; j < i; ++j)
		{
			startIndex += truncModeDims[j];
		}


		for(size_t j = 0; j < numEntries; ++j)
		{
			output << sVectors[j+startIndex] << " ";
		}

		output << std::endl;
		output << "}" << std::endl;
		output << std::endl;
	}

	output << "Mean" << std::endl;
	output << "{" << std::endl;
	
	for(size_t i = 0; i < modeDims[0]; ++i)
	{
		output << mean[i] << " ";
	}

	output << std::endl;
	output << "}" << std::endl;

	output.close();
	return true;
}

bool FileWriter::saveRestrictedMultilinearModel(const std::string& sstrFileName, std::vector<size_t>& modeDims, std::vector<size_t>& truncModeDims, std::vector<double>& multModel
															, std::vector<double>& modeMean, std::vector<double>& neutralModelMean, std::vector<double>& mean)
{
	if(sstrFileName.empty())
	{
		return false;
	}

	if(modeDims.empty() || modeDims.size() != truncModeDims.size())
	{
		return false;
	}

	const char* cstrFileName = sstrFileName.c_str();
	if(cstrFileName == NULL)
	{
		return false;
	}

	std::fstream output;
	output.open(cstrFileName, std::ios::out);

	const size_t numModes = modeDims.size();
	output << numModes << std::endl;

	for(size_t i = 0; i < numModes; ++i)
	{
		output << modeDims[i] << " ";
	}
	output << "#Mode dimensions" << std::endl;

	for(size_t i = 0; i < numModes; ++i)
	{
		output << truncModeDims[i] << " ";
	}
	output << "#Truncated mode dimensions" << std::endl;
	output << std::endl;

	output << "MultilinearModel" << std::endl;
	output << "{" << std::endl;

	std::vector<double>::const_iterator currModelIter = multModel.begin();
	std::vector<double>::const_iterator endModelIter = multModel.end();
	for(; currModelIter != endModelIter; ++currModelIter)
	{
		output << *currModelIter << " ";
	}

	output << std::endl;
	output << "}" << std::endl;
	output << std::endl;
	
	output << "ModeMean" << std::endl;
	output << "{" << std::endl;
	
	for(size_t i = 0; i < modeMean.size(); ++i)
	{
		output << modeMean[i] << " ";
	}

	output << std::endl;
	output << "}" << std::endl;
	output << std::endl;

	output << "NeutralModeMean" << std::endl;
	output << "{" << std::endl;
	
	for(size_t i = 0; i < neutralModelMean.size(); ++i)
	{
		output << neutralModelMean[i] << " ";
	}

	output << std::endl;
	output << "}" << std::endl;
	output << std::endl;

	output << "Mean" << std::endl;
	output << "{" << std::endl;
	
	for(size_t i = 0; i < mean.size(); ++i)
	{
		output << mean[i] << " ";
	}

	output << std::endl;
	output << "}" << std::endl;

	output.close();
	return true;
}

bool FileWriter::savePCAModel(const std::string& sstrFileName, const size_t inputDimension, const size_t outputDimension, const std::vector<double>& mean, const std::vector<double>& basis, const std::vector<double>& singularValues)
{
	std::fstream outStream;
	outStream.open(sstrFileName, std::ios::out);

	outStream << inputDimension << " #InputDimension" << std::endl; 
	outStream << outputDimension << " #OutputDimension" << std::endl; 
	outStream << std::endl;
	outStream << "Mean" << std::endl;
	outStream << "{" << std::endl;

	for(size_t i = 0; i < inputDimension; ++i)
	{
		outStream << mean[i] << " ";
	}
	outStream << std::endl;
	outStream << "}" << std::endl;
	outStream << std::endl;

	outStream << "Basis" << std::endl;
	outStream << "{" << std::endl;

	for(size_t row = 0; row < inputDimension; ++row)
	{
		for(size_t col = 0; col < outputDimension; ++col)
		{
			outStream << basis[col*inputDimension+row] << " ";
		}
		outStream << std::endl;
	}

	outStream << std::endl;
	outStream << "}" << std::endl;
	outStream << std::endl;

	outStream << "SingularValues" << std::endl;
	outStream << "{" << std::endl;
	for(size_t i = 0; i < outputDimension; ++i)
	{
		outStream << singularValues[i] << " ";
	}

	outStream << std::endl;
	outStream << "}" << std::endl;
	outStream << std::endl;

	outStream.close();

	return true;
}

bool FileWriter::saveSequenceWeights(const std::string& sstrShapeWeightFileName, const std::string& sstrExpressionWeightFileName, const std::vector<double>& sequenceWeights, const size_t shapeWeightDim, const size_t numFrames, const size_t expWeightDim)
{
	if(sstrShapeWeightFileName.empty() || sstrExpressionWeightFileName.empty())
	{
		return false;
	}

	if(sequenceWeights.size() != shapeWeightDim+numFrames*expWeightDim)
	{
		return false;
	}

	const char* cstrW2OutputPath = sstrShapeWeightFileName.c_str();
	if(cstrW2OutputPath==NULL)
	{
		return false;
	}

	std::fstream shapeWeightsOut;
	shapeWeightsOut.open(cstrW2OutputPath, std::ios::out);

	shapeWeightsOut << 1 << " " << shapeWeightDim << std::endl;

	for(size_t i = 0; i < shapeWeightDim; ++i)
	{
		shapeWeightsOut << sequenceWeights[i] << " ";
	}

	shapeWeightsOut.close();
	
	const char* cstrW3OutputPath = sstrExpressionWeightFileName.c_str();
	if(cstrW3OutputPath==NULL)
	{
		return false;
	}

	std::fstream expWeightsOut;
	expWeightsOut.open(cstrW3OutputPath, std::ios::out);

	expWeightsOut << numFrames << " " << expWeightDim << std::endl;

	for(size_t frame = 0; frame < numFrames; ++frame)
	{
		for(size_t i = 0; i < expWeightDim; ++i)
		{
			const size_t index = shapeWeightDim+frame*expWeightDim+i;
			expWeightsOut << sequenceWeights[index] << " ";
		}

		expWeightsOut << std::endl;
	}

	expWeightsOut.close();
	return true;
}

bool FileWriter::saveThinPlateSpline(const std::string& sstrFileName, const std::vector<double>& vecC, const std::vector<double>& matA, const std::vector<double>& matW, const std::vector<double>& sourcePoints)
{
	if(sstrFileName.empty())
	{
		return false;
	}

	const size_t targetDim = vecC.size();
	const size_t sourceDim = matA.size()/targetDim;
	const size_t numSourcePoints = sourcePoints.size()/sourceDim;
	if(matW.size() != numSourcePoints*targetDim)
	{
		return false;
	}

	std::fstream output(sstrFileName, std::ios::out);	

	if(!output.is_open())
	{
		return false;
	}

	output << sourceDim << " #source dimension" << std::endl;
	output << targetDim << " #target dimension" << std::endl;
	output << numSourcePoints << " #number of source points" << std::endl;
	output << std::endl;

	output << "c" << std::endl;
	output << "{" << std::endl;

	for(size_t i = 0; i < targetDim; ++i)
	{
		output << vecC[i] << " ";
	}
	output << std::endl;
	output << "}" << std::endl;
	output << std::endl;

	output << "A" << std::endl;
	output << "{" << std::endl;

	for(size_t i = 0; i < targetDim; ++i)
	{
		for(size_t j = 0; j < sourceDim; ++j)
		{
			output << matA[j*targetDim+i] << " ";
		}
		output << std::endl;
	}
	output << "}" << std::endl;
	output << std::endl;

	output << "W" << std::endl;
	output << "{" << std::endl;

	for(size_t i = 0; i < numSourcePoints; ++i)
	{
		for(size_t j = 0; j < targetDim; ++j)
		{
			output << matW[j*numSourcePoints+i] << " ";
		}
		output << std::endl;
	}
	output << "}" << std::endl;
	output << std::endl;

	output << "points #source data points" << std::endl;
	output << "{" << std::endl;

	for(size_t i = 0; i < numSourcePoints; ++i)
	{
		for(size_t j = 0; j < sourceDim; ++j)
		{
			output << sourcePoints[i*sourceDim+j] << " ";
		}
		output << std::endl;
	}
	output << "}" << std::endl;
	output << std::endl;

	output.close();

	return true;
}

bool FileWriter::writeOff(const std::string& sstrFileName, const DataContainer& data)
{
	if(sstrFileName.empty())
	{
		return false;
	}

	const size_t numVertices = data.getNumVertices();
	const size_t numFaces = data.getVertexIndexList().size();
	const size_t numEdges = 0;

	const char* cstrFileName = sstrFileName.c_str();
	if(cstrFileName==NULL)
	{
		return false;
	}

	std::fstream output;
	output.open(cstrFileName, std::ios::out);
	output.precision(7);
	output.setf(std::ios::fixed,std::ios::floatfield);

	const std::vector<double>& vertices = data.getVertexList();
	const std::vector<double>& vertexColors = data.getVertexColorList();

	bool bValidColors = vertices.size() == vertexColors.size();

	if(bValidColors)
	{
		output << "COFF" << std::endl;
	}
	else
	{
		output << "OFF" << std::endl;
	}

	output << numVertices << " " << numFaces << " " << numEdges << std::endl;

	for(size_t i = 0; i < numVertices; ++i)
	{
		output << vertices[3*i] << " " << vertices[3*i+1] << " " << vertices[3*i+2] << " ";
		if(bValidColors)
		{
			output << vertexColors[3*i] << " " << vertexColors[3*i+1] << " " << vertexColors[3*i+2] << " " << "1.0";
		}

		output << std::endl;
	}

	const std::vector<std::vector<int>>& vertexIndexList = data.getVertexIndexList();
	for(size_t i = 0; i < vertexIndexList.size(); ++i)
	{
		const std::vector<int>& currPolyIndices = vertexIndexList[i];
		output << currPolyIndices.size() << " ";
		for(size_t j = 0; j < currPolyIndices.size(); ++j)
		{
			output << currPolyIndices[j] << " ";
		}
		output << std::endl;
	}

	output.close();
	return true;
}

bool FileWriter::writeWrl(const std::string& sstrFileName, const DataContainer& data)
{
	if(sstrFileName.empty())
	{
		return false;
	}

	const char* cstrFileName = sstrFileName.c_str();
	if(cstrFileName==NULL)
	{
		return false;
	}

	std::fstream output;
	output.open(cstrFileName, std::ios::out);
	output.precision(7);
	output.setf(std::ios::fixed,std::ios::floatfield);

	output << "#VRML V2.0 utf8" << std::endl;
 
	output << "Transform {\n"
			<< "children [\n"
			<< "Shape {\n";

	const std::string sstrTextureName = data.getTextureName();
	if(!sstrTextureName.empty())
	{
		output << "appearance Appearance {\n"
				<< "texture ImageTexture {\n"
				<< "url ";
				
		output << "\"" << sstrTextureName << "\"" << std::endl;

		output << "} #ImageTexture\n"
			<< "} #Appearance\n";
	}

	output << "geometry IndexedFaceSet {\n";

	const size_t numVertices = data.getNumVertices();
	if(numVertices > 0)
	{
		output << "coord Coordinate {\n"
				<< "point [\n";

		const std::vector<double>& vertices = data.getVertexList();
		for(size_t i = 0; i < numVertices; ++i)
		{
			output << vertices[3*i] << " " <<  vertices[3*i+1] << " " << vertices[3*i+2] << "," << std::endl;
		}

		output << "] #point\n"
			<< "} #Coordinate\n";
	}

	const size_t numFaces = data.getNumFaces();
	if(numFaces > 0)
	{
		output << "coordIndex [\n";

		const std::vector<std::vector<int>>& vertexIndexList = data.getVertexIndexList();
		for(size_t i = 0; i < numFaces; ++i)
		{
			const std::vector<int>& currFaceIndices = vertexIndexList[i];
			for(size_t j = 0; j < currFaceIndices.size(); ++j)
			{
				output << currFaceIndices[j] << ", ";
			}
			output << "-1" << std::endl;
		}

		output << "] #coordIndex\n";
	}

	const std::vector<double>& vertexColorList = data.getVertexColorList();
	if(!vertexColorList.empty())
	{
		output << "color Color {\n"
				<< "color [\n";

		const size_t numVertexColors = vertexColorList.size()/3;
		for(size_t i = 0; i < numVertexColors; ++i)
		{
			output << vertexColorList[3*i] << " " <<  vertexColorList[3*i+1] << " " << vertexColorList[3*i+2] << std::endl;
		}

		output << "] #color\n"
			<< "} #Color\n";
	}

	const std::vector<double>& textureList = data.getTextureList();
	if(!textureList.empty())
	{
		output << "texCoord TextureCoordinate {\n"
				<< "point [\n";

		const size_t numTextureCoords = textureList.size()/2;
		for(size_t i = 0; i < numTextureCoords; ++i)
		{
			output << textureList[2*i] << " " <<  textureList[2*i+1] << std::endl;
		}

		output << "] #point\n"
				<< "} #TextureCoordinate\n";
	}

	const std::vector<std::vector<int>>& textureIndexList = data.getTextureIndexList();
	if(!textureIndexList.empty())
	{
		output << "texCoordIndex [\n";
		
		for(size_t i = 0; i < textureIndexList.size(); ++i)
		{
			const std::vector<int>& currTextureIndices = textureIndexList[i];
			for(size_t j = 0; j < currTextureIndices.size(); ++j)
			{
				output << currTextureIndices[j] << ", ";
			}

			output << "-1" << std::endl;
		}

		output << "] #texCoordIndex\n";
	}

	output << "} #IndexedFaceSet\n";

	output << "} #Shape\n"
		<< "] #children\n"
		<< "} #Transform\n";

	output.close();
	return true;
}

bool FileWriter::writeWrl(const std::string& sstrFileName, const DataContainer& data, const std::vector<double>& addPoints, const std::vector<double>& addColors)
{
	if(!writeWrl(sstrFileName, data))
	{
		return false;
	}

	const char* cstrFileName = sstrFileName.c_str();
	if(cstrFileName==NULL)
	{
		return false;
	}

	std::fstream output;
	output.open(cstrFileName, std::ios::app);
	output.precision(7);
	output.setf(std::ios::fixed,std::ios::floatfield);

	output << "\n";

	for(size_t i = 0; i < addPoints.size()/3; ++i)
	{
		output << "Transform {\n"
				<< "translation ";

		output << addPoints[3*i] << " " << addPoints[3*i+1] << " " << addPoints[3*i+2] << std::endl;

		output << "rotation 1 0 0 1.57\n"
				<< "children [\n"
				<< "Shape {\n"
				<< "appearance Appearance {\n"
				<< "material Material {\n"
				<< "diffuseColor ";

		if(!addColors.empty())
		{
			output << addColors[3*i] << " " << addColors[3*i+1] << " " << addColors[3*i+2] << std::endl;
		}
		else
		{
			output << "0.0 0.0 0.0" << std::endl;
		}

		output << "} #Material\n"
				<< "} #Appearance\n";

		output << "geometry Sphere {\n"
				<< "radius ";

		output << "2.0" << std::endl;

		output << "} #Sphere\n"
			<< "} #Shape\n"
			<< "] #children\n"
			<< "} #Transform\n"
			<< "\n";
	}

	output.close();
	return true;
}

bool FileWriter::saveBosphorusLandmarks(const std::string& sstrFileName, const std::vector<double>& landmarks, const std::vector<bool>& valid)
{
	return saveSpecifiedTLandmarks(sstrFileName, landmarks, valid);
}

bool FileWriter::saveSpecifiedTLandmarks(const std::string& sstrFileName, const std::vector<double>& landmarks, const std::vector<bool>& valid)
{
	const char* cstrFileName = sstrFileName.c_str();
	if(cstrFileName==NULL)
	{
		return false;
	}

	std::fstream output;
	output.open(cstrFileName, std::ios::out);
	output.precision(7);
	output.setf(std::ios::fixed,std::ios::floatfield);

	output << "# Specified 3D Landmark File" << std::endl;

	size_t numLandmarks(0);
	for(size_t i = 0; i < valid.size(); ++i)
	{
		if(valid[i])
		{
			++numLandmarks;
		}
	}

	output << numLandmarks << " landmarks:" << std::endl;

	for(size_t i = 0; i < landmarks.size()/3; ++i)
	{
		if(!valid[i])
		{
			continue;
		}

		if(i == 0)
		{
			output << "Outer left eye corner" << std::endl;
		}
		else if(i == 1)
		{
			output << "Inner left eye corner" << std::endl;
		}
		else if(i == 2)
		{
			output << "Inner right eye corner" << std::endl;
		}
		else if(i == 3)
		{
			output << "Outer right eye corner" << std::endl;
		}
		else if(i == 4)
		{
			output << "Nose tip" << std::endl;
		}
		else if(i == 5)
		{
			output << "Left nose peak" << std::endl;
		}
		else if(i == 6)
		{
			output << "Right nose peak" << std::endl;
		}
		else if(i == 7)
		{
			output << "Subnasal point" << std::endl;
		}
		else if(i == 8)
		{
			output << "Left mouth corner" << std::endl;
		}
		else if(i == 9)
		{
			output << "Right mouth corner" << std::endl;
		}
		else if(i == 10)
		{
			output << "Upper lip outer middle" << std::endl;
		}
		else if(i == 11)
		{
			output << "Lower lip outer middle" << std::endl;
		}
		else if(i == 12)
		{
			output << "Chin middle" << std::endl;
		}

		output << landmarks[3*i] << " ";
		output << landmarks[3*i+1] << " ";
		output << landmarks[3*i+2] << std::endl;
	}

	output.close();
	return true;
}