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

#ifndef TEMPLATEFITTING_H
#define TEMPLATEFITTING_H

#include "DataContainer.h"
#include "KDTree3.h"

#include <vnl/vnl_vector.h>

#include <set>

class TemplateFitting
{
public:
	static void fitTemplate(const DataContainer& templateMesh, const DataContainer& targetMesh, DataContainer& outMesh); 


private:

	static void computeNearestNeighbors(const std::vector<double>& sourceVertices, const std::vector<double>& sourceNormals, const std::vector<double>& targetVertices, const std::vector<double>& targetNormals, const KDTree3& targetKDTree
													, const double maxDist, const double maxAngle, std::vector<double>& nearestNeighbors, std::vector<bool>& validValues);

	static void updateTransformation(const std::vector<double>& sourceVertices, const vnl_vector<double>& trafo, std::vector<double>& trafoVertices);

	static void computeEdges(const DataContainer& mesh, std::vector<std::pair<int,int>>& templateEdges);
};

#endif