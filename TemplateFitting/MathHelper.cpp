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

#include "MathHelper.h"

#include <vector>
#include <map>
#include <iostream>
#include <sstream>

const double math_eps = 1.0e-8;

namespace clapack
{
	extern "C"
	{
		#include "blaswrap.h"
		#include "f2c.h"
		extern int dgemm_(char *transa, char *transb, integer *m, integer *n
								, integer *k, doublereal *alpha, doublereal *a, integer *lda
								, doublereal *b, integer *ldb, doublereal *beta, doublereal *c
								, integer *ldc);
		extern int dgels_(char *trans, integer *m, integer *n, integer *nrhs, doublereal *a
								, integer *lda, doublereal *b, integer *ldb, doublereal *work
								, integer *lwork, integer *info);
		extern int dgesdd_(char *jobz, integer *m, integer *n, doublereal *a
								, integer *lda, doublereal *s, doublereal *u, integer *ldu
								, doublereal *vt, integer *ldvt, doublereal *work, integer *lwork
								, integer *iwork, integer *info);
		extern int dgetrf_(integer *m, integer *n, doublereal *a, integer *lda
								, integer *ipiv, integer *info);
		extern int dgetri_(integer *n, doublereal *a, integer *lda, integer *ipiv
								, doublereal *work, integer *lwork, integer *info);	
	}
}

void MathHelper::computeVertexNormals(const DataContainer& poly, std::vector<double>& vertexNormals)
{
	const std::vector<double>& vertexList = poly.getVertexList();
	const std::vector<std::vector<int>>& vertexIndexList = poly.getVertexIndexList();

	std::map<int, std::vector<size_t>> polygonsForVertexIndexMap;
	{
		for(size_t i = 0; i < vertexIndexList.size(); ++i)
		{
			const std::vector<int>& currPolygonIndices = vertexIndexList[i];
			for(size_t j = 0; j < currPolygonIndices.size(); ++j)
			{
				const int currIndex = currPolygonIndices[j];

				std::map<int, std::vector<size_t>>::iterator mapIter = polygonsForVertexIndexMap.find(currIndex);
				if(mapIter!=polygonsForVertexIndexMap.end())
				{
					std::vector<size_t>& vec = mapIter->second;
					vec.push_back(i);
				}
				else
				{
					std::vector<size_t> vec;
					vec.push_back(i);
					polygonsForVertexIndexMap.insert(std::make_pair(currIndex, vec));
				}
			}
		}
	}

	const size_t vertexNormalDim = vertexList.size();
	vertexNormals.resize(vertexNormalDim, 0.0);
	
	std::map<int, std::vector<size_t>>::const_iterator currPointPolyIter = polygonsForVertexIndexMap.begin();
	std::map<int, std::vector<size_t>>::const_iterator endPointPolyIter = polygonsForVertexIndexMap.end();
	for(; currPointPolyIter != endPointPolyIter; ++currPointPolyIter)
	{
		const int currIndex = currPointPolyIter->first;
		const std::vector<size_t>& neighborPolygons = currPointPolyIter->second;

		const Vec3d currVertex(vertexList[3*currIndex], vertexList[3*currIndex+1], vertexList[3*currIndex+2]);
		Vec3d vertexNormal(0.0, 0.0, 0.0);

		for(size_t i = 0; i < neighborPolygons.size(); ++i)
		{
			const std::vector<int>& currPoly = vertexIndexList[neighborPolygons[i]];

			const size_t currPolySize = currPoly.size();

			int tmpPos(-1);
			for(size_t j = 0; j < currPolySize; ++j)
			{
				tmpPos = currPoly[j] == currIndex ? static_cast<int>(j) : tmpPos;
			}

			if(tmpPos == -1 || currPoly[tmpPos] != currIndex)
			{
#ifdef DEBUG_OUTPUT
				std::cout << "Wrong neighboring index while computing normal vector" << std::endl;
#endif
				continue;
			}

			const int prevIndex = currPoly[(tmpPos+(currPolySize-1))%currPolySize];
			const int nextIndex = currPoly[(tmpPos+1)%currPolySize];

			const Vec3d prevVertex(vertexList[3*prevIndex], vertexList[3*prevIndex+1], vertexList[3*prevIndex+2]);
			const Vec3d nextVertex(vertexList[3*nextIndex], vertexList[3*nextIndex+1], vertexList[3*nextIndex+2]);

			const Vec3d v1 = nextVertex-currVertex;
			const Vec3d v2 = prevVertex-currVertex;

			const double v1SqrLength = v1.sqrLength();
			const double v2SqrtLength = v2.sqrLength();

			if(v1SqrLength < DBL_EPSILON || v2SqrtLength < DBL_EPSILON)
			{
#ifdef DEBUG_OUTPUT
				std::cout << "Zero length edge while computing normal vector" << std::endl;
#endif
				continue;
			}

			const double factor = 1.0/(v1SqrLength*v2SqrtLength);

			Vec3d v1xv2;
			v1.crossProduct(v2, v1xv2);
			vertexNormal += v1xv2*factor;
		}
		
		if(!vertexNormal.normalize())
		{
#ifdef DEBUG_OUTPUT
			std::cout << "Zero length normal vector while computing normal vector" << std::endl;
#endif
			continue;
		}
		
		vertexNormals[3*currIndex] = vertexNormal[0];
		vertexNormals[3*currIndex+1] = vertexNormal[1];
		vertexNormals[3*currIndex+2] = vertexNormal[2];
	}
}

void MathHelper::getPlaneProjection(const Vec3d& p1, const Vec3d& p2, const Vec3d& n2, Vec3d& outPoint)
{
	//given: direction v, normal n
	//searched: projected direction w
	// w = v - <n,v>n

	Vec3d diff = p2-p1;
	double d = diff.dotProduct(n2);

	outPoint = n2;
	outPoint.scalarMult(d);

	outPoint = p1 + outPoint;
}

void MathHelper::transformMesh(const double s, const std::vector<double>& R, const std::string& sstrRotOp, const std::vector<double>& t, const std::string& sstrTransOp, DataContainer& mesh)
{
	std::vector<double> vertexList = mesh.getVertexList();
	MathHelper::transformData(s, R, sstrRotOp, t, sstrTransOp, vertexList);

	mesh.setVertexList(vertexList);
}

void MathHelper::transformData(const double s, const std::vector<double>& R, const std::string& sstrRotOp, const std::vector<double>& t, const std::string& sstrTransOp, std::vector<double>& data)
{
	MathHelper::rotateData(R, sstrRotOp, data);
	MathHelper::scaleData(s, data);
	MathHelper::translateData(t, sstrTransOp, data);
}

bool MathHelper::computeAlignmentTrafo(const std::vector<double>& source, const std::vector<double>& target, double& s, std::vector<double>& R, std::vector<double>& t, bool bScaling)
{
	std::vector<double> tmpSourceData = source;
	std::vector<double> tmpTargetData = target;

	std::vector<double> sourceMean;
	MathHelper::centerData(tmpSourceData, sourceMean);

	std::vector<double> targetMean;
	MathHelper::centerData(tmpTargetData, targetMean);
	
	s = 1.0;

	if(bScaling)
	{
		if(!MathHelper::computeScaling(tmpSourceData, tmpTargetData, s))
		{
			return false;
		}

		MathHelper::scaleData(s, tmpSourceData);
	}

	if(!MathHelper::computeRotationAlignmentMatrix(tmpSourceData, tmpTargetData, R))
	{
		return false;
	}

	// t = -s*R*targetMean + sourceMean
	t.clear();
	t = sourceMean;

	MathHelper::transformData(-s, R, "N", targetMean, "+", t);

	return true;
}

bool MathHelper::computeScaling(const std::vector<double>& source, const std::vector<double>& target, double& s)
{
	if(source.size() != target.size() || source.size()%3 != 0)
	{
		s = 1.0;
		return false;
	}
		
	const size_t dataSize = source.size();
	const size_t numPoints = source.size()/3;

	double sourceDist(0.0);
	double targetDist(0.0);

	for(size_t i = 0; i < dataSize; ++i)
	{
		sourceDist += pow(source[i], 2);
		targetDist += pow(target[i], 2);
	}

	sourceDist = sqrt(sourceDist/static_cast<double>(numPoints));
	targetDist = sqrt(targetDist/static_cast<double>(numPoints));

	s = sourceDist > 0.0 ? targetDist / sourceDist : 1.0;
	return true;
}

bool MathHelper::computeRotationAlignmentMatrix(const std::vector<double>& source, const std::vector<double>& target, std::vector<double>& R)
{
	const int dim = 3;
	const size_t minSampleSize = min(source.size(), target.size());
	const int minNumPoints = static_cast<int>(minSampleSize)/dim;

	char trans = 'N';
	long int m = static_cast<long int>(minNumPoints);
	long int n = static_cast<long int>(dim);

	double* X = new double[n*n];

	{
		long int nrhs = n;

		double* A = new double[m*n];
		double* B = new double[m*nrhs];

		for(int i = 0; i < m; i++)
		{
			A[i] = source[dim*i];
			A[m+i] = source[dim*i+1];
			A[2*m+i] = source[dim*i+2];

			B[i] = target[dim*i];
			B[m+i] = target[dim*i+1];
			B[2*m+i] = target[dim*i+2];
		}

		long int lda = m;
		long int lbd = m;

		long int lwork = max(1, 2*m*n);
		double* work = new double[lwork];
	
		long int info = 0;

		//Estimate rotation matrix by A*X=B (points are rows of A and B)
		clapack::dgels_(&trans, &m, &n, &nrhs, A, &lda, B, &lbd, work, &lwork, &info);
		if(info != 0)
		{
#ifdef DEBUG_OUTPUT
			std::cout<<"Calculation of rotation matrix not successful "<< info << std::endl;
#endif		
			delete [] A;
			delete [] B;
			delete [] work;
			return false;
		}

	#pragma omp parallel for
		for(int i = 0; i < n; ++i)
		{
			for(int j = 0; j < n; ++j)
			{
				X[i*n+j] = B[i*m+j];
			}
		}

		delete [] A;
		delete [] B;
		delete [] work;
	}

	double* U = new double[n*n];
	double* VT = new double[n*n];
	
	{
		char jobz = 'S';

		double* S = new double[n];

		long int ldu = n;
		long int ldvt = n;

		long int lwork = 7*n*n+4*n;
		double* work = new double[lwork];

		long int * iwork = new long int[8*n];
		long int info = 0;

		//Singular value decomposition of estimated rotation matrix (X=U*S*VT)
		clapack::dgesdd_(&jobz, &n, &n, X, &n, S, U, &ldu, VT, &ldvt, work, &lwork, iwork, &info);
		if(info != 0)
		{
#ifdef DEBUG_OUTPUT
			std::cout<<"SVD decomposition of R not successful "<< info << std::endl;
#endif	

			delete [] U;
			delete [] VT;
			delete [] S;
			delete [] work;
			delete [] iwork;
			delete [] X;
			return false;
		}

		delete [] S;
		delete [] work;
		delete [] iwork;
	}

	{
		char transU = 'N';
		char transVT = 'N';

		long int ldu = n;
		long int ldvt = n;
		long int ldr = n;

		double alpha = 1.0;
		double beta = 0.0;
		
		//X=U*VT
		clapack::dgemm_(&transU, &transVT, &n, &n, &n, &alpha, U, &ldu, VT, &ldvt, &beta, X, &ldr);
	}

	//disallow reflections
   const double det = X[0]*X[4]*X[8]+X[3]*X[7]*X[2]+X[6]*X[1]*X[5]-X[2]*X[4]*X[6]-X[5]*X[7]*X[0]-X[8]*X[1]*X[3];
	if(det < 0)
	{
		std::cout<<"Determinant is "<< det << std::endl;
		delete [] X;
		delete [] U;
		delete [] VT;
		return false;
	}

	R.clear();
	R.resize(n*n);

	//Transpose tmpR matrix because we return rotation matrix which is multiplied from left to the data 
#pragma omp parallel for
	for(int i = 0; i < n; ++i)
	{
		for(int j = 0; j < n; ++j)
		{
			R[i*n+j] = X[j*n+i];
		}
	}

	delete [] X;
	delete [] U;
	delete [] VT;

	return true;
}

void MathHelper::centerData(std::vector<double>& data, std::vector<double>& mean)
{
	mean.clear();
	mean.push_back(0.0);
	mean.push_back(0.0);
	mean.push_back(0.0);

	const size_t numPoints = data.size()/3;

	for(size_t i = 0; i < numPoints; ++i)
	{
		for(size_t j = 0; j < 3; ++j)
		{
			mean[j] += data[i*3+j];
		}
	}

	const double invNumPoints(1.0/static_cast<double>(numPoints));
	for(size_t i = 0; i < 3; ++i)
	{
		mean[i] *= invNumPoints;
	}

	MathHelper::translateData(mean, "-", data);
}

void MathHelper::rotateData(const std::vector<double>& R, const std::string& sstrOp, std::vector<double>& data)
{
	if(R.size() != 9)
	{
		return;
	}

	const int numPoints = static_cast<int>(data.size()/3);

	char transA = sstrOp == "N" ? 'N' : 'T';
	char transB = 'N';

	long int m = static_cast<long int>(3);
	long int n = static_cast<long int>(numPoints);
	long int k = static_cast<long int>(3);	
	double alpha = 1.0;

	long int lda = m;
	long int ldb = k;
	long int ldc = m;

	const int mm = m*m;
	double* A = new double[mm];
#pragma omp parallel for	
	for(int i = 0; i < mm; ++i)
	{
		A[i] = R[i];
	}

	const int mn = m*n;
	double* B = new double[mn];
#pragma omp parallel for	
	for(int i = 0; i < mn; ++i)
	{
		B[i] = data[i];
	} 

	double* C = new double[mn];

	double beta = 0.0;

	clapack::dgemm_(&transA, &transB, &m, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc);

	data.clear();
	data.resize(mn);
#pragma omp parallel for	
	for(int i = 0; i < mn; ++i)
	{
		data[i] = C[i];
	} 
	
	delete [] A;
	delete [] B;
	delete [] C;
}

void MathHelper::translateData(const std::vector<double>& t, const std::string& sstrOp, std::vector<double>& data)
{
	if(t.size() != 3)
	{
		return;
	}

	const size_t numPoints = data.size() / 3;
	for(size_t i = 0; i < numPoints; ++i)
	{
		const size_t startIndex = 3*i;

		for(size_t j = 0; j < 3; ++j)
		{
			const double value = sstrOp == "+" ? data[startIndex+j]+t[j] : data[startIndex+j]-t[j];
			data[startIndex+j] = value;
		}
	}
}

void MathHelper::scaleData(const double factor, std::vector<double>& data)
{
	for(size_t i = 0; i < data.size(); ++i)
	{
		data[i] *= factor;
	}
}

void MathHelper::cleanMesh(DataContainer& mesh)
{
	std::set<size_t> validPointIndices;

	const std::vector<std::vector<int>>& vertexIndexList = mesh.getVertexIndexList();
	for(size_t i = 0; i < vertexIndexList.size(); ++i)
	{
		const std::vector<int>& currPolygonIndices = vertexIndexList[i];
		const size_t currPolygonSize = currPolygonIndices.size();

		size_t numDisjointVertices(0);
		for(size_t j = 0; j < currPolygonSize; ++j)
		{
			const size_t i1 = j;
			const size_t i2 = (j+1)%currPolygonSize;

			if(currPolygonIndices[i1] != currPolygonIndices[i2])
			{
				++numDisjointVertices;
			}
		}

		if(numDisjointVertices < 3)
		{
			continue;
		}

		for(size_t j = 0; j < currPolygonSize; ++j)
		{
			validPointIndices.insert(currPolygonIndices[j]);
		}
	}


	std::vector<double> meshVertices = mesh.getVertexList();
	std::vector<double> meshVertexColors = mesh.getVertexColorList();
	bool bHasVertexColors(meshVertices.size() == meshVertexColors.size());

	std::vector<std::pair<int, int>> oldNewMap;

	size_t newId = 0;
	for(size_t id = 0; id < mesh.getNumVertices(); ++id)
	{
		if(validPointIndices.find(id) == validPointIndices.end())
		{
			oldNewMap.push_back(std::make_pair(static_cast<int>(id), -1));
		}
		else
		{
			oldNewMap.push_back(std::make_pair(static_cast<int>(id), static_cast<int>(newId)));
			++newId;
		}
	}

	DataContainer cleanMesh = mesh;

	std::vector<double> cleanMeshVertices;
	std::vector<double> cleanMeshVertexColors;

	for(size_t oldId = 0; oldId < mesh.getNumVertices(); ++oldId)
	{
		const int newPointId = oldNewMap[oldId].second;
		if(newPointId == -1)
		{
			continue;
		}
		
		cleanMeshVertices.push_back(meshVertices[3*oldId]);
		cleanMeshVertices.push_back(meshVertices[3*oldId+1]);
		cleanMeshVertices.push_back(meshVertices[3*oldId+2]);

		if(bHasVertexColors)
		{
			cleanMeshVertexColors.push_back(meshVertexColors[3*oldId]);
			cleanMeshVertexColors.push_back(meshVertexColors[3*oldId+1]);
			cleanMeshVertexColors.push_back(meshVertexColors[3*oldId+2]);
		}
	}
	
	cleanMesh.setVertexList(cleanMeshVertices);
	cleanMesh.setVertexColorList(cleanMeshVertexColors);

	std::vector<std::vector<int>> cleanMeshPolygons;
	for(size_t i = 0; i < vertexIndexList.size(); ++i)
	{
		const std::vector<int>& currPolygonIndices = vertexIndexList[i];
		const size_t currPolygonSize = currPolygonIndices.size();

		size_t numDisjointVertices(0);
		for(size_t j = 0; j < currPolygonSize; ++j)
		{
			const size_t i1 = j;
			const size_t i2 = (j+1)%currPolygonSize;

			if(currPolygonIndices[i1] != currPolygonIndices[i2])
			{
				++numDisjointVertices;
			}
		}

		if(numDisjointVertices < 3)
		{
			continue;
		}

		std::vector<int> newPolygonIndices;
		for(size_t j = 0; j < currPolygonSize; ++j)
		{
			const int newId = oldNewMap[currPolygonIndices[j]].second;
			if(newId == -1)
			{
				std::cout << "Error" << std::endl;
				return;
			}

			newPolygonIndices.push_back(newId);
		}

		cleanMeshPolygons.push_back(newPolygonIndices);
	}

	cleanMesh.setVertexIndexList(cleanMeshPolygons);

	if(cleanMesh.getNumVertices() != mesh.getNumVertices())
	{
		std::cout << "Removed " << mesh.getNumVertices() - cleanMesh.getNumVertices() << " vertices" << std::endl;
		std::cout << "Removed " << mesh.getNumFaces() - cleanMesh.getNumFaces() << " faces" << std::endl;
	}

	mesh = cleanMesh;
}