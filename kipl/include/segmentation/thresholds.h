#ifndef __IMAGETHRES_H
#define __IMAGETHRES_H

#include <vector>
#include <deque>
#include <iostream>
#include <sstream>

#include "../logging/logger.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#include "../base/timage.h"
#include "../morphology/morphology.h"
#include "../base/thistogram.h"

/// Namespace that supports threshold methods on images
namespace kipl { namespace segmentation {
	typedef enum {
		/// Value less than th $\rightarrow$ 1
		cmp_less=0,
		/// Value greater than th $\rightarrow$ 1
		cmp_greater,
		/// Value less than or equal to th $\rightarrow$ 1
		cmp_lesseq,
		/// Value greater than or equal to th $\rightarrow$ 1
		cmp_greatereq,
		/// Value not equal to th $\rightarrow$ 1
		cmp_noteq,
		/// Value equal to th $\rightarrow$ 1
		cmp_eq
	} CmpType;


	typedef enum {
		/// Don't exclude tails
		tail_none=0,
		/// Exclude both tails
		tail_both,
		/// Exclude the left tail
		tail_left,
		/// Exclude the right tail
		tail_right
	} TailType;


	/// \brief Limits the intensity dynamics to specified interval
	///	
	///	This operation can be used as a presegmentation step
	///
	///	\param data array to be modified
	/// \param N number of elements
	///	\param qlo lower limit
	///	\param qhi upper limit
	///	\param normalize switch to select normalization
	template <class T>
	int LimitDynamics(T * data, const size_t N, const T qlo, const T qhi, bool normalize=false)
	{
		if (normalize) {
			const float scale=1.0f/(float(qhi)-float(qlo));
			for (ptrdiff_t i=0; i<N; i++) {
				if (data[i]<qlo) 
					data[i]=T(0);
				else 
					if (qhi<data[i]) 
						data[i]=T(1);	
					else {
						data[i]=static_cast<T>((float(data[i])-float(qlo))*scale);
					}
			}
		}
		else {
			
			//#pragma omp parallel for default(none) shared(N,data,qlo,qhi)
				for (ptrdiff_t i=0; i<N; i++) {					
					if (data[i]<qlo) 
						data[i]=qlo;
					else 
						if (data[i]>qhi) data[i]=qhi;	
				}
		}
		
		return 0;
	}
	
	
	/// \brief Applies a threshold to the image
	///
	/// \param img Input image
	/// \param result Resulting image
	/// \param N number of elements to threshold
	/// \param th Threshold level
	/// \param cmp Type of comparison (less or greater), complementary image selector
	/// \param mask ROI mask, must have the same size as img or an xy slice of img
	/// \param Nmask number mask elements 0=default -> Nmask = N
	template <typename T0, typename T1, typename TM>
	int Threshold(T0 * data,
			T1 *result,
			size_t N,
			const T0 th,
			CmpType cmp=cmp_greater,
			TM * mask = reinterpret_cast<TM *>(NULL),
			const size_t Nmask=size_t(0))
	{
		T1 cmptrue,cmpfalse;
		if (cmp==cmp_greater) 
			cmptrue=1;
		else
			cmptrue=0;

		cmpfalse=!cmptrue;

		if (mask != NULL) {
			Nmask = Nmask==0 ? N : Nmask;
			for (size_t i=0, j=0; i<N; i++, j++) {
				if (j==Nmask) j=0;
				if (mask[j])
					result[i]=(data[i]>=th) ? cmptrue: cmpfalse;
				else
					result[i]=cmpfalse;
			}

		}
		else {
			for (size_t i=0; i<N; i++)
				result[i]=(data[i]>=th) ? cmptrue: cmpfalse;;
		}

		return 0;

	}

/*

	/// \brief Applies multiple thresholds to the image
	///
	/// \param img Input image
	/// \param th Array containing the threshold levels
	/// \param n Number of thresholds
	/// \param vals Assignment values for the intervals, if vals=NULL will automaticly 
	/// increasing numbers be used
	/// \param mask ROI mask
	template <class ImgType,int NDim>
		int MultiThreshold(CImage<ImgType,NDim> & img,CImage<char,NDim> & simg, vector<ImgType>  &th, vector<char> & vals=vector<char>(0), CImage<char,NDim> *mask=NULL)
	{
		simg(img.getDimsptr());
		ImgType *imgdata=img.getDataptr();
		char *tmpdata=simg.getDataptr(), *maskdata=NULL;

		int n=th.size();
		if ((mask) && (mask->N()==img.N())) {
			maskdata=mask->getDataptr();
			for (int i=0; i<img.N(); i++) 
				if (maskdata[i])
					for (char j=0; j<n; j++) {
						if (imgdata[i]<th[j]) {
							if (vals)
								tmpdata[i]=vals[j];
							else
								tmpdata[i]=j;
							break;
						}
						if (!vals.empty())
							tmpdata[i]=vals[n];
						else
							tmpdata[i]=n;
					}
				else
					tmpdata[i]=0;

		}
		else {
			for (int i=0; i<img.N(); i++) {
				for (char j=0; j<th.size(); j++) {
					if (imgdata[i]<th[j]) {
						if (!vals.empty())
							tmpdata[i]=vals[j];
						else
							tmpdata[i]=j;
						break;
					}
					if (!vals.empty())
						tmpdata[i]=vals[n];
					else
						tmpdata[i]=n;
				}
			}
		}

		return 0;
	}
*/
/// \brief Applies a double threshold to the image
///
///		The method is based on region growing from small presegmented
///		region (img>hi) and is bounded by a larger region (img>lo).
///
///		\param img Input image
///		\param bilevel resulting segmented image
///		\param lo lower threshold level
///		\param hi upper threshold level
///		\param cmp complementation switch
///		\param conn morphological connectivity selector
///		\param mask ROI mask for the processing. If mask=NULL, mask processing will be skipped
template <typename T,size_t NDim>
int DoubleThreshold(kipl::base::TImage<T,NDim> &img, kipl::base::TImage<char,NDim> &bilevel, T lo, T hi, kipl::segmentation::CmpType cmp, kipl::morphology::MorphConnect conn, kipl::base::TImage<char,NDim> *mask=reinterpret_cast<kipl::base::TImage<char,NDim> *>(NULL))
{
	//kipl::base::TImage<char,NDim> *mask=NULL;
	kipl::logging::Logger logger("kipl::segmentation::DoubleThreshold");
	std::ostringstream msg;

	size_t const * const dims=img.Dims();
	bilevel.Resize(dims);
	bilevel=char(0);
	const char th_inque=-1;
	const char th_false=0;
	const char th_true=1;

	int x,y,j,dx,dy,s;
	long n=1;

	msg<<"Starting double threshold (lo="<<lo<<", hi="<<hi<<")";
	logger(kipl::logging::Logger::LogMessage,msg.str());
	msg.str("");

	std::deque<int> posQ;
		
	kipl::morphology::CNeighborhood NG(dims,NDim,conn);
	int Nng=NG.N();

	T *pImg=img.GetDataPtr();
	char *pTmp=bilevel.GetDataPtr();

	long i,pos,p;

	if (mask) {
		long maskscope;
		if (kipl::base::CheckEqualSize(img,*mask))
			maskscope=img.Size();
		else {
			size_t const * const mdims=mask->Dims();
			if ((mdims[0]==dims[0]) && (mdims[1]==dims[1]))
				maskscope=mdims[0]*mdims[1];
		}

		char *pMask=mask->GetDataPtr();
		for (i=0; i<bilevel.Size(); i++) {
			if (cmp==cmp_greater) {
				if ((pImg[i]>hi) && pMask[i % maskscope]) {
					pTmp[i]=th_true;

					for (j=0; j<Nng; j++) {
						if ((pos=NG.neighbor(i,j))!=-1) {
							if ((!pTmp[pos]) && (pImg[pos]>lo) && pMask[pos% maskscope]) {
								posQ.push_back(pos);
								pTmp[pos]=th_inque;
							}
						}
					}
				}
				else
					pTmp[i]=th_false;
			}
			else {
				if (pMask[i%maskscope] && (pImg[i]<lo)) {
					pTmp[i]=th_true;
					for (j=0; j<Nng; j++) {
						if ((pos=NG.neighbor(i,j))!=-1) {
							if ((!pTmp[pos]) && (pImg[pos]<hi) && pMask[pos%maskscope]) {
								posQ.push_back(pos);
								pTmp[pos]=th_inque;
							}
						}
					}
				}
				else
					pTmp[i]=th_false;
			}
		}

		msg.str("");
		msg<<"Queue processing size(queue)="<<posQ.size();
		logger(kipl::logging::Logger::LogMessage,msg.str());


		while (!posQ.empty()) {
			pos=posQ.front();
			posQ.pop_front();
			//if (pTmp[pos]==th_inque) {
				pTmp[pos]=th_true;
				if (cmp==cmp_greater) {
					for (j=0; j<Nng; j++) {
						if ((p=NG.neighbor(pos,j))!=-1) {
							if ((!pTmp[p]) && (pImg[p]>lo) && pMask[p%maskscope]) {
								posQ.push_back(p);
								pTmp[p]=th_inque;
							}

						}
					}
				}
				else {
					for (j=0; j<Nng; j++) {
						if ((p=NG.neighbor(pos,j))!=-1) {
							if (pMask[p%maskscope] && (!pTmp[p]) && (pImg[p]<hi)) {
								posQ.push_back(p);
								pTmp[p]=th_inque;
							}
							
						}
					}
				}
			//}
		}
	}
	else {
		for (i=0; i<bilevel.Size(); i++) {
			if (cmp==cmp_greater) {
				if (pImg[i]>hi) {
					pTmp[i]=1;
					for (j=0; j<Nng; j++) {
						if ((pos=NG.neighbor(i,j))!=-1) {
							if ((!pTmp[pos]) && (pImg[pos]>lo)) {
								posQ.push_back(pos);
								pTmp[pos]=th_inque;
							}
						}
					}
				}
				else
					pTmp[i]=th_false;
			}
			else {
				if (pImg[i]<lo) {
					pTmp[i]=1;
					for (j=0; j<Nng; j++) {
						if ((pos=NG.neighbor(i,j))!=-1) {
							if ((!pTmp[pos]) && (pImg[pos]<hi)) {
								posQ.push_back(pos);
								pTmp[pos]=th_inque;
							}
						}
					}
				}
				else
					pTmp[i]=th_false;
			}
		}

		msg.str("");
		msg<<"Queue processing size(queue)="<<posQ.size();
		logger(kipl::logging::Logger::LogMessage,msg.str());

		while (!posQ.empty()) {
			pos=posQ.front();
			posQ.pop_front();
			pTmp[pos]=1;
			if (cmp==cmp_greater) {
				for (j=0; j<Nng; j++) {
					if ((p=NG.neighbor(pos,j))!=-1) {
						if ((!pTmp[p]) && (pImg[p]>lo)) {
							posQ.push_back(p);
							pTmp[p]=th_inque;
						}

					}
				}
			}
			else {
				for (j=0; j<Nng; j++) {
					if ((p=NG.neighbor(pos,j))!=-1) {
						if ((!pTmp[p]) && (pImg[p]<hi)) {
							posQ.push_back(p);
							pTmp[p]=th_inque;
						}
					}
				}
			}
		}
	}

	return 0;
}

/*
/// \brief Threshold operator that grows the edges of the regions until stability
template<class ImgType, int NDim>	
int RegGrowThresh(const CImage<ImgType,NDim> & img, CImage<char,NDim> & simg, float t0, float t1,  MorphConnect conn=NDim==3 ? conn6 : conn4)
{
	const unsigned int * dims=img.getDimsptr();
	//const int NDim=2;
	simg.resize(dims);
	
	const char fuzzy=-1;
	const char hi=1;
	const char lo=0;
	
//	CImgViewer fig;
	
	int i,j,pos;
	// Initial thresholding
	CNeighborhood NG(dims,NDim,conn);
	int Nng=NG.N();
	
	deque<int> fifo_hiA, fifo_hiB, fifo_loA, fifo_loB;
	deque<int> *pFifo_hiA=&fifo_hiA, *pFifo_hiB=&fifo_hiB;
	deque<int> *pFifo_loA=&fifo_loA, *pFifo_loB=&fifo_loB;
	cout<<"Initial thresholding"<<endl;
	for (i=0; i<img.N(); i++) {
		if (img[i]<=t0) {
			simg[i]=lo;
			continue;
		}
		if (img[i]>t1) {
			simg[i]=hi;
			continue;
		}
		simg[i]=fuzzy;
		for (j=0; j<Nng; j++) {
			if ((pos=NG.neighbor(i,j))!=-1) {
				if (img[pos]<=t0) {
					fifo_loA.push_back(i);
				//	break;
				}
				if (img[pos]>t1) {
					fifo_hiA.push_back(i);
				//	break;
				}
			}
		}
	}
		
	int it_cnt=0;
	cout<<"Processing the fuzzy region "<<endl;
	while (!(pFifo_hiA->empty() || pFifo_loA->empty())) {
		cout<<it_cnt++<<" "<<pFifo_hiA->size()<<", "<<pFifo_loA->size()<<endl; 
		while (!pFifo_hiA->empty()) {
			i=pFifo_hiA->front();
			pFifo_hiA->pop_front();
			if (simg[i]==fuzzy) {
				if (img[i]>t0) {
					simg[i]=hi;
					for (j=0; j<Nng; j++) {
						if ((pos=NG.neighbor(i,j))!=-1) {
							if (simg[pos]==fuzzy) 
								pFifo_hiB->push_back(pos);
						}
					}
				}
				else {
					simg[i]=lo;
					for (j=0; j<Nng; j++) {
						if ((pos=NG.neighbor(i,j))!=-1) {
							if (simg[pos]==fuzzy) 
								pFifo_loB->push_back(pos);
						}
					}
				}
			}
		}
		
		while (!pFifo_loA->empty()) {
			i=pFifo_loA->front();
			pFifo_loA->pop_front();
			if (simg[i]==fuzzy) {
				if (img[i]<t1) {
					simg[i]=lo;
					for (j=0; j<Nng; j++) {
						if ((pos=NG.neighbor(i,j))!=-1) {
							if (simg[pos]==fuzzy) 
								pFifo_loB->push_back(pos);
						}
					}
				}
				else {
					simg[i]=hi;
					for (j=0; j<Nng; j++) {
						if ((pos=NG.neighbor(i,j))!=-1) {
							if (simg[pos]==fuzzy) 
								pFifo_hiB->push_back(pos);
						}
					}
				}
			}
		}
		
		if (fifo_hiA.empty()) {
			pFifo_hiA=&fifo_hiB;
			pFifo_hiB=&fifo_hiA;
			pFifo_loA=&fifo_loB;
			pFifo_loB=&fifo_loA;
		}
		else {
			pFifo_hiA=&fifo_hiA;
			pFifo_hiB=&fifo_hiB;
			pFifo_loA=&fifo_loA;
			pFifo_loB=&fifo_loB;
		}
	}
	
	cout<<endl<<"Done"<<endl;
	return 1;
}
*/

	/// \brief Computes the image threshold using an entropy based approach
	///	the method delivers a central threshold and two hi/lo thresholds
	/// \param hist Vector containing the histogram of the image to be thresholded
	/// \param re tuning parameter to determine the width of the threshold interval
	/// \param Tc central threshold that maximize the cost function
	/// \param T0 lower threshold
	/// \param T1 upper threshold
	int Threshold_Entropy(vector<long> &hist, vector<float> & inter, float re, int &Tc, int &T0, int &T1);
	
	/// \brief Computes the image threshold acording to Otsu
	/// \param hist Vector containing the histogram of the image to be thresholded
	/// \bug Sometimes the second attempt fails...
	int Threshold_Otsu(size_t const * const hist, const size_t N);

	/// \brief computes the threshold for a unimodal histogram
	///	\param hist The histogram vector
	///	\param tail Selects the tail
	///	\param median_filter_len Length of the median filter
	int Threshold_Rosin(size_t const * const hist, const TailType tail=tail_left, const size_t median_filter_len=0);
}}


#endif
