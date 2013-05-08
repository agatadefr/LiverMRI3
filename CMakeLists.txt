PROJECT(LiverSegmentationMRI3)

FIND_PACKAGE(ITK REQUIRED)
IF( ITK_FOUND )
  INCLUDE (${USE_ITK_FILE})
ENDIF( ITK_FOUND )

FIND_PACKAGE(VTK REQUIRED)
IF( VTK_FOUND )
  INCLUDE (${USE_VTK_FILE})
ENDIF( VTK_FOUND )


ADD_EXECUTABLE(LiverSegmentationPart1 MACOSX_BUNDLE LiverSegmentationPart1.cxx )

TARGET_LINK_LIBRARIES(LiverSegmentationPart1
    ITKIO ITKBasicFilters ITKNumerics ITKCommon 
    vtkWidgets vtkRendering vtkGraphics vtkHybrid 
    vtkImaging vtkIO vtkFiltering vtkCommon )

ADD_EXECUTABLE(LiverSegmentationPart2 MACOSX_BUNDLE LiverSegmentationPart2.cxx )

TARGET_LINK_LIBRARIES( LiverSegmentationPart2
    ITKIO ITKBasicFilters ITKNumerics ITKCommon 
    vtkWidgets vtkRendering vtkGraphics vtkHybrid 
    vtkImaging vtkIO vtkFiltering vtkCommon )


ADD_EXECUTABLE(LiverMesh MACOSX_BUNDLE LiverMesh.cxx )

TARGET_LINK_LIBRARIES( LiverMesh
    ITKIO ITKBasicFilters ITKNumerics ITKCommon 
    vtkWidgets vtkRendering vtkGraphics vtkHybrid 
    vtkImaging vtkIO vtkFiltering vtkCommon )

ADD_EXECUTABLE(LiverVol MACOSX_BUNDLE LiverVol.cxx )

TARGET_LINK_LIBRARIES( LiverVol
    ITKIO ITKBasicFilters ITKNumerics ITKCommon 
    vtkWidgets vtkRendering vtkGraphics vtkHybrid 
    vtkImaging vtkIO vtkFiltering vtkCommon )

