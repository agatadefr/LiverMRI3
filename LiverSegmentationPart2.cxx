#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCropImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkVotingBinaryHoleFillingImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkBinaryPruningImageFilter.h"
#include "itkAntiAliasBinaryImageFilter.h"

int main(int argc, char *argv[])
{
	const unsigned int Dimension = 3;
	typedef signed short PixelType;
	typedef itk::Image<PixelType, Dimension> ImageType;
	typedef itk::Image<float, Dimension> InternalImageType;

	typedef itk::ImageFileReader<InternalImageType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName("WysegmentowanaWatrobaPart1.mha");
	reader->Update();

	int iteration = 100;//60; // bylo 36 opt
	typedef itk::BinaryPruningImageFilter <InternalImageType, InternalImageType >  BinaryPruningImageFilterType;
	BinaryPruningImageFilterType::Pointer pruneFilter = BinaryPruningImageFilterType::New();
	pruneFilter->SetInput(reader->GetOutput());
	pruneFilter->SetIteration(iteration);
	pruneFilter->GetOutput();

	// Wypelnianie dziur
	typedef itk::VotingBinaryHoleFillingImageFilter<InternalImageType, InternalImageType> VotingFilterType;
	VotingFilterType::Pointer voteFilter = VotingFilterType::New();

	InternalImageType::SizeType indexRadius;
	const unsigned int radiusX = 9;
	const unsigned int radiusY = 9;
	const unsigned int radiusZ = 1;

	indexRadius[0] = radiusX; // radius along x
	indexRadius[1] = radiusY; // radius along y
	indexRadius[2] = radiusZ; /// radius along z

	voteFilter->SetRadius(indexRadius);
	voteFilter->SetBackgroundValue(0);
	voteFilter->SetForegroundValue(1);
	voteFilter->SetMajorityThreshold(2);
	voteFilter->SetInput(pruneFilter->GetOutput());
	voteFilter->Update();

	int radius_1 = 36; 
	typedef itk::BinaryBallStructuringElement<InternalImageType::PixelType, InternalImageType::ImageDimension> StructuringElementType;
	StructuringElementType strElement1;
	strElement1.SetRadius(radius_1);
	strElement1.CreateStructuringElement();
 
	typedef itk::BinaryErodeImageFilter <InternalImageType, InternalImageType, StructuringElementType>
    BinaryErodeImageFilterType;
	BinaryErodeImageFilterType::Pointer erodeFilter = BinaryErodeImageFilterType::New();
	erodeFilter->SetInput(voteFilter->GetOutput());
	erodeFilter->SetKernel(strElement1);

	typedef itk::BinaryDilateImageFilter<InternalImageType, InternalImageType, StructuringElementType>
	BinaryDilateImageFilterType;
	BinaryDilateImageFilterType::Pointer dilateFilter = BinaryDilateImageFilterType::New();
	dilateFilter->SetInput(erodeFilter->GetOutput());
	dilateFilter->SetKernel(strElement1);

	const unsigned int numberOfIterations = 5; 
    const double       timeStep = 0.25; 
	typedef itk::CurvatureFlowImageFilter<InternalImageType, InternalImageType> CurvatureFilterType;
	CurvatureFilterType::Pointer curvatureFilter3 = CurvatureFilterType::New();
    curvatureFilter3->SetInput(dilateFilter->GetOutput());
    curvatureFilter3->SetNumberOfIterations(numberOfIterations); 
    curvatureFilter3->SetTimeStep(timeStep );
    curvatureFilter3->Update();

	// Zmiana typu pixela z float na signed short
	typedef itk::CastImageFilter<InternalImageType, ImageType> CastFilterType;
	CastFilterType::Pointer castFilter = CastFilterType::New();
	castFilter->SetInput(curvatureFilter3->GetOutput());
	castFilter->Update();

	typedef itk::ImageFileWriter<ImageType> WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName("WysegmentowanaWatrobaPart2.mha");
    writer->SetInput(castFilter->GetOutput());
    writer->Update();

	return EXIT_SUCCESS;
}



  
