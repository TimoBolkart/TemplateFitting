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

#include "TemplateFittingCostFunction.h"
#include "VectorNX.h"

#include <iostream>

const double math_eps = 1.0e-6;

TemplateFittingCostFunction::TemplateFittingCostFunction(const std::vector<double>& templateVertices, const std::vector<std::pair<int,int>>& templateEdges, const std::vector<double>& targetVertices, const std::vector<bool>& validTargetVertices
																			, const double nearestNeighborWeight, const double regularizationWeight, const double rigidWeight)
: vnl_cost_function(4*templateVertices.size())
, m_templateVertices(templateVertices)
, m_templateEdges(templateEdges)
, m_targetVertices(targetVertices)
, m_validTargetVertices(validTargetVertices)
, m_numTemplateVertices(templateVertices.size()/3)
, m_nearestNeighborWeight(nearestNeighborWeight)
, m_regularizationWeight(regularizationWeight)
, m_rigidWeight(rigidWeight)
{
	m_trafoTemplateVertices.resize(templateVertices.size(), 0.0);
}

TemplateFittingCostFunction::~TemplateFittingCostFunction()
{

}

void TemplateFittingCostFunction::compute(const vnl_vector<double>& x, double* f, vnl_vector<double>* g)
{
	//Update current transformed points
	updateTransformation(x);

	//Initialize function value and gradient
	init(f, g);

	//Compute energy and gradient
	addNearestNeighborEnergy(f, g);
	addRegularizationEnergy(x, f, g);
	addRigidEnergy(x, f, g);
}

void TemplateFittingCostFunction::addNearestNeighborEnergy(double* f, vnl_vector<double>* g)
{
	if(m_nearestNeighborWeight < math_eps)
	{
		return;
	}

	std::vector<double> functionValues;
	functionValues.resize(m_numTemplateVertices, 0.0);

#pragma omp parallel for
	for(int i = 0; i < m_numTemplateVertices; ++i)
	{
		if(m_validTargetVertices[i])
		{
			const size_t vertexOffset = 3*i;
			const size_t trafoOffset = 12*i;

			const double tmpDX = m_trafoTemplateVertices[vertexOffset+0]-m_targetVertices[vertexOffset+0];
			const double tmpDY = m_trafoTemplateVertices[vertexOffset+1]-m_targetVertices[vertexOffset+1];
			const double tmpDZ = m_trafoTemplateVertices[vertexOffset+2]-m_targetVertices[vertexOffset+2];
			functionValues[i] = m_nearestNeighborWeight*(std::pow(tmpDX,2)+std::pow(tmpDY,2)+std::pow(tmpDZ,2));

			(*g)[trafoOffset+0] += 2.0*m_nearestNeighborWeight*(tmpDX)*(m_templateVertices[vertexOffset+0]); //t_00
			(*g)[trafoOffset+1] += 2.0*m_nearestNeighborWeight*(tmpDY)*(m_templateVertices[vertexOffset+0]); //t_10
			(*g)[trafoOffset+2] += 2.0*m_nearestNeighborWeight*(tmpDZ)*(m_templateVertices[vertexOffset+0]); //t_20

			(*g)[trafoOffset+3] += 2.0*m_nearestNeighborWeight*(tmpDX)*(m_templateVertices[vertexOffset+1]); //t_01
			(*g)[trafoOffset+4] += 2.0*m_nearestNeighborWeight*(tmpDY)*(m_templateVertices[vertexOffset+1]); //t_11
			(*g)[trafoOffset+5] += 2.0*m_nearestNeighborWeight*(tmpDZ)*(m_templateVertices[vertexOffset+1]); //t_21

			(*g)[trafoOffset+6] += 2.0*m_nearestNeighborWeight*(tmpDX)*(m_templateVertices[vertexOffset+2]); //t_02
			(*g)[trafoOffset+7] += 2.0*m_nearestNeighborWeight*(tmpDY)*(m_templateVertices[vertexOffset+2]); //t_12
			(*g)[trafoOffset+8] += 2.0*m_nearestNeighborWeight*(tmpDZ)*(m_templateVertices[vertexOffset+2]); //t_22

			(*g)[trafoOffset+9] += 2.0*m_nearestNeighborWeight*(tmpDX); //t_03
			(*g)[trafoOffset+10] += 2.0*m_nearestNeighborWeight*(tmpDY); //t_13
			(*g)[trafoOffset+11] += 2.0*m_nearestNeighborWeight*(tmpDZ); //t_23
		}
	}

	for(size_t i = 0; i < m_numTemplateVertices; ++i)
	{
		(*f) += functionValues[i];
	}
}

void TemplateFittingCostFunction::addRegularizationEnergy(const vnl_vector<double>& trafo, double* f, vnl_vector<double>* g)
{
	if(m_regularizationWeight < math_eps)
	{
		return;
	}

	const size_t numEdges = m_templateEdges.size();

	for(int i = 0; i < numEdges; ++i)
	{
		const int id1 = m_templateEdges[i].first;
		const int id2 = m_templateEdges[i].second;

		const int offset1 = id1*12;
		const int offset2 = id2*12;

		for(int j = 0; j < 12; ++j)
		{
			const double diff = trafo[offset1+j]-trafo[offset2+j];

			(*f) += m_regularizationWeight*std::pow(diff, 2);
			
			(*g)[offset1+j] += 2.0*m_regularizationWeight*diff;
			(*g)[offset2+j] -= 2.0*m_regularizationWeight*diff;
		}
	}
}

void TemplateFittingCostFunction::addRigidEnergy(const vnl_vector<double>& trafo, double* f, vnl_vector<double>* g)
{
	if(m_rigidWeight < math_eps)
	{
		return;
	}

	std::vector<double> functionValues;
	functionValues.resize(m_numTemplateVertices, 0.0);

#pragma omp parallel for
	for(int i = 0; i < m_numTemplateVertices; ++i)
	{
		const size_t trafoOffset = 12*i;

		const Vec3d t1(trafo[trafoOffset+0], trafo[trafoOffset+1], trafo[trafoOffset+2]);
		const Vec3d t2(trafo[trafoOffset+3], trafo[trafoOffset+4], trafo[trafoOffset+5]);
		const Vec3d t3(trafo[trafoOffset+6], trafo[trafoOffset+7], trafo[trafoOffset+8]);

		const double t1t2 = t1.dotProduct(t2);
		const double t1t3 = t1.dotProduct(t3);
		const double t2t3 = t2.dotProduct(t3);

		const double t1Length = t1.length();
		const double t2Length = t2.length();
		const double t3Length = t3.length();

		const Vec3d grad1 = t2*t1t2*2.0 + t3*t1t3*2.0 - t1*4.0*(1-std::pow(t1Length,2));
		const Vec3d grad2 = t1*t1t2*2.0 + t3*t2t3*2.0 - t2*4.0*(1-std::pow(t2Length,2));
		const Vec3d grad3 = t1*t1t3*2.0 + t2*t2t3*2.0 - t3*4.0*(1-std::pow(t3Length,2));

		functionValues[i] =  m_rigidWeight*(std::pow(t1t2, 2) + std::pow(t1t3, 2) + std::pow(t2t3, 2) + std::pow(1-std::pow(t1Length, 2),2) + std::pow(1-std::pow(t2Length, 2),2) + std::pow(1-std::pow(t3Length, 2),2));

		(*g)[trafoOffset+0] += m_rigidWeight*grad1[0];
		(*g)[trafoOffset+1] += m_rigidWeight*grad1[1];
		(*g)[trafoOffset+2] += m_rigidWeight*grad1[2];
	
		(*g)[trafoOffset+3] += m_rigidWeight*grad2[0];
		(*g)[trafoOffset+4] += m_rigidWeight*grad2[1];
		(*g)[trafoOffset+5] += m_rigidWeight*grad2[2];

		(*g)[trafoOffset+6] += m_rigidWeight*grad3[0];
		(*g)[trafoOffset+7] += m_rigidWeight*grad3[1];
		(*g)[trafoOffset+8] += m_rigidWeight*grad3[2];
	}

	for(size_t i = 0; i < m_numTemplateVertices; ++i)
	{
		(*f) += functionValues[i];
	}
}

void TemplateFittingCostFunction::updateTransformation(const vnl_vector<double>& trafo)
{
#pragma omp parallel for
	for(int i = 0; i < m_numTemplateVertices; ++i)
	{
		const size_t vertexOffset = 3*i;
		const size_t trafoOffset = 12*i;
		const double x = trafo[trafoOffset+0]*m_templateVertices[vertexOffset+0]
							+ trafo[trafoOffset+3]*m_templateVertices[vertexOffset+1]
							+ trafo[trafoOffset+6]*m_templateVertices[vertexOffset+2]
							+ trafo[trafoOffset+9];

		const double y = trafo[trafoOffset+1]*m_templateVertices[vertexOffset+0]
							+ trafo[trafoOffset+4]*m_templateVertices[vertexOffset+1]
							+ trafo[trafoOffset+7]*m_templateVertices[vertexOffset+2]
							+ trafo[trafoOffset+10];

		const double z = trafo[trafoOffset+2]*m_templateVertices[vertexOffset+0]
							+ trafo[trafoOffset+5]*m_templateVertices[vertexOffset+1]
							+ trafo[trafoOffset+8]*m_templateVertices[vertexOffset+2]
							+ trafo[trafoOffset+11];

		m_trafoTemplateVertices[vertexOffset+0] = x;
		m_trafoTemplateVertices[vertexOffset+1] = y;
		m_trafoTemplateVertices[vertexOffset+2] = z;
	}
}

void TemplateFittingCostFunction::init(double* f, vnl_vector<double>* g)
{
	*f = 0.0;

	const size_t dim = 12*m_numTemplateVertices;
		
#pragma omp parallel for
	for(int i = 0; i < dim; ++i)
	{
		(*g)[i] = 0.0;
	}
}