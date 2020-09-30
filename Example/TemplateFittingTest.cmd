::Adjust the path of the ANN library
set PATH=%PATH%;C:\ShapeAnalysisCode\trunk\ann_1.1.1\\MS_Win32\x64\bin

call ..\VS2010\TemplateFitting\Release\TemplateFitting.exe ..\Example\Template.off ..\Example\TemplateLmks.txt ..\Example\Target.off ..\Example\TargetLmks.txt ..\Example\Result.off

pause

