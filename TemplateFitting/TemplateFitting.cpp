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

#include "TemplateFitting.h"
#include "TemplateFittingCostFunction.h"
#include "KDTree3.h"
#include "VectorNX.h"
#include "MathHelper.h"
#include "Definitions.h"

#include <iostream>
#include <set>
#include <vnl/vnl_cost_function.h>
#include <vnl/algo/vnl_lbfgsb.h>

void TemplateFitting::fitTemplate(const DataContainer& templateMesh, const DataContainer& targetMesh, DataContainer& outMesh)
{
	//Initialize weights
	double nnWeight = NN_WEIGHT;
	double regWeight = REG_WEIGHT; 
	double rigidWeight = RIGID_WEIGHT;

	const size_t numTemplateVertices = templateMesh.getNumVertices();
	const size_t numParameter = 12*numTemplateVertices;
	
	//Pre-compute template edges
	std::vector<std::pair<int,int>> templateEdges;
	TemplateFitting::computeEdges(templateMesh, templateEdges);

	//Pre-compute target kd tree
	const std::vector<double>& targetVertices = targetMesh.getVertexList();
	KDTree3 targetKDTree(targetVertices);

	//Pre-compute target normals
	std::vector<double> targetNormals;
	MathHelper::computeVertexNormals(targetMesh, targetNormals);

	//Initialize transformation
	vnl_vector<double> trafo(numParameter, 0.0);
	for(size_t i = 0; i < numTemplateVertices; ++i)
	{
		const size_t trafoOffset = 12*i;
		trafo[trafoOffset+0] = 1.0;
		trafo[trafoOffset+4] = 1.0;
		trafo[trafoOffset+8] = 1.0;
	}		

	//Initialize transform
	std::vector<double> sourceVertices;

	//Initialize fitting template
	DataContainer tmpMesh(templateMesh);

	for(size_t iIter = 0; iIter < MAX_NUM_ITER; ++iIter)
	{
		std::cout << "****************************************************" << std::endl;
		std::cout << "Current iteration: " << iIter+1 << " of " << MAX_NUM_ITER << std::endl;

		TemplateFitting::updateTransformation(templateMesh.getVertexList(), trafo, sourceVertices);
		tmpMesh.setVertexList(sourceVertices);

		std::vector<double> sourceNormals;
		MathHelper::computeVertexNormals(tmpMesh, sourceNormals);

		//Compute nearest neighbors used for current iteration
		std::vector<double> nearestNeighbors; 
		std::vector<bool> validValues;
		TemplateFitting::computeNearestNeighbors(sourceVertices, sourceNormals, targetVertices, targetNormals, targetKDTree, MAX_NN_DIST, MAX_ANGLE, nearestNeighbors, validValues);

		TemplateFittingCostFunction fkt(templateMesh.getVertexList(), templateEdges, nearestNeighbors, validValues, nnWeight, regWeight, rigidWeight);

		vnl_lbfgsb minimizer(fkt);
		minimizer.set_cost_function_convergence_factor(1e+7); 
		minimizer.set_projected_gradient_tolerance(1e-5);		
		minimizer.set_max_function_evals(100);

#ifdef OUTPUT_TRACE
		minimizer.set_trace(true);
#endif

		vnl_vector<double> x = trafo;
		minimizer.minimize(x);

		if(minimizer.get_failure_code() == vnl_lbfgsb::CONVERGED_FTOL
		|| minimizer.get_failure_code() == vnl_lbfgsb::CONVERGED_XTOL
		|| minimizer.get_failure_code() == vnl_lbfgsb::CONVERGED_XFTOL
		|| minimizer.get_failure_code() == vnl_lbfgsb::CONVERGED_GTOL)
		{
			trafo = x;
		}
		else if(minimizer.get_failure_code() == vnl_lbfgsb::FAILED_TOO_MANY_ITERATIONS)
		{
			std::cout << "Reached maximum number of function evaluations " << minimizer.get_failure_code() << std::endl;
			if(minimizer.obj_value_reduced())
			{
				std::cout << "Function value reduced" << std::endl;
				trafo = x;
			}
			else
			{
				std::cout << "Function value not reduced" << std::endl;
			}
		}
		else
		{
			std::cout << "Minimizer failed convergence " << minimizer.get_failure_code() << std::endl;
		}

		regWeight = regWeight / 2.0;
		rigidWeight = rigidWeight / 2.0;

		std::cout << "****************************************************" << std::endl;
	}

	std::vector<double> outVertices;
	TemplateFitting::updateTransformation(templateMesh.getVertexList(), trafo, outVertices);

	outMesh = templateMesh;
	outMesh.setVertexList(outVertices);
}

void TemplateFitting::computeNearestNeighbors(const std::vector<double>& sourceVertices, const std::vector<double>& sourceNormals, const std::vector<double>& targetVertices, const std::vector<double>& targetNormals, const KDTree3& targetKDTree
															, const double maxDist, const double maxAngle, std::vector<double>& nearestNeighbors, std::vector<bool>& validValues)
{
	const size_t numVertices = sourceVertices.size()/3;
	
	nearestNeighbors.clear();
	nearestNeighbors.resize(3*numVertices, 0.0);

	validValues.clear();
	validValues.resize(numVertices, false);
	
	for(size_t i = 0; i < numVertices; ++i)
	{
		const Vec3d sourcePoint(sourceVertices[3*i],sourceVertices[3*i+1],sourceVertices[3*i+2]);

		std::vector<double> querySourcePoint;
		querySourcePoint.push_back(sourcePoint[0]);
		querySourcePoint.push_back(sourcePoint[1]);
		querySourcePoint.push_back(sourcePoint[2]);

		int nnPointIndex(0);
		double nnSqrPointDist(0);
		targetKDTree.getNearestPoint(querySourcePoint, nnPointIndex, nnSqrPointDist);

		const Vec3d sourceNormal(sourceNormals[3*i], sourceNormals[3*i+1], sourceNormals[3*i+2]);
		const Vec3d targetNormal(targetNormals[3*nnPointIndex], targetNormals[3*nnPointIndex+1], targetNormals[3*nnPointIndex+2]);
		const double angle = sourceNormal.angle(targetNormal);

		bool bPointValid = (sqrt(nnSqrPointDist) <= maxDist) && (angle <= maxAngle);
		if(bPointValid)
		{
			const Vec3d nnPoint(targetVertices[3*nnPointIndex], targetVertices[3*nnPointIndex+1], targetVertices[3*nnPointIndex+2]);

			Vec3d planeProjectionPoint;					
			MathHelper::getPlaneProjection(sourcePoint, nnPoint, targetNormal, planeProjectionPoint);

			nearestNeighbors[3*i] = planeProjectionPoint[0];
			nearestNeighbors[3*i+1] = planeProjectionPoint[1];
			nearestNeighbors[3*i+2] = planeProjectionPoint[2];

			validValues[i] = true;
		}
	}
}

void TemplateFitting::updateTransformation(const std::vector<double>& sourceVertices, const vnl_vector<double>& trafo, std::vector<double>& trafoVertices)
{
	const size_t numSourceVertices = sourceVertices.size()/3;
	
	trafoVertices.clear();
	trafoVertices.resize(3*numSourceVertices, 0.0);

//#pragma omp parallel for
	for(int i = 0; i < numSourceVertices; ++i)
	{
		const size_t vertexOffset = 3*i;
		const size_t trafoOffset = 12*i;
		const double x = trafo[trafoOffset+0]*sourceVertices[vertexOffset+0]
							+ trafo[trafoOffset+3]*sourceVertices[vertexOffset+1]
							+ trafo[trafoOffset+6]*sourceVertices[vertexOffset+2]
							+ trafo[trafoOffset+9];

		const double y = trafo[trafoOffset+1]*sourceVertices[vertexOffset+0]
							+ trafo[trafoOffset+4]*sourceVertices[vertexOffset+1]
							+ trafo[trafoOffset+7]*sourceVertices[vertexOffset+2]
							+ trafo[trafoOffset+10];

		const double z = trafo[trafoOffset+2]*sourceVertices[vertexOffset+0]
							+ trafo[trafoOffset+5]*sourceVertices[vertexOffset+1]
							+ trafo[trafoOffset+8]*sourceVertices[vertexOffset+2]
							+ trafo[trafoOffset+11];

		trafoVertices[vertexOffset+0] = x;
		trafoVertices[vertexOffset+1] = y;
		trafoVertices[vertexOffset+2] = z;
	}
}

void TemplateFitting::computeEdges(const DataContainer& mesh, std::vector<std::pair<int,int>>& templateEdges)
{
	const std::vector<std::vector<int>>& triangles = mesh.getVertexIndexList();
	
	std::vector<std::set<int>> indices;
	indices.resize(mesh.getNumVertices());

	for(size_t i = 0; i < triangles.size(); ++i)
	{
		const int i1 = triangles[i][0];
		const int i2 = triangles[i][1];
		const int i3 = triangles[i][2];
		
		indices[i1 < i2 ? i1 : i2].insert(i1 < i2 ? i2 : i1);
		indices[i1 < i3 ? i1 : i3].insert(i1 < i3 ? i3 : i1);
		indices[i2 < i3 ? i2 : i3].insert(i2 < i3 ? i3 : i2);
	}

	for(size_t i = 0; i < indices.size(); ++i)
	{
		const std::set<int>& currSet = indices[i];
		std::set<int>::const_iterator currIter = currSet.begin();
		const std::set<int>::const_iterator endIter = currSet.end();
		for(; currIter != endIter; ++currIter)
		{
			templateEdges.push_back(std::make_pair(i,*currIter));
		}
	}
}