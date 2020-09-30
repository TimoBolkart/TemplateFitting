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

#ifndef TEMPLATEFITTINGCOSTFUNCTION_H
#define TEMPLATEFITTINGCOSTFUNCTION_H

#include <vnl/vnl_vector.h>
#include <vnl/vnl_cost_function.h>

#include <vector>

class TemplateFittingCostFunction : public vnl_cost_function
{
public:

	TemplateFittingCostFunction(const std::vector<double>& templateVertices, const std::vector<std::pair<int,int>>& templateEdges, const std::vector<double>& targetVertices, const std::vector<bool>& validTargetVertices
										, const double nearestNeighborWeight, const double regularizationWeight, const double rigidWeight);

	~TemplateFittingCostFunction();

	virtual void compute(const vnl_vector<double>& x, double* f, vnl_vector<double>* g);

private:

	void addNearestNeighborEnergy(double* f, vnl_vector<double>* g);

	void addRegularizationEnergy(const vnl_vector<double>& trafo, double* f, vnl_vector<double>* g);

	void addRigidEnergy(const vnl_vector<double>& trafo, double* f, vnl_vector<double>* g);

	void updateTransformation(const vnl_vector<double>& trafo);

	void init(double* f, vnl_vector<double>* g);

	size_t m_numTemplateVertices;
	const std::vector<double>& m_templateVertices;
	const std::vector<std::pair<int,int>>& m_templateEdges;	
	const std::vector<double>& m_targetVertices;
	const std::vector<bool>& m_validTargetVertices;

	std::vector<double> m_trafoTemplateVertices;

	const double m_nearestNeighborWeight;
	const double m_regularizationWeight;
	const double m_rigidWeight;
};

#endif