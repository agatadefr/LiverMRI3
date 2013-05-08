#include "itkImage.h"
#include "itkOrientedImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkConfidenceConnectedImageFilter.h"
#include "itkRGBPixel.h"
#include "itkCommand.h"
#include "itkVTKImageExport.h"
#include "itkVTKImageImport.h"

#include "vtkImageImport.h"
#include "vtkImageExport.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkActor.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkContourFilter.h"
#include "vtkImageData.h"
#include "vtkDataSet.h"
#include "vtkProperty.h"
#include "vtkImagePlaneWidget.h"
#include "vtkCellPicker.h"
#include "vtkPolyDataWriter.h"

template <typename ITK_Exporter, typename VTK_Importer>
void ConnectPipelines(ITK_Exporter exporter, VTK_Importer* importer)
{
  importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
  importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
  importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
  importer->SetSpacingCallback(exporter->GetSpacingCallback());
  importer->SetOriginCallback(exporter->GetOriginCallback());
  importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
  importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
  importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
  importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
  importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
  importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
  importer->SetCallbackUserData(exporter->GetCallbackUserData());
}

template <typename VTK_Exporter, typename ITK_Importer>
void ConnectPipelines(VTK_Exporter* exporter, ITK_Importer importer)
{
  importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
  importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
  importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
  importer->SetSpacingCallback(exporter->GetSpacingCallback());
  importer->SetOriginCallback(exporter->GetOriginCallback());
  importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
  importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
  importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
  importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
  importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
  importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
  importer->SetCallbackUserData(exporter->GetCallbackUserData());
}


int main()
{

// Def typ pixela
typedef signed short PixelType; 
// Def wymiar
const unsigned int Dimension = 3; 
// def typ obrazu
typedef itk::OrientedImage< PixelType, Dimension > ImageType; 
typedef itk::ImageFileReader< ImageType > ReaderType;

try 
{ 
	// Wczytanie obrazu WE
	ReaderType::Pointer reader = ReaderType::New();
	std::string plik = "WysegmentowanaWatrobaPart2.mha";
	/*std::cout<<"Podaj sciezke do pliku wejsciowego : "<<std::endl;
	std::cin>>plik;*/
	// D:\SegmentacjaWatroby\Liver\build\Volumen3D-Watroba.dcm

	reader->SetFileName(plik);
	reader->Update(); 
	std::cout << reader<< std::endl;

	// Filtr do wydzielenia powierzchni obiektu - region growing 
    typedef itk::ConfidenceConnectedImageFilter<ImageType,ImageType> SegmentationFilterType;
    SegmentationFilterType::Pointer filter = SegmentationFilterType::New();
    filter->SetInput( reader->GetOutput() );
    filter->SetNumberOfIterations(2);
    filter->SetReplaceValue(255);
    filter->SetMultiplier(2.5);

    // Wyznaczanie srodka obiektu/ obrazu
    ImageType::Pointer inputImage = reader->GetOutput();
    ImageType::SizeType  size  = inputImage->GetBufferedRegion().GetSize();
    ImageType::IndexType start = inputImage->GetBufferedRegion().GetIndex();

    // Arbitralnie dobrany punkt startowy
    ImageType::IndexType seed;
    seed[0] = start[0] + size[0] / 2;
    seed[1] = start[1] + size[1] / 2;
    seed[2] = start[2] + size[2] / 2;
    filter->SetSeed( seed );
      
    // Export z itk -> vtk
    typedef itk::VTKImageExport< ImageType > ExportFilterType;
    ExportFilterType::Pointer itkExporter1 = ExportFilterType::New();
    ExportFilterType::Pointer itkExporter2 = ExportFilterType::New();

    itkExporter1->SetInput( reader->GetOutput() );
    itkExporter2->SetInput( filter->GetOutput() );

    // Stworzenie mostka po³¹cznie itk -> vtk w celu wizualizacji., stworzenia modelu
    // itk::VTKImageExport instance.
    vtkImageImport* vtkImporter1 = vtkImageImport::New();  
    ConnectPipelines(itkExporter1, vtkImporter1);
    
    vtkImageImport* vtkImporter2 = vtkImageImport::New();  
    ConnectPipelines(itkExporter2, vtkImporter2);
    vtkImporter1->Update();
     
    // Utworzenie kamery, okna wyœwielania oraz aktora w celu zwizualizowania rezultatów
    vtkRenderer* renderer = vtkRenderer::New();
    vtkRenderWindow* renWin = vtkRenderWindow::New();
    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();

    renWin->SetSize(500, 500);
    renWin->AddRenderer(renderer);
    iren->SetRenderWindow(renWin);

    // Utowrzenie pickera aby móc np. obracac model
    vtkCellPicker * picker = vtkCellPicker::New();
    picker->SetTolerance(0.005);

    // Na³o¿enie tekstur
    vtkProperty * ipwProp = vtkProperty::New();
    renderer->SetBackground(0.4392, 0.5020, 0.5647);
    
	// Filtr do ekstrakcji powierzchni 
    vtkContourFilter * contour = vtkContourFilter::New();
    contour->SetInput( vtkImporter2->GetOutput() );
    contour->SetValue(0, 128); 

    vtkPolyDataMapper * polyMapper = vtkPolyDataMapper::New();
    vtkActor          * polyActor  = vtkActor::New();

    polyActor->SetMapper( polyMapper );
    polyMapper->SetInput( contour->GetOutput() );
    polyMapper->ScalarVisibilityOff();

	// Aplikowanie w³asnoœci takich jak kolor, jasnosæ, reprezentacja itd..
    vtkProperty * property = vtkProperty::New();
    property->SetAmbient(0.1);
    property->SetDiffuse(0.1);
    property->SetSpecular(0.5);
    property->SetColor(1.0,0.0,0.0);
    property->SetLineWidth(2.0);
	property->SetRepresentationToPoints();
    property->SetRepresentationToSurface();
    polyActor->SetProperty( property );
    renderer->AddActor( polyActor );

	// Zapis do pliku vtk utworzonego modelu/siatki
    vtkPolyDataWriter * writer = vtkPolyDataWriter::New(); 
	writer->SetFileName("LiverMesh_cykl5.vtk");
    writer->SetInput(contour->GetOutput());
    writer->Write();

    // Rozpoczêcie renderowania, okno interakcja z u¿ytkownikiem.
    renderer->ResetCamera();
    renWin->Render();
    iren->Start();

    // Zwalnianie zalokowanej pamiêci na obiekty.
    polyActor->Delete();
    picker->Delete();
    ipwProp->Delete();
    vtkImporter1->Delete();
    vtkImporter2->Delete();
    contour->Delete();
    property->Delete();
    polyMapper->Delete();
    renWin->Delete();
    renderer->Delete();
    iren->Delete();

} catch (itk::ExceptionObject &ex) 
{ 
   std::cout << ex << std::endl; 
   return EXIT_FAILURE; 
} 

}
