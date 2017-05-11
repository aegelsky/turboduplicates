// Copyright (c) 2008-2011, Guoshen Yu <yu@cmap.polytechnique.fr>
// Copyright (c) 2008-2011, Jean-Michel Morel <morel@cmla.ens-cachan.fr>
//
// WARNING: 
// This file implements an algorithm possibly linked to the patent
//
// Jean-Michel Morel and Guoshen Yu, Method and device for the invariant 
// affine recognition recognition of shapes (WO/2009/150361), patent pending. 
//
// This file is made available for the exclusive aim of serving as
// scientific tool to verify of the soundness and
// completeness of the algorithm description. Compilation,
// execution and redistribution of this file may violate exclusive
// patents rights in certain countries.
// The situation being different for every country and changing
// over time, it is your responsibility to determine which patent
// rights restrictions apply to you before you compile, use,
// modify, or redistribute this file. A patent lawyer is qualified
// to make this determination.
// If and only if they don't conflict with any patent terms, you
// can benefit from the following license terms attached to this
// file.
//
// This program is provided for scientific and educational only:
// you can use and/or modify it for these purposes, but you are
// not allowed to redistribute this work or derivative works in
// source or executable form. A license must be obtained from the
// patent right holders for any other use.
//
// 
//*----------------------------- demo_ASIFT  --------------------------------*/
// Detect corresponding points in two images with the ASIFT method. 

// Please report bugs and/or send comments to Guoshen Yu yu@cmap.polytechnique.fr
// 
// Reference: J.M. Morel and G.Yu, ASIFT: A New Framework for Fully Affine Invariant Image 
//            Comparison, SIAM Journal on Imaging Sciences, vol. 2, issue 2, pp. 438-469, 2009. 
// Reference: ASIFT online demo (You can try ASIFT with your own images online.) 
//			  http://www.ipol.im/pub/algo/my_affine_sift/
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <sstream>
#include <sys/stat.h>

using namespace std;

#ifdef _OPENMP
#include <omp.h>
#endif

#include "demo_lib_sift.h"
#include "io_png/io_png.h"

#include "library.h"
#include "frot.h"
#include "fproj.h"
#include "compute_asift_keypoints.h"
#include "compute_asift_matches.h"

# define IM_X 480
# define IM_Y 360

void draw_matching_horizontal(const size_t w1, size_t h1, 
					const size_t w2, size_t h2, 
					const float zoom1, const float zoom2, 
					const std::vector<float>& ipixels1, const std::vector<float>& ipixels2,
					matchingslist& matchings,
					const char* filename);

void draw_matching_vertical(const size_t w1, size_t h1, 
					const size_t w2, size_t h2, 
					const float zoom1, const float zoom2, 
					const std::vector<float>& ipixels1, const std::vector<float>& ipixels2,
					matchingslist& matchings,
					const char* filename);

int main(int argc, char **argv)
{
    if ((argc != 4) && (argc != 5)) {
        std::cerr << " ******************************************************************************* " << std::endl
				  << " ***************************  ASIFT image matching  **************************** " << std::endl
				  << " ******************************************************************************* " << std::endl
				  << "Usage: " << argv[0] << " num_of_matches [Resize option: 0/1] draw_matches symmetric_calc" << std::endl
									     << "- num_of_matches: minimim value of match points for images to be match. " << std::endl
										  << "- [Resize optional 0/1]. 1: input images resize to 800x600 (default). 0: no resize. " << std::endl 
										  << "- draw_images [ 0/10/11/12 ]: output images (vertical/horizontal concatenated, " << std::endl
								      << "0: no draw (default). 10: draw images horizontal concatenated. " << std::endl
								      << "11: draw images vertical concatenated. 12: draw images vertical and horizontal concatenated. " << std::endl
					<< "- symmetric_calc [optional 0/1]: make symmetric compares, e.g. 1.png compares with 2.png and vice versa." << std::endl 
										<< "0: make symmetric (default), it's more efficiently." << std::endl 
										<< "1: don't make symmetric." << std::endl 
				<< " ******************************************************************************* " << std::endl
				<< " *********************  Jean-Michel Morel, Guoshen Yu, 2010 ******************** " << std::endl
				<< " ******************************************************************************* " << std::endl;
        return 1;
    }

	// ищем файлы в текущей директории
	char path[] = ".";
	std::vector<std::string> fileNames;
	get_dir_content(path, fileNames);

	int num_of_tilts1 = 7;
	int num_of_tilts2 = 7;
	int verb = 0;

	int num_keys = 0;

	// Define the SIFT parameters
	siftPar siftparameters;
	default_sift_parameters(siftparameters);

	// make folder for picture's points files
	const std::string pathPoints = "picsPoints/";
	const std::string pathComp = "picsComp/";
	struct stat st = {0};
	if (-1 == stat(pathPoints.c_str(), &st))
		mkdir(pathPoints.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	std::vector<std::string> filePoints;
        int nFilesCount = fileNames.size();
	for (int i = 0; i < nFilesCount; ++i) {

		float zoom = 1;
		std::string fileName = fileNames.at(i);
		{
			std::ostringstream strStream;
			strStream << "./" << pathPoints << fileName.substr(2) << ".txt";
			fileName = strStream.str();
		}
		filePoints.push_back(fileName);

		st = {0};
		// if such file already exists - do nothing
		if (0 == stat(fileName.c_str(), &st))
			continue;

		st = {0};
		std::size_t found = fileName.find_last_of("/");
		std::string subDir = fileName.substr(0, found);
		if (-1 == stat(subDir.c_str(), &st))
			mkdir(subDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);


		//////////////////////////////////////////////// Input
		// Read image
		float * iarr1;
		size_t w1, h1;
		if (NULL == (iarr1 = read_png_f32_gray(fileNames.at(i).c_str(), &w1, &h1))) {
				std::cerr << "Unable to load image file " << fileNames.at(i).c_str() << std::endl;
				continue;
		}
		std::vector<float> ipixels1(iarr1, iarr1 + w1 * h1);
		free(iarr1); /*memcheck*/

		///// Resize the images to area wS*hW in remaining the ap#EEEEEE#E5E5E5#E5E5E5sect-ratio	
		///// Resize if the resize flag is not set or if the flag is set unequal to 0
		float wS = IM_X;
		float hS = IM_Y;

		vector<float> ipixels1_zoom, ipixels2_zoom;

		// обсудить, скорее всего придется задать в 0
		int flag_resize = 0;
		if (argc > 2) {
			flag_resize = atoi(argv[2]);
		}

		int wS1 = 0, hS1 = 0;

		if (flag_resize != 0) {
			cout << "WARNING: The input images are resized to " << wS << "x" << hS << " for ASIFT. " << endl 
			<< "         But the results will be normalized to the original image size." << endl << endl;
		
			float InitSigma_aa = 1.6;
		
			float fproj_p, fproj_bg;
			char fproj_i;
			float *fproj_x4, *fproj_y4;
			int fproj_o;

			fproj_o = 3;
			fproj_p = 0;
			fproj_i = 0;
			fproj_bg = 0;
			fproj_x4 = 0;
			fproj_y4 = 0;
				
			float areaS = wS * hS;

			// Resize image 1
			float area1 = w1 * h1;
			zoom = sqrt(area1/areaS);

			wS1 = (int) (w1 / zoom);
			hS1 = (int) (h1 / zoom);

			int fproj_sx = wS1;
			int fproj_sy = hS1;     

			float fproj_x1 = 0;
			float fproj_y1 = 0;
			float fproj_x2 = wS1;
			float fproj_y2 = 0;
			float fproj_x3 = 0;	     
			float fproj_y3 = hS1;

			/* Anti-aliasing filtering along vertical direction */
			if ( zoom > 1 ) {
				float sigma_aa = InitSigma_aa * zoom / 2;
				GaussianBlur1D(ipixels1,w1,h1,sigma_aa,1);
				GaussianBlur1D(ipixels1,w1,h1,sigma_aa,0);
			}

			// simulate a tilt: subsample the image along the vertical axis by a factor of t.
			ipixels1_zoom.resize(wS1*hS1);
			fproj (ipixels1, 
							ipixels1_zoom, 
							w1, 
							h1, 
							&fproj_sx, 
							&fproj_sy, 
							&fproj_bg, 
							&fproj_o, 
							&fproj_p, 
							&fproj_i , 
							fproj_x1 , 
							fproj_y1 , 
							fproj_x2 , 
							fproj_y2 , 
							fproj_x3 , 
							fproj_y3, 
							fproj_x4, 
							fproj_y4); 
		}
		else 
		{
			ipixels1_zoom.resize(w1*h1);
			ipixels1_zoom = ipixels1;
			wS1 = w1;
			hS1 = h1;
			zoom = 1;
		}


		///// Compute ASIFT keypoints
		// number N of tilts to simulate t = 1, \sqrt{2}, (\sqrt{2})^2, ..., {\sqrt{2}}^(N-1)
		//	int num_of_tilts1 = 1;
		//	int num_of_tilts2 = 1;

		cout << "Computing keypoints on the image: " << fileNames.at(i).c_str() << " progress here %: " << trunc(i*100/nFilesCount) << endl;
		time_t tstart;
		tstart = time(0);

		// вычисление точек непосредственно на изображениях
		vector< vector< keypointslist > > keys;

		num_keys = compute_asift_keypoints(ipixels1_zoom, wS1, hS1, num_of_tilts1, verb, keys, siftparameters);

		// Write all the keypoints (row, col, scale, orientation, desciptor (128 integers)) to 
		// the file ./picsPoints/pict_name.txt (so that the users can match the keypoints with their own matching algorithm if they wish to)
		// keypoints in the image
		write_keypoints (fileName.c_str(), num_keys, keys, zoom);
	}

	time_t tend;
	tend = time(0);
	//	cout << "Keypoints computation accomplished in " << difftime(tend, tstart) << " seconds." << endl;

	int flag_draw_matches = 0;
	if (argc == 4) {
		flag_draw_matches = atoi(argv[3]);
	}

	int MIN_MATCHING = atoi(argv[1]);
	int SIMMETRIC_CALC = 1;
	if (argc == 5) {
		SIMMETRIC_CALC -= atoi(argv[4]);
	}
	
	int nTotalComps = nFilesCount*(nFilesCount-1)/2;
	//// Match ASIFT keypoints
	for (int i = 0; i < nFilesCount - SIMMETRIC_CALC; ++i) {

		st = {};
		std::string checkFileName = "./" + pathComp + fileNames.at(i).substr(2) + ".txt";
		// if for this image already compared with other images
		if (0 == stat(checkFileName.c_str(), &st)) continue;

		int num_matchings;

		float * iarr1;
		size_t wS1, hS1;
		if (NULL == (iarr1 = read_png_f32_gray(fileNames.at(i).c_str(), &wS1, &hS1))) {
			std::cerr << "Unable to load image file " << fileNames.at(i).c_str() << std::endl;
			continue;
		}
		std::vector<float> ipixels1(iarr1, iarr1 + wS1 * hS1);
		free(iarr1); /*memcheck*/

		vector< vector< keypointslist > > keys1;
		make_keys_all_vector(num_of_tilts1, keys1);
		float zoom_1 = 1;
		read_keypoints(filePoints.at(i).c_str(), keys1, zoom_1);

		vector< vector< keypointslist > > keys2;
		make_keys_all_vector(num_of_tilts2, keys2);
		
		std::size_t found = checkFileName.find_last_of("/");
		std::string fileAllMatches = checkFileName.substr(found + 1);

		bool needCaption = true;
		std::string strResults = "";
		const std::string fileResults = "results.txt";
		int nCurFilesMatched = (i+1)*(2*nFilesCount - 2 - i)/2;
		cout << "Matching the keypoints for: " << fileNames.at(i).c_str() << " progress %: " << (nCurFilesMatched*100/nTotalComps) << endl;

		std::string sMainFileName = fileNames.at(i);
		std::size_t foundMainFileSlash = sMainFileName.find_last_of("/");
		std::string sMainFileDir = sMainFileName.substr(0, foundMainFileSlash);

		int j = SIMMETRIC_CALC == 1 ? i + 1 : 0;
		for (; j < nFilesCount; ++j) {
			//	tstart = time(0);
			int nSubCurFilesMatched = nCurFilesMatched + j - i - 1;
			cout << "  matching for: " << fileNames.at(j).c_str() << " sub progress %: " << ((j-i)*100/(nFilesCount-1-i)) << " total progress %: " << (nSubCurFilesMatched*100/nTotalComps) << endl;

			std::string sSubFileName = fileNames.at(j);
			std::size_t foundSubFileSlash = sSubFileName.find_last_of("/");
			std::string sSubFileDir = sSubFileName.substr(0, foundSubFileSlash);
			if (sMainFileDir == sSubFileDir) {
				// cout << "  !!! matching for: " << fileNames.at(j).c_str() << " skipped because they are in the same dir!" << endl;
				continue;
			}


			if (i == j) continue;

			float * iarr2;
			size_t wS2, hS2;
			if (NULL == (iarr2 = read_png_f32_gray(fileNames.at(j).c_str(), &wS2, &hS2))) {
				std::cerr << "Unable to load image file " << fileNames.at(j).c_str() << std::endl;
				continue;
			}
			std::vector<float> ipixels2(iarr2, iarr2 + wS2 * hS2);
			free(iarr2); /*memcheck*/
			float zoom_2 = 1;

			read_keypoints(filePoints.at(j).c_str(), keys2, zoom_2);

			matchingslist matchings;

			num_matchings = compute_asift_matches(num_of_tilts1, 
																						num_of_tilts2, 
																						wS1, 
																						hS1, 
																						wS2, 
																						hS2, 
																						verb, 
																						keys1, 
																						keys2, 
																						matchings, 
																						siftparameters, 
																						MIN_MATCHING);

	///////////////// Output image containing line matches (the two images are concatenated one above the other)


	////// Write the coordinates of the matched points (row1, col1, row2, col2) to the file argv[5]
	// запись координат только, если кол-во совпадающих точек не меньше заданного

			//начало if на кол-во совпадающих точек
			if (num_matchings >= MIN_MATCHING) {
				cout << "  " << fileNames.at(i) << " -- " << fileNames.at(j) << endl << endl;
				if (needCaption) {
					strResults += fileNames.at(i) + ":\n";
					needCaption = false;
				}

				{
					std::ostringstream strStream;
					strStream << "\t" << fileNames.at(j) << " -- " 
										<< num_matchings << " matches\n"; 
					strResults += strStream.str();
				}

				std::ofstream file_matches(fileAllMatches.c_str());
				if (file_matches.is_open()) {
					file_matches << fileNames.at(j).c_str() << " -- " << num_matchings << " matches\n";
				}
				else {
					std::cerr << "Unable to open the file_matches for " << fileAllMatches.c_str();
				}
				file_matches.close();
			}

			std::string fileMatches, fileNameTemplate;
			{
				std::size_t slash = fileNames[i].find_last_of("/");
				std::string subDir = fileNames[i].substr(slash + 1);

				slash = fileNames[j].find_last_of("/");
				std::ostringstream strStream;
				strStream << "./" << pathComp << subDir << "/" << fileNames[j].substr(2, slash - 1);
				subDir = strStream.str();
				std::string command = "mkdir -p " + subDir;
				st = {0};
				// if such file already exists - do nothing
				if (-1 == stat(subDir.c_str(), &st))
					int dir_er = system(command.c_str());

				std::size_t foundPoint = fileNames[j].find_last_of(".");
				fileNameTemplate = subDir + fileNames[j].substr(slash + 1, foundPoint - slash - 1);
				fileMatches = fileNameTemplate + ".txt";
			}
			std::ofstream file(fileMatches.c_str());
			if (file.is_open())	{
				// Write the number of matchings in the first line
				file << num_matchings << "\n";

				matchingslist::iterator ptr = matchings.begin();
				for(int k = 0; k < (int) matchings.size(); ++k, ++ptr) {
					file << zoom_1*ptr->first.x << "  " << zoom_1*ptr->first.y << "  " 
						<<  zoom_2*ptr->second.x << "  " <<  zoom_2*ptr->second.y << "\n";
				}
			}
			else {
				std::cerr << "Unable to open the file matchings.";
			}

			// draw match lines
			std::string picsHorizont, picsVertical;
			if (flag_draw_matches == 10 || flag_draw_matches == 12 ) {
				picsHorizont = fileNameTemplate + "_0.png";
				draw_matching_horizontal(wS1, hS1, wS2, hS2, zoom_1, zoom_2, ipixels1, ipixels2,
										matchings, picsHorizont.c_str());
			}

			if (flag_draw_matches > 10 ) {
				picsVertical = fileNameTemplate + "_1.png";
				draw_matching_vertical(wS1, hS1, wS2, hS2, zoom_1, zoom_2, ipixels1, ipixels2,
										matchings, picsVertical.c_str());
			}
			file.close();
		}

		// make result file
		if ( strResults.size() != 0) {
			std::ofstream file_results;
			file_results.open(fileResults.c_str(), std::ofstream::out | std::ofstream::app);
			if (file_results.is_open()) {
				file_results << strResults.c_str() << "\n";
			}
			else {
				std::cerr << "Unable to open the file_results for " << fileResults.c_str();
			}
			file_results.close();
		}


		// if such file already exists
		if (0 == stat(fileAllMatches.c_str(), &st)) {
			std::size_t slash = fileNames.at(i).find_last_of("/");
			st = {0};
			std::string fileNewName = "./" + pathComp + fileNames.at(i).substr(2, slash - 1);
			if (-1 == stat(fileNewName.c_str(), &st)) {
				mkdir(fileNewName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			}

			if (0 != rename(fileAllMatches.c_str(), checkFileName.c_str())) {
				std::cerr << "Unable to rename the file all matchings for " << fileAllMatches.c_str() << endl;
			}
		}
	}

	// конец цикла
    return 0;
}

void draw_matching_horizontal(const size_t w1, size_t h1, 
					const size_t w2, size_t h2, 
					const float zoom1, const float zoom2, 
					const std::vector<float>& ipixels1, const std::vector<float>& ipixels2,
					matchingslist& matchings,
					const char* filename) {
	/////////// Output image containing line matches (the two images are concatenated one aside the other)
	int band_w = 20; // insert a black band of width band_w between the two images for better visibility
	int wo =  w1 + w2 + band_w;
	int ho = MAX(h1, h2);

	float *opixelsASIFT = new float[wo*ho];

	for (int j = 0; j < (int) ho; ++j)
		for(int i = 0; i < (int) wo; ++i)  
			opixelsASIFT[j * wo + i] = 255;

	/////////////////////////////////////////////////////////////////// Copy both images to output
	for (int j = 0; j < (int) h1; ++j)
		for(int i = 0; i < (int) w1; ++i)  
			opixelsASIFT[j*wo+i] = ipixels1[j*w1+i];

	for (int j = 0; j < (int) h2; ++j)
		for(int i = 0; i < (int) w2; ++i)  
			opixelsASIFT[w1 + band_w + j*wo + i] = ipixels2[j*w2 + i];

	//////////////////////////////////////////////////////////////////// Draw matches
	matchingslist::iterator ptr = matchings.begin();
	for (int i = 0; i < (int) matchings.size(); ++i, ++ptr)	{
		draw_line(opixelsASIFT, 
			(int) (zoom1*ptr->first.x), 
			(int) (zoom1*ptr->first.y), 
			(int) (zoom2*ptr->second.x) + w1 + band_w, 
			(int) (zoom2*ptr->second.y), 
			255.0f, wo, ho);
	}

	///////////////////////////////////////////////////////////////// Save imgOut
	write_png_f32(filename, opixelsASIFT, wo, ho, 1);

	delete[] opixelsASIFT; /*memcheck*/
}

void draw_matching_vertical(const size_t w1, const size_t h1, 
					const size_t w2, const size_t h2, 
					const float zoom1, const float zoom2, 
					const std::vector<float>& ipixels1, const std::vector<float>& ipixels2,
					matchingslist& matchings,
					const char* filename) {
	///////////////// Output image containing line matches (the two images are concatenated one above the other)
	int band_w = 20; // insert a black band of width band_w between the two images for better visibility

	int wo = MAX(w1, w2);
	int ho = h1 + h2 + band_w;

	float *opixelsASIFT = new float[wo*ho];

	for (int j = 0; j < (int) ho; ++j)
		for(int i = 0; i < (int) wo; ++i)  
			opixelsASIFT[j * wo + i] = 255;

	/////////////////////////////////////////////////////////////////// Copy both images to output
	for (int j = 0; j < (int) h1; ++j)
		for(int i = 0; i < (int) w1; ++i)  
			opixelsASIFT[j * wo + i] = ipixels1[j * w1 + i];

	for (int j = 0; j < (int) h2; ++j)
		for(int i = 0; i < (int) w2; ++i)  
			opixelsASIFT[(h1 + band_w + j)*wo + i] = ipixels2[j*w2 + i];

	//////////////////////////////////////////////////////////////////// Draw matches
	matchingslist::iterator ptr = matchings.begin();
	for (int i=0; i < (int) matchings.size(); ++i, ++ptr) {
		draw_line(opixelsASIFT, 
			(int) (zoom1*ptr->first.x), 
			(int) (zoom1*ptr->first.y), 
			(int) (zoom2*ptr->second.x), 
			(int) (zoom2*ptr->second.y) + h1 + band_w, 
			255.0f, wo, ho);
	}

	///////////////////////////////////////////////////////////////// Save imgOut	
	write_png_f32(filename, opixelsASIFT, wo, ho, 1);

	delete[] opixelsASIFT; /*memcheck*/
}
