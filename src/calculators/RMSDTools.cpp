
#include "RMSDTools.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
using namespace std;



/*
 * Centers all the conformations to origin and stores the movement vectors.
 *
 */
void RMSDTools::centerAllAtOrigin(
		unsigned int atomsPerConformation,
		unsigned int numberOfConformations,
		double * const all_coords,
		double* const translations){

	unsigned int coordsPerConformation = atomsPerConformation * 3;

	//#pragma omp parallel for
	for (unsigned int i = 0; i < numberOfConformations; ++i){
		double* center = &(translations[3*i]);
		double* coords = &(all_coords[coordsPerConformation*i]);
		RMSDTools::geometricCenter(atomsPerConformation, coords, center);
		RMSDTools::shift3D(atomsPerConformation, coords, center, -1.);
	}
}

void RMSDTools::applyTranslationsToAll(unsigned int atomsPerConformation, unsigned int numberOfConformations, double * const all_coords, double* const translations){
	unsigned int coordsPerConformation = atomsPerConformation * 3;

	//#pragma omp parallel for
	for (unsigned int i = 0; i < numberOfConformations; ++i){
		double* translation_vector = &(translations[3*i]);
		double* coords = &(all_coords[coordsPerConformation*i]);
		RMSDTools::shift3D(atomsPerConformation, coords, translation_vector, 1.);
	}
}

void RMSDTools::applyTranslationToAll(unsigned int atomsPerConformation, unsigned int numberOfConformations, double * const all_coords, double* const translation_vector){
	unsigned int coordsPerConformation = atomsPerConformation * 3;

	//#pragma omp parallel for
	for (unsigned int i = 0; i < numberOfConformations; ++i){
		double* coords = &(all_coords[coordsPerConformation*i]);
		RMSDTools::shift3D(atomsPerConformation, coords, translation_vector, 1.);
	}
}

void RMSDTools::geometricCenter(unsigned int n, const double * const x, double * const center){
	unsigned int i;

	// Initialize variables before the loop
	center[0] = 0.0;
	center[1] = 0.0;
	center[2] = 0.0;

	for(i=0; i<n; i++)
	{
		int offset = 3*i;
		center[0] += x[offset];
		center[1] += x[offset+1];
		center[2] += x[offset+2];
	}

	center[0] /= n;
	center[1] /= n;
	center[2] /= n;
}

void RMSDTools::shift3D(unsigned int numberOfPoints, double * const x, double trans[3], double scalar){
	double shiftVector[3];

	shiftVector[0] = trans[0]*scalar;
	shiftVector[1] = trans[1]*scalar;
	shiftVector[2] = trans[2]*scalar;

	for(unsigned int i=0; i<numberOfPoints; ++i){
		int offset = 3*i;
		x[offset] += shiftVector[0];
		x[offset+1] += shiftVector[1];
		x[offset+2] += shiftVector[2];
	}
}

void RMSDTools::rotate3D(unsigned int n, double * const x, double* rot_matrix){
	double u[3][3];
	u[0][0] = rot_matrix[0];
	u[0][1] = rot_matrix[1];
	u[0][2] = rot_matrix[2];
	u[1][0] = rot_matrix[3];
	u[1][1] = rot_matrix[4];
	u[1][2] = rot_matrix[5];
	u[2][0] = rot_matrix[6];
	u[2][1] = rot_matrix[7];
	u[2][2] = rot_matrix[8];
	rotate3D(n, x, u);
}
void RMSDTools::rotate3D(unsigned int number_of_atoms, double * const coords, double u[3][3]){
	// We go through all selected atoms
	for(unsigned int i=0; i<number_of_atoms; ++i){
		int offset = i*3;
		double x_tmp_0,x_tmp_1,x_tmp_2;
		x_tmp_0 = coords[offset];
		x_tmp_1 = coords[offset+1];
		x_tmp_2 = coords[offset+2];

		// An rotate each of them
		coords[offset] 	= u[0][0] * x_tmp_0 + u[0][1] * x_tmp_1 + u[0][2] * x_tmp_2;
		coords[offset+1] = u[1][0] * x_tmp_0 + u[1][1] * x_tmp_1 + u[1][2] * x_tmp_2;
		coords[offset+2] = u[2][0] * x_tmp_0 + u[2][1] * x_tmp_1 + u[2][2] * x_tmp_2;
	}
}

double RMSDTools::calcRMS(const double * const x, const double * const y, unsigned int num_atoms){
	double sum_res = 0.0;

	for(unsigned int i=0; i<num_atoms*3; ++i){
		sum_res += (x[i] - y[i]) * (x[i] - y[i]);
	}

	return sqrt(sum_res/num_atoms);
}

void RMSDTools::initializeTo(double* array, double value, int array_len){
	fill(array, array+array_len, value);
}

//array1 = array2
void RMSDTools::copyArrays(double* array1, double* array2, int array_len){
	copy(array2,array2+array_len,array1);
}

void RMSDTools::calculateMeanCoordinates(double* meanCoordinates, double* allCoordinates,
											int numberOfConformations, int atomsPerConformation){

	// Zero mean coordinates
	RMSDTools::initializeTo(meanCoordinates, 0.0, atomsPerConformation*3);

	// Do calculation
	for (int i  = 0; i <  numberOfConformations; ++i){
		int conformation_offset = i*atomsPerConformation*3;
		//#pragma omp parallel for shared(meanCoordinates)
		for (int j = 0; j < atomsPerConformation; ++j){
			int atom_offset = 3*j;
			int offset = conformation_offset + atom_offset;
			meanCoordinates[atom_offset] += allCoordinates[ offset ];
			meanCoordinates[atom_offset+1] += allCoordinates[ offset + 1];
			meanCoordinates[atom_offset+2] += allCoordinates[ offset + 2];
		}
	}

	// Divide by the number of conformations
	//#pragma omp parallel for shared(meanCoordinates)
	for (int i = 0; i < atomsPerConformation*3; ++i){
		meanCoordinates[i] /= numberOfConformations;
	}
}

/*
 David J. Heisterberg
 The Ohio Supercomputer Center
 1224 Kinnear Rd.
 Columbus, OH  43212-1163
 (614)292-6036
 djh@ccl.net    djh@ohstpy.bitnet    ohstpy::djh

 Translated to C from fitest.f program and interfaced with Xmol program
 by Jan Labanowski,  jkl@ccl.net   jkl@ohstpy.bitnet   ohstpy::jkl

 Some minor changes and indentation by Víctor Gil Sepúlveda

 Copyright: Ohio Supercomputer Center, David J. Heisterberg, 1990.
 The program can be copied and distributed freely, provided that
 this copyright in not removed. You may acknowledge the use of the
 program in published material as:
 David J. Heisterberg, 1990, unpublished results.
*/
void RMSDTools::jacobi(double a[4][4], double d[4], double v[4][4], int nrot){
	double onorm, dnorm;
	double b, dma, q, t, c, s;
	double atemp, vtemp, dtemp;
	int i, j, k, l;

	for (j = 0; j <= 3; j++) {
		for (i = 0; i <= 3; i++) {
			v[i][j] = 0.0;
		}
		v[j][j] = 1.0;
		d[j] = a[j][j];
	}

	for (l = 1; l <= nrot; l++) {
		dnorm = 0.0;
		onorm = 0.0;
		for (j = 0; j <= 3; j++) {
			dnorm = dnorm + fabs(d[j]);
			for (i = 0; i <= j - 1; i++) {
				onorm = onorm + fabs(a[i][j]);
			}
		}

		if((onorm/dnorm) <= 1.0e-12){
			break;
		}

		for (j = 1; j <= 3; j++) {
			for (i = 0; i <= j - 1; i++) {
				b = a[i][j];
				if(fabs(b) > 0.0) {
					dma = d[j] - d[i];
					if((fabs(dma) + fabs(b)) <=  fabs(dma)) {
						t = b / dma;
					}
					else {
						q = 0.5 * dma / b;
						t = 1.0/(fabs(q) + sqrt(1.0+q*q));
						if(q < 0.0) {
							t = -t;
						}
					}
					c = 1.0/sqrt(t * t + 1.0);
					s = t * c;
					a[i][j] = 0.0;
					for (k = 0; k <= i-1; k++) {
						atemp = c * a[k][i] - s * a[k][j];
						a[k][j] = s * a[k][i] + c * a[k][j];
						a[k][i] = atemp;
					}
					for (k = i+1; k <= j-1; k++) {
						atemp = c * a[i][k] - s * a[k][j];
						a[k][j] = s * a[i][k] + c * a[k][j];
						a[i][k] = atemp;
					}
					for (k = j+1; k <= 3; k++) {
						atemp = c * a[i][k] - s * a[j][k];
						a[j][k] = s * a[i][k] + c * a[j][k];
						a[i][k] = atemp;
					}
					for (k = 0; k <= 3; k++) {
						vtemp = c * v[k][i] - s * v[k][j];
						v[k][j] = s * v[k][i] + c * v[k][j];
						v[k][i] = vtemp;
					}
					dtemp = c*c*d[i] + s*s*d[j] - 2.0*c*s*b;
					d[j] = s*s*d[i] + c*c*d[j] +  2.0*c*s*b;
					d[i] = dtemp;
				}  /* end if */
			} /* end for i */
		} /* end for j */
	} /* end for l */

	nrot = l;

	for (j = 0; j <= 2; j++) {
		k = j;
		dtemp = d[k];
		for (i = j+1; i <= 3; i++) {
			if(d[i] < dtemp) {
				k = i;
				dtemp = d[k];
			}
		}

		if(k > j) {
			d[k] = d[j];
			d[j] = dtemp;
			for (i = 0; i <= 3; i++) {
				dtemp = v[i][k];
				v[i][k] = v[i][j];
				v[i][j] = dtemp;
			}
		}
	}
}

void RMSDTools::transposeMatrix(const double (*const source_matrix)[3], double matrix[3][3]){
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			matrix[i][j] = source_matrix[j][i];
		}
	}
}

void RMSDTools::transposeMatrix(const double (*const source_matrix)[4], double matrix[3][3]){
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			matrix[i][j] = source_matrix[j][i];
		}
	}
}

//-------------------------------------------
// Code based on the work of Dr. Bosco K. Ho
//
// http://boscoh.com/code/rmsd.c
//
//-------------------------------------------
/*
 *    Diagonalize a 3x3 matrix & sort eigenval by magnitude
 */
bool RMSDTools::diagonalize_symmetric(
		double matrix[3][3],
        double eigen_vec[3][3],
        double eigenval[3]){

	double val;
	double vec[4][4];

	double tmp_matrix[4][4];
	RMSDTools::initializeTo(tmp_matrix[0],0,16);

	for(int i = 0; i <3; ++i){
		for(int j = 0; j< 3;++j){
			tmp_matrix[i][j] = matrix[i][j];
		}
	}

	RMSDTools::jacobi(tmp_matrix, eigenval, vec);

	//Sort solutions by eigenvalue
	int k;
	for (int i = 0; i < 4; ++i){
		k = i;
		val = eigenval[i];

		// Find a bigger eigenvalue
		for (int j = i+1; j < 4; j++){
			if (eigenval[j] > val)	{
				k = j;
				val = eigenval[k];
			}
		}

		// If there was a bigger eigenvalue...
		if (k != i){
			eigenval[k] = eigenval[i];
			eigenval[i] = val;
			// Swap columns i and j
			for (int j = 0; j < 4; ++j){
				swap(vec[j][i],vec[j][k]);
				/*val = vec[j][i];
				vec[j][i] = vec[j][k];
				vec[j][k] = val;*/
			}
		}
	}

	// Transpose such that first index refers to solution index
	/*for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			eigen_vec[i][j] = vec[j][i];
		}
	}*/
	RMSDTools::transposeMatrix(vec,eigen_vec);

	return (true);
}

void RMSDTools::normalize(double* a){
  double  b;

  b = sqrt((double)(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]));
  a[0] /= b;
  a[1] /= b;
  a[2] /= b;
}

double RMSDTools::dot(double* a, double* b){
  return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}

void RMSDTools::cross(double* a, double* b, double* c)	{
	  a[0] = b[1]*c[2] - b[2]*c[1];
	  a[1] = b[2]*c[0] - b[0]*c[2];
	  a[2] = b[0]*c[1] - b[1]*c[0];
}
