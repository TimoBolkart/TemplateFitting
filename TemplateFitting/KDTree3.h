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

#ifndef KDTREE3_H
#define KDTREE3_H

#include <ANN\ANN.h>

#include <vector>

//! kd tree
class KDTree3
{
public:
	//! Construct kd tree for a set of 3d vertices.
	//! \param points				3d vertices
	KDTree3(const std::vector<double>& points);

	~KDTree3();

	//! Get nearest neighbor.
	//! \param point				3d point of request
	//! \param pointIndex		index of nearest neighbor
	//! \param sqrDist			squared Euclidean distance of the point
	//! \return true if successful
	bool getNearestPoint(const std::vector<double>& point, int& pointIndex, double& sqrDist) const;

	//! Get k nearest neighbors.
	//! \param point				3d point of request
	//! \param k					number of requested nearest neighbors
	//! \param pointIndexVec	vector of nearest neighbor indices
	//! \param sqrDistVec		vector of squared Euclidean distances
	//! \return true if successful
	bool getKNearestPoints(const std::vector<double>& point, const size_t k, std::vector<int>& pointIndexVec, std::vector<double>& sqrDistVec) const;

private:
	KDTree3(const KDTree3& kdTree);
	
	KDTree3& operator=(const KDTree3& kdTree);

	ANNpointArray m_pointArray;
	ANNkd_tree* m_pKDTree;
};

#endif