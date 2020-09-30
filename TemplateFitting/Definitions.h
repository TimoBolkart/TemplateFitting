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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

//Weight of the nearest neighbor energy
const double NN_WEIGHT = 1.0;

//Start weight of the regularization energy (reduced during iteration)
//A high weight encourages the afine transformation matrices of neighboring vertices to be similar
const double REG_WEIGHT = 1000.0;

//Start weight of the rigid energy (reduced during iteration)
//A high weight forces the affine transformation to be a rigid
const double RIGID_WEIGHT = 1000.0;

//Maximum number of iterations 
const size_t MAX_NUM_ITER = 10;

//Maximum valid distance of a template vertex to its nearest neighbor
const double MAX_NN_DIST = 15.0;

//Maximum valid angle between a template vertex and its nearest neighbor
const double MAX_ANGLE = 80.0;

//Enables per-iteration printouts of LBFGSB
//#define OUTPUT_TRACE

#endif