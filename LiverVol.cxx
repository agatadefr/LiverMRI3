#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMultiplyImageFilter.h"

int main()
{
	const unsigned int Dimension = 3;
	typedef signed short PixelType;
	typedef itk::Image<PixelType, Dimension> ImageType;
	
	// Wczytanie obrazu organalnego
	// Nalezy wczytac przyciety obraz z obszarem watroby (LiverSegmentation1.cxx)
	typedef itk::ImageFileReader<ImageType> ReaderType;
	ReaderType::Pointer reader1 = ReaderType::New();
	reader1->SetFileName("image_crop.mha"); // Wczytanie przyciêtego obrazu orginalnego 
	reader1->Update();

	// Wczytanie obrazu binarnego - wynik segmentacji part2.
	ReaderType::Pointer reader2 = ReaderType::New();
	reader2->SetFileName("WysegmentowanaWatrobaPart2.mha");
	reader2->Update();
    
	typedef itk::MultiplyImageFilter<ImageType, ImageType> MultiplyImageFilterType;
	MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New();
	multiplyFilter->SetInput1(reader1->GetOutput());
	multiplyFilter->SetInput2(reader2->GetOutput());

	// Zapis wysegmentowanej watroby Vol (watroba + czarne tlo).
	typedef itk::ImageFileWriter<ImageType> WriterType;
	WriterType::Pointer writerVol = WriterType::New();
    writerVol->SetFileName("WatrobaVol_MRI.mha"); 
    writerVol->SetInput(multiplyFilter->GetOutput());
    writerVol->Update();

	return  EXIT_SUCCESS;
}