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

#ifndef MathHelper_H
#define MathHelper_H

#include "DataContainer.h"
#include "VectorNX.h"

#include <set>
#include <stdlib.h>

class MathHelper
{
public:
	//! Compute normals, based on Max1999 - Weights for Computing Vertex Normals from Facet Normals
	static void computeVertexNormals(const DataContainer& poly, std::vector<double>& vertexNormals);

	//Compute projection of p1 into tangential plane of p2
	static void getPlaneProjection(const Vec3d& p1, const Vec3d& p2, const Vec3d& n2, Vec3d& outPoint);

	static void transformMesh(const double s, const std::vector<double>& R, const std::string& sstrRotOp, const std::vector<double>& t, const std::string& sstrTransOp, DataContainer& mesh);

	static void transformData(const double s, const std::vector<double>& R, const std::string& sstrRotOp, const std::vector<double>& t, const std::string& sstrTransOp, std::vector<double>& data);

	//! Computes scaling, rotation and translation for modelData = s*R*sequenceData+t
	static bool computeAlignmentTrafo(const std::vector<double>& source, const std::vector<double>& target, double& s, std::vector<double>& R, std::vector<double>& t, bool bScaling = true);
	
	static bool computeScaling(const std::vector<double>& source, const std::vector<double>& target, double& s);

	//! Calculating best solution of R*source = target
	static bool computeRotationAlignmentMatrix(const std::vector<double>& source, const std::vector<double>& target, std::vector<double>& R);

	static void centerData(std::vector<double>& data, std::vector<double>& mean);

	static void rotateData(const std::vector<double>& R, const std::string& sstrOp, std::vector<double>& data);

	static void translateData(const std::vector<double>& t, const std::string& sstrOp, std::vector<double>& data);

	static void scaleData(const double factor, std::vector<double>& data);

	static void cleanMesh(DataContainer& mesh);
};

#endif