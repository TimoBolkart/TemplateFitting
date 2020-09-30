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

#include "KDTree3.h"

KDTree3::KDTree3(const std::vector<double>& points)
{
	const size_t numPoints = points.size()/3;

	m_pointArray = annAllocPts(static_cast<int>(numPoints),3);

	for(size_t i = 0; i < numPoints; ++i)
	{
		for(size_t j = 0; j < 3; ++j)
		{
			const size_t index = 3*i+j;
			m_pointArray[i][j] = points[index];
		}
	}

	m_pKDTree = new ANNkd_tree(m_pointArray, static_cast<int>(numPoints), 3); 
}

KDTree3::~KDTree3()
{
	annDeallocPts(m_pointArray);

	delete m_pKDTree;
}

bool KDTree3::getNearestPoint(const std::vector<double>& point, int& pointIndex, double& sqrDist) const
{
	if(point.size() != 3)
	{
		return false;
	}

	ANNpoint queryPoint;
	queryPoint = annAllocPt(3);

	queryPoint[0] = point[0];
	queryPoint[1] = point[1];
	queryPoint[2] = point[2];

	ANNidxArray nnIdx;
	nnIdx = new ANNidx[1];
	
	ANNdistArray sqrDists;
	sqrDists = new ANNdist[1];

	m_pKDTree->annkSearch(queryPoint, 1, nnIdx, sqrDists, 0.0);

	pointIndex = nnIdx[0];
	sqrDist = sqrDists[0];

	annDeallocPt(queryPoint);
	delete [] nnIdx;
	delete [] sqrDists;

	return true;
}

bool KDTree3::getKNearestPoints(const std::vector<double>& point, const size_t k, std::vector<int>& pointIndexVec, std::vector<double>& sqrDistVec) const
{
	if(point.size() != 3 || k < 1)
	{
		return false;
	}

	ANNpoint queryPoint;
	queryPoint = annAllocPt(3);

	queryPoint[0] = point[0];
	queryPoint[1] = point[1];
	queryPoint[2] = point[2];

	ANNidxArray nnIdx;
	nnIdx = new ANNidx[k];

	ANNdistArray sqrDists;
	sqrDists = new ANNdist[k];

	m_pKDTree->annkSearch(queryPoint, static_cast<int>(k), nnIdx, sqrDists);

	pointIndexVec.reserve(k);
	sqrDistVec.reserve(k);

	for(size_t i = 0; i < k; ++i)
	{
		pointIndexVec.push_back(nnIdx[i]);
		sqrDistVec.push_back(sqrDists[i]);
	}

	annDeallocPt(queryPoint);
	delete [] nnIdx;
	delete [] sqrDists;

	return true;
}