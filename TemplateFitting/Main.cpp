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

#include <string>
#include <iostream>

#include "DataContainer.h"
#include "FileLoader.h"
#include "FileWriter.h"
#include "TemplateFitting.h"
#include "MathHelper.h"

int computeTempateFitting(const std::string& sstrTemplateFile, const std::string& sstrTargetFile, const std::string& sstrOutFile)
{
	if(!FileLoader::fileExist(sstrTemplateFile))
	{
		std::cout << "Template file does not exist " << sstrTemplateFile << std::endl;
		return 1;
	}

	if(!FileLoader::fileExist(sstrTargetFile))
	{
		std::cout << "Target file does not exist " << sstrTargetFile << std::endl;
		return 1;
	}

	FileLoader loader;
	
	DataContainer templateMesh;
	if(!loader.loadFile(sstrTemplateFile, templateMesh))
	{
		std::cout << "Unable to load template file " << sstrTemplateFile << std::endl;
		return 1;
	}

	DataContainer targetMesh;
	if(!loader.loadFile(sstrTargetFile, targetMesh))
	{
		std::cout << "Unable to load target file " << sstrTargetFile << std::endl;
		return 1;
	}

	DataContainer outMesh;
	TemplateFitting::fitTemplate(templateMesh, targetMesh, outMesh);
	

	if(!FileWriter::saveFile(sstrOutFile, outMesh))
	{
		std::cout << "Unable to save file " << sstrOutFile << std::endl;
		return 1;
	}

	return 0;
}

int computeAlignedTempateFitting(const std::string& sstrTemplateFile, const std::string& sstrTemplateLmkFile, const std::string& sstrTargetFile, const std::string& sstrTargetLmkFile, const std::string& sstrOutFile)
{
	if(!FileLoader::fileExist(sstrTemplateFile))
	{
		std::cout << "Template file does not exist " << sstrTemplateFile << std::endl;
		return 1;
	}

	if(!FileLoader::fileExist(sstrTemplateLmkFile))
	{
		std::cout << "Template landmark file does not exist " << sstrTemplateLmkFile << std::endl;
		return 1;
	}

	if(!FileLoader::fileExist(sstrTargetFile))
	{
		std::cout << "Target file does not exist " << sstrTargetFile << std::endl;
		return 1;
	}

	if(!FileLoader::fileExist(sstrTargetLmkFile))
	{
		std::cout << "Target landmark file does not exist " << sstrTargetLmkFile << std::endl;
		return 1;
	}

	FileLoader loader;
	
	DataContainer templateMesh;
	if(!loader.loadFile(sstrTemplateFile, templateMesh))
	{
		std::cout << "Unable to load template file " << sstrTemplateFile << std::endl;
		return 1;
	}

	std::vector<double> templateLmks;
	if(!loader.loadDataFile(sstrTemplateLmkFile, templateLmks))
	{
		std::cout << "Unable to load template landmark file " << sstrTemplateLmkFile << std::endl;
		return 1;
	}

	DataContainer targetMesh;
	if(!loader.loadFile(sstrTargetFile, targetMesh))
	{
		std::cout << "Unable to load target file " << sstrTargetFile << std::endl;
		return 1;
	}

	std::vector<double> targetLmks;
	if(!loader.loadDataFile(sstrTargetLmkFile, targetLmks))
	{
		std::cout << "Unable to load target landmark file " << sstrTargetLmkFile << std::endl;
		return 1;
	}

	//Compute rigid landmark alignment
	double s(1.0);
	std::vector<double> R;
	std::vector<double> t;
	if(!MathHelper::computeAlignmentTrafo(templateLmks, targetLmks, s, R, t))
	{
		std::cout << "Unable to compute rigid landmark alignment" << std::endl;
	}

	//Transform template mesh
	MathHelper::transformMesh(s, R, "N", t, "+", templateMesh);

	DataContainer outMesh;
	TemplateFitting::fitTemplate(templateMesh, targetMesh, outMesh);
	
	if(!FileWriter::saveFile(sstrOutFile, outMesh))
	{
		std::cout << "Unable to save file " << sstrOutFile << std::endl;
		return 1;
	}
	else
	{
		std::cout << "Successfull " << sstrOutFile << std::endl;
	}

	return 0;

}

int main(int argc, char* argv[])
{
	if(argc == 4)
	{
		const std::string sstrTemplateFile(argv[1]);
		const std::string sstrTargetFile(argv[2]);
		const std::string sstrOutFile(argv[3]);
		return computeTempateFitting(sstrTemplateFile, sstrTargetFile, sstrOutFile);
	}
	else if(argc == 6)
	{
		const std::string sstrTemplateFile(argv[1]);
		const std::string sstrTemplateLmkFile(argv[2]);
		const std::string sstrTargetFile(argv[3]);
		const std::string sstrTargetLmkFile(argv[4]);
		const std::string sstrOutFile(argv[5]);
		return computeAlignedTempateFitting(sstrTemplateFile, sstrTemplateLmkFile, sstrTargetFile, sstrTargetLmkFile, sstrOutFile);
	}
	else
	{
		std::cout << "Wrong number of parameters " << argc << std::endl;
		return 1;
	}

	return 0;
}