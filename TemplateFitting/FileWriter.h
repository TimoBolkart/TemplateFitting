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

#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <stdlib.h>
#include "DataContainer.h"

#include <vector>
#include <string>

class FileWriter
{
public:
	static void makeDirectory(const std::string& sstrPath);

	static std::string getFileExtension(const std::string& sstrFileName);

	static bool saveFile(const std::string& sstrFileName, const DataContainer& data);

	static bool saveLandmarks(const std::string& sstrFileName, const std::vector<double>& landmarks, const std::vector<bool>& valid);

	static bool saveMultilinearModel(const std::string& sstrFileName, std::vector<size_t>& modeDims, std::vector<size_t>& truncModeDims, std::vector<double>& multModel
												  , std::vector<double>& uMatrices, std::vector<double>& sVectors, std::vector<double>& mean);

	static bool saveRestrictedMultilinearModel(const std::string& sstrFileName, std::vector<size_t>& modeDims, std::vector<size_t>& truncModeDims, std::vector<double>& multModel
															, std::vector<double>& modeMean, std::vector<double>& neutralModelMean, std::vector<double>& mean);

	static bool savePCAModel(const std::string& sstrFileName, const size_t inputDimension, const size_t outputDimension, const std::vector<double>& mean, const std::vector<double>& basis, const std::vector<double>& singularValues);

	static bool saveSequenceWeights(const std::string& qstrShapeWeightFileName, const std::string& qstrExpressionWeightFileName, const std::vector<double>& sequenceWeights, const size_t shapeWeightDim, const size_t numFrames, const size_t expWeightDim);

	/**/static bool saveFile(const std::string& sstrFileName, const DataContainer& data, const std::vector<double>& addPoints, const std::vector<double>& addColors);

	static bool saveThinPlateSpline(const std::string& sstrFileName, const std::vector<double>& vecC, const std::vector<double>& matA, const std::vector<double>& matW, const std::vector<double>& sourcePoints);

private:

	static bool writeOff(const std::string& qstrFileName, const DataContainer& data);

	static bool writeWrl(const std::string& qstrFileName, const DataContainer& data);

	/**/static bool writeWrl(const std::string& sstrFileName, const DataContainer& data, const std::vector<double>& addPoints, const std::vector<double>& addColors);

	static bool saveBosphorusLandmarks(const std::string& sstrFileName, const std::vector<double>& landmarks, const std::vector<bool>& valid);

	static bool saveSpecifiedTLandmarks(const std::string& sstrFileName, const std::vector<double>& landmarks, const std::vector<bool>& valid);
};

#endif