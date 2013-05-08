#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkCropImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkChangeLabelImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkVotingBinaryHoleFillingImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"
#include "itkCastImageFilter.h"

int main(int argc, char *argv[])
{
	const unsigned int Dimension = 3;
	typedef signed short PixelType;
	typedef itk::Image<PixelType, Dimension> ImageType;
	typedef itk::Image<float, Dimension> InternalImageType;

	// Wczytanie obrazu (saggital plane)
	typedef itk::ImageFileReader<InternalImageType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName("cykl5.mha");
	reader->Update();
	/*std::cout<<"Podaj sciezke do pliku wejsciowego : "<<std::endl;
	std::cin>>plik;*/
	// D:\SegmentacjaWatroby\Liver\build\cykl.mha

	// Wydzielenie obrazu do dalszej segmentacji. Wydzielenie  - arbitralnie dobrane punkty.
	ImageType::SizeType cropSize;
    cropSize[0] = 33;
	cropSize[1] = 33; // 35
	cropSize[2] = 0;

	typedef itk::CropImageFilter <InternalImageType, InternalImageType> CropImageFilterType;
	CropImageFilterType::Pointer cropFilter = CropImageFilterType::New();
	cropFilter->SetBoundaryCropSize(cropSize);
	cropFilter->SetInput(reader->GetOutput());
	cropFilter->Update();

	// Zmiana typu pixela do zapisu 
	typedef itk::CastImageFilter<InternalImageType, ImageType> CastFilterType;
	CastFilterType::Pointer castFilter1 = CastFilterType::New();
	castFilter1->SetInput(cropFilter->GetOutput());
	castFilter1->Update();

	// Zapis wyniku crop image - potem potrzebne do utworzenia LiverVol
	typedef itk::ImageFileWriter<ImageType> WriterType;
	WriterType::Pointer writerCrop = WriterType::New();
	writerCrop->SetFileName("image_crop.mha");
	writerCrop->SetInput(castFilter1->GetOutput());
	writerCrop->Update();

	// Wstepne wygladzenie obrazu przy pomocy curvature filter.
	typedef itk::CurvatureFlowImageFilter<InternalImageType, InternalImageType> CurvatureFilterType;
    CurvatureFilterType::Pointer curvatureFilter1 = CurvatureFilterType::New();
    curvatureFilter1->SetInput(cropFilter->GetOutput());
    const unsigned int numberOfIterations =5; 
    const double       timeStep = 0.25; 

    curvatureFilter1->SetNumberOfIterations( numberOfIterations );
    curvatureFilter1->SetTimeStep(timeStep );
    curvatureFilter1->Update();

	// Progowanie - binaryzacja
	typedef itk::BinaryThresholdImageFilter<InternalImageType, InternalImageType> BinaryThresholdImageType;
	BinaryThresholdImageType::Pointer binFilter = BinaryThresholdImageType::New();
	binFilter->SetInput(curvatureFilter1->GetOutput());
	binFilter->SetLowerThreshold(44);
	binFilter->SetUpperThreshold(55);
	binFilter->SetInsideValue(1);
	binFilter->SetOutsideValue(0);

	// Element strukturalny
	int radius_1 = 60; // bylo ostatnio 54; //ok/48; // 18 - optymal
	typedef itk::BinaryBallStructuringElement<InternalImageType::PixelType, 3> StructuringElementType;
	StructuringElementType strElement_1;
	strElement_1.SetRadius(radius_1);
	strElement_1.CreateStructuringElement();

	// Dylatacja
	typedef itk::BinaryDilateImageFilter<InternalImageType, InternalImageType, StructuringElementType> BinaryDilateImageFilterType;
	BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
	dilateFilter->SetInput(binFilter->GetOutput());
	dilateFilter->SetKernel(strElement_1);
	dilateFilter->Update();

	// Otwarcie 
	typedef itk::BinaryMorphologicalOpeningImageFilter<InternalImageType, InternalImageType, StructuringElementType>
		BinaryOpeningFilterType;
	BinaryOpeningFilterType::Pointer openingFilter = BinaryOpeningFilterType::New();
	openingFilter->SetInput(dilateFilter->GetOutput());
	openingFilter->SetKernel(strElement_1);
	openingFilter->Update();

	// Domkniecie
	typedef itk::BinaryMorphologicalClosingImageFilter<InternalImageType, InternalImageType, StructuringElementType>
		BinaryClosingImageFilterType;
	BinaryClosingImageFilterType::Pointer closingFilter = BinaryClosingImageFilterType::New();
	closingFilter->SetInput(openingFilter->GetOutput());
	closingFilter->SetKernel(strElement_1);
	closingFilter->Update();

	// Wypelnianie dziur
	typedef itk::VotingBinaryHoleFillingImageFilter<InternalImageType, InternalImageType> VotingFilterType;
	VotingFilterType::Pointer voteFilter = VotingFilterType::New();

	InternalImageType::SizeType indexRadius;
	const unsigned int radiusX = 18;
	const unsigned int radiusY = 18;
	const unsigned int radiusZ = 1;

	indexRadius[0] = radiusX; // radius along x
	indexRadius[1] = radiusY; // radius along y
	indexRadius[2] = radiusZ; /// radius along z

	voteFilter->SetRadius(indexRadius);
	voteFilter->SetBackgroundValue(0);
	voteFilter->SetForegroundValue(1);
	voteFilter->SetMajorityThreshold(2);
	voteFilter->SetInput(closingFilter->GetOutput());
	voteFilter->Update();

	// Wygladzenie obrazu curvature filter number 2
    CurvatureFilterType::Pointer curvatureFilter2 = CurvatureFilterType::New();
    curvatureFilter2->SetInput(voteFilter->GetOutput());
    curvatureFilter2->SetNumberOfIterations(10); // bylo 10 
    curvatureFilter2->SetTimeStep(timeStep );
    curvatureFilter2->Update();

	// Element strukturalny
	int radius_2 = 48; 
	StructuringElementType strElement_2;
	strElement_2.SetRadius(radius_2);
	strElement_2.CreateStructuringElement();

	// Dylatacja
	BinaryDilateImageFilterType::Pointer dilateFilter2 = BinaryDilateImageFilterType::New();
	dilateFilter2->SetInput(curvatureFilter2->GetOutput());
	dilateFilter2->SetKernel(strElement_2);
	dilateFilter2->Update();

	// Zmiana typu pixela z float -> signed short
	typedef itk::CastImageFilter<InternalImageType, ImageType> CastFilterType;
	CastFilterType::Pointer castFilter2 = CastFilterType::New();
	castFilter2->SetInput(dilateFilter2->GetOutput());
	castFilter2->Update();

	WriterType::Pointer writerBin1 = WriterType::New();
    writerBin1->SetFileName("WysegmentowanaWatrobaPart1.mha"); 
    writerBin1->SetInput(castFilter2->GetOutput());
    writerBin1->Update();

	return EXIT_SUCCESS;
}