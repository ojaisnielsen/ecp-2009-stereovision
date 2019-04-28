#include "mathTools.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Interpolation_traits_2.h>
#include <CGAL/natural_neighbor_coordinates_2.h>
#include <CGAL/interpolation_functions.h>


CImg<float> crossProdMat(CImg<float> v)
{
	CImg<float> M(3, 3, 1, 1, 0);
	M(1, 0) = -v(0, 2);
	M(2, 0) = v(0, 1);
	M(0, 1) = -v(0, 2);
	M(2, 1) = v(0, 0);
	M(0, 2) = -v(0, 1);
	M(1, 2) = v(0, 0);

	return M;
}

CImg<float> crossProdVect(CImg<float> M)
{
	CImg<float> v(1, 3);
	v(0, 0) = M(1, 2);
	v(0, 1) = M(2, 0);
	v(0, 2) = M(0, 1);

	return v;
}


float crossCorrel(CImg<float> &vect0, CImg<float> &vect1)
{
	CImg<float> newVect0(vect0);
	CImg<float> newVect1(vect1);
	newVect0 -= vect0.mean();
	newVect0 /= newVect0.norm();
	newVect1 -= vect1.mean();
	newVect1 /= newVect1.norm();
	//return newVect0.dot(newVect1);
	//float correl = regmax(newVect0.dot(newVect1), 0);
	float correl = (1 + newVect0.dot(newVect1)) / 2;
	return correl;
}


CImg<float> fundamentalMat(CImgList<float> &pairsList)
{
	int n = pairsList.size;
	CImg<float> matP0 = pairsList[0];
	CImg<float> matP1 = pairsList[1];
	for (int i = 2; i < n; i += 2)
	{
		matP0.append(pairsList[i], 'x');
		matP1.append(pairsList[i + 1], 'x');
	}

	CImg<float> T0 = normalize(matP0);
	CImg<float> T1 = normalize(matP1);

	CImg<float> F = eightPoints(matP0, matP1);
	F = T1.get_transpose() * F * T0;

	CImg<float> U, S, V;
	F.SVD(U, S, V);

	S.identity_matrix();
	S(2, 2) = 0;

	F = U * S * (V.get_transpose());
	
	return F;

}

CImg<float> essentialMat(CImg<float> &K0, CImg<float> &K1, CImgList<float> &pairsList)
{

	int n = pairsList.size;
	CImg<float> matP0 = pairsList[0];
	CImg<float> matP1 = pairsList[1];
	for (int i = 2; i < n; i += 2)
	{
		matP0.append(pairsList[i], 'x');
		matP1.append(pairsList[i + 1], 'x');
	}

	matP0 = K0.get_invert() * matP0;
	matP1 = K1.get_invert() * matP1;

	CImg<float> T0 = normalize(matP0);
	CImg<float> T1 = normalize(matP1);

	CImg<float> E = eightPoints(matP0, matP1);
	E = T1.get_transpose() * E * T0;

	CImg<float> U, S, V;
	E.SVD(U, S, V);

	S.identity_matrix();
	S(2, 2) = 0;

	E = U * S * (V.get_transpose());
	
	return E;

}

CImg<float> normalize(CImg<float> &P)
{
	int n = P.dimx();

	//CImg<float> W = P.get_line(2);
	//W.append(P.get_line(2), 'y');
	//W.append(P.get_line(2), 'y');
	//CImg<float> Q = P.get_div(W);
	CImg<float> Q = toNotHomog(P);

	CImg<float> c(1, 2, 1, 1);
	c(0, 0) = Q.get_line(0).mean();
	c(0, 1) = Q.get_line(1).mean();

	float d = 0;
	for (int i = 0; i < n; i++)
	{
		d += (Q.get_column(i) - c).norm(2);
	}
	float s = n * sqrt(2.) / d;

	CImg<float> T(3, 3);
	T.identity_matrix();
	T(2, 0) = -c(0, 0);
	T(2, 1) = -c(0, 1);

	CImg<float> S(3, 3);
	S.identity_matrix();
	S *= s;
	S(2, 2) = 1;

	T = S * T;
	P = T * P;

	return T;
}

CImg<float> eightPoints(CImg<float> &P0, CImg<float> &P1)
{
	int n = P0.dimx();

	CImg<float> M = (P1.get_column(0) * (P0.get_column(0).get_transpose())).get_vector().get_transpose();
	for (int i = 1; i < n; i++)
	{
		M.append((P1.get_column(i) * (P0.get_column(i).get_transpose())).get_vector().get_transpose(), 'y');
	}

	M = M.get_transpose() * M;

	CImg<float> eigVals, eigVecs;
	M.symmetric_eigen(eigVals, eigVecs);

	eigVecs.column(8);
	eigVecs.matrix();

	return eigVecs;
}

CImg<float> alternateCameraMat(CImg<float> &F)
{
	CImg<float> U, S, V;
	F.SVD(U, S, V);

	CImg<float> K1t1 = U.get_column(2);
	CImg<float> K1R1 = -crossProdMat(K1t1) * F;
	CImg<float> R1 = K1R1.get_append(K1t1, 'x');
	return R1;
}

// alternateCameraMat renvoie une liste de matrices contenant R1 et t1, rotation et translation séparant les deux cameras
// elle prend en argument :
// - E : matrice essentielle
// - K0 et K1, matrices de calibration
// - testPts une liste de vecteurs telle que deux vecteur successifs forment une paire de points 2D (en coordonnées homogènes) correspondant aux deux images
// - width0, height0, width1, height1 sont les dimensions des images

CImgList<float> alternateCameraMat(CImg<float> &E, CImg<float> &K0, CImg<float> &K1, CImgList<float> &testPts, int width0, int height0, int width1, int height1)
{


	//CImgList<float> normTestPts; // normTestsPts : on passe les points en coordonnées normalisées
	//for (int i = 0; i < testPts.size; i += 2)
	//{
	//	normTestPts << K0.get_invert() * testPts[i] << K1.get_invert() * testPts[i + 1];
	//}

	CImg<float> U, S, V; // décomposition SVD de E
	E.SVD(U, S, V);
	CImg<float> W(3, 3, 1, 1, 0); // W = 0 -1  0
	W(1, 0) = -1;                 //     1  0  0
	W(0, 1) = 1;                  //     0  0  1
	W(2, 2) = 1;



	CImgList<float> R1; // R1 : liste des deux rotations possibles : U * W * Vt et U * Wt * Vt
	R1 << U * W * (V.get_transpose());
	R1 << U * (W.get_transpose()) * (V.get_transpose());

	CImgList<float> t1; // t1 : liste des deux translations possibles : troisième colonne de U ou -1 * ce vecteur
	t1 << U.get_column(2);
	t1 << - U.get_column(2);

	CImg<float> O(1, 3, 1, 1, 0);
	CImg<float> R0(3, 3); // P0 : matrice de projection calibrée de la première caméra : 1 0 0 0
	R0.identity_matrix(); //                                                             0 1 0 0



	int iR[4] = {0, 0, 1, 1};
	int it[4] = {0, 1, 0, 1};
	int maxPosDepths = 0; 
	int winner = 0;
	for (int i = 0; i < 4; i++)
	{
		CImgList<float> matrices = rectifMats(R1[iR[i]], t1[it[i]], K0, K1, width0, height0, width1, height1);
		CImg<float> T0 = matrices[0];
		CImg<float> T1 = matrices[1];
		CImg<float> n_K0 = matrices[2];
		CImg<float> n_K1 = matrices[3];
		CImg<float> R = matrices[4];
		CImg<float> n_t1 = matrices[5];

		float posDepths = 0;
		for (int j = 0; j < testPts.size; j += 2)
		{
			CImg<float> pL = T0 * testPts[i];
			CImg<float> pR = T1 * testPts[i + 1];
			CImg<float> Kl = n_K0;
			CImg<float> Kr = n_K1;
			if (toNotHomog(pL)(0, 0) < toNotHomog(pR)(0, 0))
			{
				CImg<float> temp = pL;
				pL = pR;
				pR = temp;
				temp = Kl;
				Kl = Kr;
				Kr = temp;
			}
			CImg<float> X0 = R.get_invert() * triangulate(pR, pL, Kr, Kl, 1, n_t1.norm(2)); // reconstruction 3D des points tests dans le repére de la première camera 
			CImg<float> X1 = R1[iR[i]] * (X0 - t1[it[i]]);
			if (X0(0, 2) > 0 && X1(0, 2) > 0)
			{
				posDepths++;
			}
		}
		saveMatrix(CImg<float>(1, 1, 1, 1, posDepths));
		if (posDepths > maxPosDepths) 
		{
			maxPosDepths = posDepths;
			winner = i; 
		}
	}
	saveMatrix(CImg<float>(1, 1, 1, 1, winner));

	winner = 1;
	CImgList<float> output;
	output << R1[iR[winner]] << t1[it[winner]];

	return output;

}



// rectifMats renvoie une liste de matrices contenant : T0, T1, n_K0, n_K1, R, n_t1 avec
// - T0 : matrice de rectification de l'image 0
// - T1 : matrice de rectification de l'image 1
// - n_k0 : matrice de calibration de l'image 0 rectifiée 
// - n_k1 : matrice de calibration de l'image 1 rectifiée 
// - R : matrice de rotation commune aux matrices de projection des images rectifiées
// - n_t1 : vecteur translation de la matrice de projection de l'image 1 rectifiée
//
// les arguments sont : 
// - R1 : matrice de rotation de la matrice de projection de l'image 1
// - t1 : vecteur translation de la matrice de projection de l'image 1
// - K0 : matrice de calibration de l'image 0  
// - K1 : matrice de calibration de l'image 1  

CImgList<float> rectifMats(CImg<float> &R1, CImg<float> &t1, CImg<float> &K0, CImg<float> &K1, int width0, int height0, int width1, int height1)
{

	CImg<float> c1 =  -R1.get_invert() * t1;   // c1 : centre de la camera 1 dans le repère de la camera 0

	CImg<float> xAxis = c1;        // xAxis : nouvel axe des x défini par c0->c1
	CImg<float> yAxis = CImg<float>(1, 3, 1, 1, "0, 0, 1", 0).get_cross(xAxis);  // yAxis : nouvel axe y produit vectoriel de l'ancien axe z et du nouvel axe x
	CImg<float> zAxis = xAxis.get_cross(yAxis); // zAxis : nouvel axe z produit vectoriel du nouvel axe x et du nouvel axe y


	CImg<float> K = (K0 + K1) / 2; //  K : nouvelle matrice de calibration commune (temporaire) définie arbitrairement

	CImg<float> R = xAxis.get_transpose() / xAxis.norm(2);  // R : matrice de rotation commune dont les lignes sont les vecteurs de la nouvelle base
	R.append(yAxis.get_transpose() / yAxis.norm(2), 'y');
	R.append(zAxis.get_transpose() / zAxis.norm(2), 'y');

	CImg<float> n_t1 = - R * c1; // n_t1 : tanslation séparant les nouvelles caméras 

	CImg<float> T0 = K * R * (K0.get_invert()); // Première matrice de rectification pour l'image 0
	CImg<float> T1 = K * R * ((K1 * R1).get_invert()); // Première matrice de rectification pour l'image 1

	//CImg<float> imC0 = K0.get_column(2); // imC0 : coordonnées du point principal de l'image 0
	//CImg<float> imC1 = K1.get_column(2); // imC1 : coordonnées du point principal de l'image 1
	CImg<float> imC0(1, 3, 1, 1, 1);
	imC0(0, 0) = width0 / 2.;
	imC0(0, 1) = height0 / 2.;
	CImg<float> imC1(1, 3, 1, 1, 1);
	imC1(0, 0) = width1 / 2.;
	imC1(0, 1) = height1 / 2.;

	CImg<float> n_imC0 = toNotHomog(T0 * imC0); // imC0 : coordonnées du point principal de l'image rectifiée 0
	CImg<float> n_imC1 = toNotHomog(T1 * imC1); // imC1 : coordonnées du point principal de l'image rectifiée 1


	float dx0 = n_imC0(0, 0) - imC0(0, 0); // dx0 : déplacement horizontal du point principal de l'image 0 lors de la rectification
	float dx1 = n_imC1(0, 0) - imC1(0, 0); // dx1 : déplacement horizontal du point principal de l'image 1 lors de la rectification
	float dy = n_imC0(0, 1) - imC0(0, 1); // dy : déplacement vertical commun du point principal lors de la rectification

	CImg<float> n_K0(K); // nK0 : nouvelle matrice de calibration de l'image 0 : matrice arbitraire choisie précédemment à laquelle on corrige la position du point principal
	CImg<float> n_K1(K); // nK1 : nouvelle matrice de calibration de l'image 1 : matrice arbitraire choisie précédemment à laquelle on corrige la position du point principal
	n_K0(2, 0) -= dx0;
	n_K0(2, 1) -= dy;
	n_K1(2, 0) -= dx1;
	n_K1(2, 1) -= dy;


	CImg<int> region0 = transRegion(T0, 0, 0, width0, height0); // region0 : image du bord de l'image0 par la rectification
	CImg<int> region1 = transRegion(T1, 0, 0, width1, height1); // region1 : image du bord de l'image1 par la rectification

	float newSize = (float) regmax(regmax(width0, height0), regmax(width1, height1));
	float s = newSize / regmax(regmax(region0(2) - region0(0), region0(3) - region0(1)), regmax(region1(2) - region1(0), region1(3) - region1(1))); 
	CImg<float> S(3, 3);
	S.identity_matrix(); // S : matrice diagonale(s, s, 1) ou s est le rapport entre la nouvelle taille souhaitée de l'image celle obtenue a priori par rectifiaction
	S *= s;
	S(2, 2) = 1;

	n_K0 = S * n_K0;
	n_K1 = S * n_K1;

	T0 = n_K0 * R * (K0.get_invert());
	T1 = n_K1 * R * ((K1 * R1).get_invert());

	CImgList<float> output;
	output << T0 << T1 << n_K0 << n_K1 << R << n_t1;

	return output;

}

CImg<int> mutualRegion(int width0, int height0, int width1, int height1, CImg<float> &T0, CImg<float> &T1)
{
	CImg<int> region0 = transRegion(T0, 0, 0, width0 - 1, height0 - 1);
	CImg<int> region1 = transRegion(T1, 0, 0, width1 - 1, height1 - 1);

	CImg<int> n_region(4);
	n_region(0) = regmin(region0(0), region1(0)); 
	n_region(1) = regmin(region0(1), region1(1)); 
	n_region(2) = regmax(region0(2), region1(2)); 
	n_region(3) = regmax(region0(3), region1(3)); 

	return n_region;
}


CImg<int> transRegion(CImg<float> &T, int x0, int y0, int x1, int y1)
{
	CImg<float> P(4, 3, 1, 1, 1);
	P(0, 0) = (float)x0;
	P(0, 1) = (float)y0;
	P(1, 0) = (float)x1;
	P(1, 1) = (float)y0;
	P(2, 0) = (float)x1;
	P(2, 1) = (float)y1;
	P(3, 0) = (float)x0;
	P(3, 1) = (float)y1;

	P = toNotHomog(T * P);

	CImg<int> output(4);
	output(0) = (int)floor(P.get_line(0).min());
	output(1) = (int)floor(P.get_line(1).min());
	output(2) = (int)ceil(P.get_line(0).max());
	output(3) = (int)ceil(P.get_line(1).max());

	return output;
}

CImgList<float> transFilledRegion(int x0, int y0, int x1, int y1, CImg<float> &T, int Ox, int Oy)
{
	CImg<float> P(4, 3, 1, 1, 1);
	P(0, 0) = (float)x0;
	P(0, 1) = (float)y0;
	P(1, 0) = (float)x0;
	P(1, 1) = (float)y1;
	P(2, 0) = (float)x1;
	P(2, 1) = (float)y1;
	P(3, 0) = (float)x1;
	P(3, 1) = (float)y0;

	CImg<float> O(1, 3, 1, 1, 0);
	O(0, 0) = Ox;
	O(0, 1) = Oy;
	P = (T * P);
	P = toHomog(toNotHomog(P));
	CImgList<float> output;
	for (int i = 0; i < 4; i++)
	{
		output << (P.get_column(i) - O);
	}

	return output;
}


void transImage(CImg<float> &T, CImg<float> *input, CImg<float> *output, CImg<int> &region, int *nOper, int *status)
{

	unsigned int newWidth = region(2) - region(0) + 1;
	unsigned int newHeight = region(3) - region(1) + 1;	

	if (nOper != NULL)
	{
		(*nOper) += newWidth * newHeight;
	}

	CImg<float> P(1, 3, 1, 1, 1);
	CImg<float> Ti = T.get_invert();
	for (int x = 0; x < newWidth; x++)
	{
		for (int y = 0; y < newHeight; y++)
		{
			P(0, 0) = x + region(0);
			P(0, 1) = y + region(1);
			CImg<float> Q = toNotHomog(Ti * P);
			(*output)(x, y, 0, 0) = input->linear_atXYZV(Q(0, 0), Q(0, 1), 0, 0, 0);
			(*output)(x, y, 0, 1) = input->linear_atXYZV(Q(0, 0), Q(0, 1), 0, 1, 0);
			(*output)(x, y, 0, 2) = input->linear_atXYZV(Q(0, 0), Q(0, 1), 0, 2, 0);
			if (status != NULL)
			{
				(*status)++;
			}
		}
	}
}

CImg<float> triangulate(CImg<float> &pR, CImg<float> &pL, CImg<float> &Kr, CImg<float> &Kl, float f, float b)//float disparity, CImg<float> &Kl, CImg<float> &Kr, CImg<float> &t)
{
	CImg<float> Cr = Kr;
	Cr(0, 0) /= f;
	Cr(1, 1) /= f;
	CImg<float> Cl = Kl;
	Cl(0, 0) /= f;
	Cl(1, 1) /= f;
	CImg<float> nhPr = toNotHomog(Cr.get_invert() * pR);
	CImg<float> nhPl = toNotHomog(Cl.get_invert() * pL);

	float d = nhPl(0, 0) - nhPr(0, 0) + 2;

	CImg<float> X(1, 3);
	X(0, 0) = nhPl(0, 0) * b / d;
	X(0, 1) = nhPl(0, 1) * b / d;
	X(0, 2) = f * b / d;
	return X;
}

void triangulate(CImg<float> &dispMap, CImg<float> &output, CImg<float> &Kl, CImg<float> &Kr, CImg<float> &t, float f)
{
	output.assign(dispMap.dimx(), dispMap.dimy(), 1, 3);

	float xCl = Kl(0, 0);
	float xCr = Kr(0, 0);
	for (int x = 0; x < dispMap.dimx();  x++)
	{
		for (int y = 0; y < dispMap.dimy();  y++)
		{
			CImg<float> pR(1, 3, 1, 1, 1);
			pR(0, 0) = x;// + 128;
			pR(0, 1) = y;// + 126;
			CImg<float> pL = pR;
			pL(0, 0) += dispMap(x, y) + xCl - xCr;


			CImg<float> X = triangulate(pR, pL, Kr, Kl, f, t.norm(2));
			output(x, y, 0, 0) = X(0, 0);
			output(x, y, 0, 1) = X(0, 1);
			output(x, y, 0, 2) = X(0, 2);
		}
	}

}


bool isOutOfFrame(CImgList<float> &C, CImg<float> &P)
{
	CImgList<float> Cp;
	for (int i = 0; i < C.size; i++)
	{
		Cp << toNotHomog(C[i]);
	}
	CImg<float> Pp = toNotHomog(P);
	CImg<float> M = Pp - Cp[0];
	M.append(Cp[1] - Cp[0], 'x');
	int sign = M.det();
	bool isInside = true;
	for (int i = 1; i < 4; i++)
	{
		CImg<float> M = Pp - Cp[i];
		M.append(Cp[(i + 1) % 4] - Cp[i], 'x');
		isInside = isInside && (M.det() * sign > 0);
		sign = M.det();
	}
	return !isInside;
}

CImg<float> toHomog(CImg<float> &M)
{
	CImg<float> l(M.dimx(), 1, 1, 1, 1);
	return M.get_append(l, 'y');
}

CImg<float> toNotHomog(CImg<float> &M)
{
	CImg<float> l = M.get_line(M.dimy() - 1);
	CImg<float> N = l;
	for (int i = 1; i < M.dimy(); i++)
	{
		N.append(l, 'y');
	}
	return M.get_div(N).get_lines(0, M.dimy() - 2);
}


CImg<float> interpLinear2d(CImg<float> knownPoints)
{
	typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
	typedef CGAL::Delaunay_triangulation_2<K> Delaunay_triangulation;
	typedef CGAL::Interpolation_traits_2<K> Traits;
	typedef CGAL::Data_access<std::map<K::Point_2, K::FT, K::Less_xy_2>> Value_access;

	Delaunay_triangulation T;
	std::map<K::Point_2, K::FT, K::Less_xy_2> function_values;

	int W = knownPoints.dimx();
	int H = knownPoints.dimy();

	CImg<float> output(W, H);

	int topLeftX, topLeftY, topRightX, topRightY, bottomLeftX, bottomLeftY, bottomRightX, bottomRightY;
	bool init = true;
	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			if (knownPoints(x, y, 0, 0) != 0)
			{
				if (init)
				{
					topLeftX = x;
					topLeftY = y;
					topRightX = x;
					topRightY = y;
					bottomLeftX = x;
					bottomLeftY = y;
					bottomRightX = x;
					bottomRightY = y;
					init = false;
				}
				else if (x < topLeftX && y < topLeftY)
				{
					topLeftX = x;
					topLeftY = y;
				}
				else if (x > topRightX && y < topRightY)
				{
					topRightX = x;
					topRightY = y;
				}
				else if (x < bottomLeftX && y > bottomLeftY)
				{
					bottomLeftX = x;
					bottomLeftY = y;
				}
				else if (x > bottomRightX && y > bottomRightY)
				{
					bottomRightX = x;
					bottomRightY = y;
				}

				K::Point_2 p(x,y);
				T.insert(p);
				function_values.insert(std::make_pair(p, knownPoints(x, y, 0, 1)));
			}
		}
	}

	K::Point_2 p0(0, 0);
	T.insert(p0);
	function_values.insert(std::make_pair(p0, knownPoints(topLeftX, topLeftY)));
	K::Point_2 p1(W - 1, 0);
	T.insert(p1);
	function_values.insert(std::make_pair(p1, knownPoints(topRightX, topRightY)));
	K::Point_2 p2(W - 1, H - 1);
	T.insert(p2);
	function_values.insert(std::make_pair(p2, knownPoints(bottomRightX, bottomRightY)));
	K::Point_2 p3(0, H - 1);
	T.insert(p3);
	function_values.insert(std::make_pair(p3, knownPoints(bottomLeftX, bottomLeftY)));

	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			K::Point_2 p(x,y);
			std::vector< std::pair< K::Point_2, K::FT > > coords;
			K::FT norm = CGAL::natural_neighbor_coordinates_2(T, p, std::back_inserter(coords)).second;
			K::FT res =  CGAL::linear_interpolation(coords.begin(), coords.end(), norm, Value_access(function_values));
			output(x, y) = res;
		}
	}
	return output;
}