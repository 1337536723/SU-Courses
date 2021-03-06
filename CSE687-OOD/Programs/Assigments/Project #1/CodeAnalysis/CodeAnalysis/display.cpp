//////////////////////////////////////////////////////////////////////
//  display.cpp - print analysis result to console					//
//  ver 1.0															//
//																	//
//  Language:      Visual C++ 2010, SP1								//
//  Platform:      Mac Book Pro, Windows 7 Professional				//
//  Application:   Prototype for CSE687 Pr1, Sp09					//
//  Author:        Ider Zheng, Syracuse University					//
//                 (315) 560-4977, ider.zheng@gmail.com			    //
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include "display.h"

//////////////////////////////////////////////////////////////////////////
//print analysis result
void display::printAnalysis(packageInfo* packInfo)
{
	pack = packInfo;
	printPackageInfo();
	printFunctions();
	printSummary();
}

//////////////////////////////////////////////////////////////////////////
//print names of package files
void display::printPackageInfo()
{
	funcCount = 0;
	dataCount = 0;
	deepestScope = 0;
	std::cout<<"\n\nPackage: ";
	for (int i = 0; i< pack->fileCount(); ++i)
	{
		std::cout<<"\n"<< (*pack)[i];
	}

	std::cout<<std::endl;
	std::cout<<std::string(70,'=')<<std::endl;
}

//////////////////////////////////////////////////////////////////////////
//print all function information of package
void display::printFunctions()
{
	std::vector<funcInfo*>& funcs = pack->functions;
	std::vector<funcInfo*>::iterator it;
	funcCount+=funcs.size();
	for (it=funcs.begin();it!=funcs.end();++it)
	{
		funcInfo* func = (*it);
		if (deepestScope < func->deepestBrace)deepestScope=func->deepestBrace;
		std::cout<<"\n\tFunction Name:\t";
		std::cout<<func->getFullName()<<"\t";
		//std::cout<<"Class:"<<func->getClassName();
		std::cout<< "\n\tLines:  " <<func->getFunctionSize();
		if (func->getFunctionSize()>optimalFuncSize)std::cout<<"(*)";
		std::cout<<"\n\tScope Nesting:  " << func->getFunctionScopeNesting();
		std::cout<<"\n\tCyclomatic Complexity:  "<<func->cyclometer;
		if (func->cyclometer>optimalCyclom)std::cout<<"(*)";
		std::cout << std::endl;	
		printControls(func->controls);
		printVaraibels(func->variables);
		std::cout<<std::string(70,'*');
	}
	std::cout<<std::endl;
}

//////////////////////////////////////////////////////////////////////////
//print all
void display::printControls(std::vector<controlInfo*>& ctrls)
{
	if (ctrls.size() <=0)return;
	dataCount += ctrls.size();

	std::cout<<"\n\tInformations of controls in this function:"<<std::endl;
	std::vector<controlInfo*>::iterator it;
	std::cout<<"\t"<<std::string(50,'-')<<std::endl;
	std::cout<<"\tLine@\tLines\tName"<<std::endl;;
	for (it = ctrls.begin(); it != ctrls.end(); ++it)
	{
		controlInfo& ctrl = *(*it);
		std::cout<<"\t "<<ctrl.getBeginLine();
		std::cout<<"\t "<<ctrl.getControlSize();
		if (ctrl.getControlSize()>optimalCtrlSpan)std::cout<<"(*)";
		std::cout<<"\t"<<ctrl.getName();
		std::cout<<std::endl;
	}
}

//////////////////////////////////////////////////////////////////////////
//print names of package files
void display::printVaraibels(std::vector<variableInfo*>& vInfos)
{
	if (vInfos.size() <=0)return;

	std::cout<<"\n\tInformations of variables:"<<std::endl;
	std::vector<variableInfo*>::iterator it;
	std::cout<<"\t"<<std::string(50,'-')<<std::endl;

	std::cout<<"\tLine@\tRef#\tDensity\t\tType - Name"<<std::endl;;
	for (it = vInfos.begin(); it != vInfos.end(); ++it)
	{
		variableInfo& var = *(*it);
		std::cout<<"\t "<<var.getBeginLine();
		std::cout<<"\t "<<var.getReferencedCount();
		if (var.getReferencedCount()>optimalReference)std::cout<<"(*)";
		std::cout<<"\t "<<var.getLocality();
		if (var.getLocality()>optimalLocality)std::cout<<"(*)";
		std::cout<<"\t\t"<<var.getType();
		std::cout<<" - "<<var.getName();
		std::cout<<std::endl;
	}
}

void display::printSummary()
{
	std::cout<<"\tSummary\t"<<std::string(40,'-')<<std::endl;
	std::cout<<"\t\t\tFunc#\tData#\tDeepest Scope"<<std::endl;

	std::cout<<"\t\t\t "<<funcCount;
	std::cout<<"\t "<<dataCount;
	std::cout<<"\t\t"<<deepestScope;
	std::cout<<std::endl;

	std::cout<<"\t\t"<<std::string(40,'-')<<std::endl;
}


//
//----< test stub >--------------------------------------------


#ifdef DISPLAY_TEST

#include "fileHandler.h"

int main(int argc, char* argv[])
{
	std::cout << "\n  Testing Display module\n "
		<< std::string(32,'=') << std::endl;

	// collecting tokens from files, named on the command line

	if(argc < 2)
	{
		std::cout 
			<< "\n  please enter name of file to process on command line\n\n";
		return 1;
	}

	fileHandler handler;
	try
	{
		handler.attach(argc,argv);
		handler.parse();
		handler.printResult();
	}
	catch(std::exception& ex)
	{
		std::cout << "\n\n    " << ex.what() << "\n\n";
	}

}


#endif