/////////////////////////////////////////////////////////////////////////
//  ClientMessageHandler.h    - Client side message handler		       //
//  ver 1.0                                                            //
//  Language:       Visual C++, ver 2010                               //
//  Platform:       MacBook Pro, Windows7 Pro				           //
//  Application:    CSE687 project #4	                               //
//  Author:         Ider Zheng, Syracuse University					   //
//                  (315) 560-4977, ider.cs@gmail.com				   //
/////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <stack>
using namespace std;

#include "ClientMessageHandler.h"
using namespace Ider;

//////////////////////////////////////////////////////////////////////////
//Receive message from server, call specific message process
void MessageHandler::ReceiveMessage(conStrRef message)
{
	_msg.SetMessage(message);
	switch (_msg.Type())
	{
	case MsgType::Login: 
		LoginProcess();
		break;
	case MsgType::Dependency: 
		DependencyProcess();
		break;
	case MsgType::File: 
		FileProcess();
		break;
	case MsgType::Checkin: 
		CheckinProcess();
		break;
	case MsgType::Commit: 
		CommitProcess();
		break;
	case MsgType::Package: 
		PackageProcess();
		break;
	case MsgType::Warning: 
		WarningProcess();
		break;
	default:
		break;
	}

}

//////////////////////////////////////////////////////////////////////////
//Generate message for sending to server
Message MessageHandler::MessageForSending(MsgType::Value type)
{
	switch (type)
	{
	case MsgType::Login: 
		return LoginMessage();
	case MsgType::Dependency: 
		return DependencyMessage();
	case MsgType::File: 
		return FileMessage();
	case MsgType::Checkin: 
		return CheckinMessage();
	case MsgType::Commit: 
		return CommitMessage();
	case MsgType::Package: 
		return PackageMessage();
	default:
		break;
	}

	return Message();
}

//////////////////////////////////////////////////////////////////////////
//Process File message
void MessageHandler::FileProcess()
{
	if(_msg.Type()!=MsgType::File)return;
	strVal tagName ="Name";

	vector<XmlDoc> names = _msg.Doc().Children(tagName);
	size_t count = names.size();
	if (cout<=0)return;

	vector<strVal> files;
	files.reserve(count);
	for (size_t i=0; i<count; ++i)
		files.push_back(names[i].InnerText());

	_form->UploadFiles(files);
}

//////////////////////////////////////////////////////////////////////////
//Process Login message
void MessageHandler::LoginProcess()
{
	if(_msg.Type()!=MsgType::Login)return;

	//request unclosed packages that checked in by this user
	_form->RequestChickedin = true;
	_form->SendMessage(MsgType::Checkin);

	//request all packages in repository
	_form->SendMessage(MsgType::Dependency);
}

//////////////////////////////////////////////////////////////////////////
//Process Checkin message
void MessageHandler::CheckinProcess()
{
	if(_msg.Type()!=MsgType::Checkin)return;
	
  	System::Collections::Generic::List<System::String^>^ packages
  		=gcnew System::Collections::Generic::List<System::String^>();
   	
	vector<XmlDoc> deps = _msg.Doc().Children("Name");
	strVal name;
	for (size_t i = 0; i<deps.size(); ++i)
	{
		name = deps[i].InnerText();
		packages->Add(Convert(name));
	}

 	_form->Invoke(_form->ShowCheckinListBox,packages);
}

//////////////////////////////////////////////////////////////////////////
//Process Dependency message
void MessageHandler::DependencyProcess()
{
	if(_msg.Type()!=MsgType::Dependency)return;

  	System::Collections::Generic::List<System::String^>^ packages
  		=gcnew System::Collections::Generic::List<System::String^>();
   	
	vector<XmlDoc> deps = _msg.Doc().Children("Name");
	strVal name;
	for (size_t i = 0; i<deps.size(); ++i)
	{
		name = deps[i].InnerText();
		packages->Add(Convert(name));
	}

 	_form->Invoke(_form->ShowPackageListBox,packages);
}

//////////////////////////////////////////////////////////////////////////
//Process warning message
void MessageHandler::CommitProcess()
{
	//refresh the checked-in list box
	_form->RequestChickedin = true;
	_form->SendMessage(MsgType::Checkin);

	//refresh the dependency list box
	_form->SendMessage(MsgType::Dependency);
}

//////////////////////////////////////////////////////////////////////////
//Process warning message
void MessageHandler::PackageProcess()
{
	System::String^ content = System::String::Empty;
	vector<XmlDoc> name = _msg.Doc().Children("Name");
	if (name.size()>0)
		content = Convert(name[0].InnerText());

	_form->Invoke(_form->ShowPackageLabel, content);
}

//////////////////////////////////////////////////////////////////////////
//Process warning message
void MessageHandler::WarningProcess()
{
	if(_msg.Type()!=MsgType::Warning)return;

	ShowWarning(_msg.Doc().InnerText());
}

//////////////////////////////////////////////////////////////////////////
//convert cli String to std::string
strVal MessageHandler::Convert(System::String^ s)
{
	strVal temp;
	for(int i=0; i<s->Length; ++i)
		temp += (char)s[i];
	return temp;
}

//////////////////////////////////////////////////////////////////////////
//convert std::string to cli String
System::String^ MessageHandler::Convert(conStrRef s)
{
	System::Text::StringBuilder^ temp = gcnew System::Text::StringBuilder();
	for(size_t i=0; i<s.size(); ++i)
		temp->Append((wchar_t)s[i]);
	return temp->ToString();
}

//////////////////////////////////////////////////////////////////////////
//generate a File type message
Message MessageHandler::FileMessage()
{
	const MsgType::Value type = MsgType::File;
	string typeTag = MsgType::EnumToString(type);
	string nameTag = "Name";
	//indicate whether extract only this file
	//or full dependencies
	string flagTag = "Full";

	string fileName = "xmlTran";
	bool flag = false;

	strVal depName = Convert(_form->ListDependency->SelectedItem->ToString());// "*.*";

	if (depName.size()<=0)return Message();

	xmlElem elem(nameTag,depName);
	xmlRep rep(elem.elemStr());

	if (flag) elem = xmlElem(flagTag,"true");
	else elem = xmlElem(flagTag,"false");

	rep.addSibling(elem);
	rep.makeParent(typeTag);

	elem.elemStr() = rep.xmlStr();

	return Message(elem);
}

//////////////////////////////////////////////////////////////////////////
//generate a chickin type message
Message MessageHandler::CheckinMessage()
{
	const MsgType::Value type = MsgType::Checkin;
	strVal typeTag = MsgType::EnumToString(type);
	strVal fileName ="*.*";
	strVal tagName = "Name";
	xmlRep  rep;
	
	if (_form->RequestChickedin == true)
	{
		//request all unclosed packages that checked in by this user
		rep.addSibling(xmlElem(tagName,fileName));
		rep.makeParent(typeTag);
		return Message(rep.xmlStr());
	}

	array<System::String^>^ fileNames = _form->FilesForUpload;
	int count = fileNames->Length;
	if (count<=0)return Message();

	for (int i=0; i<fileNames->Length; ++i)
	{
		fileName = Convert(fileNames[i]);
		rep.addSibling(xmlElem(tagName,fileName));
	}

	rep.makeParent(typeTag);

	return Message(rep.xmlStr());
}

//////////////////////////////////////////////////////////////////////////
//generate a login type message
Message MessageHandler::LoginMessage()
{
	const MsgType::Value type = MsgType::Login;
	strVal typeTag = MsgType::EnumToString(type);
	strVal nameTag = "Name";

	strVal userName = Convert(_form->UserName());
	if (userName.size()<=0)return Message();

	xmlElem elem(nameTag,userName);
	xmlRep rep(elem.elemStr());
	rep.makeParent(typeTag);

	elem.elemStr() = rep.xmlStr();

	return Message(elem);
}

//////////////////////////////////////////////////////////////////////////
//generate a dependency type message
Message MessageHandler::DependencyMessage()
{
	const MsgType::Value type = MsgType::Dependency;
	strVal typeTag = MsgType::EnumToString(type);
	strVal nameTag = "Name";

	strVal depName = Convert(_form->SelectedPackageName());

	if (depName.size()<=0)return Message();

	xmlElem elem(nameTag,depName);
	xmlRep rep(elem.elemStr());
	rep.makeParent(typeTag);

	elem.elemStr() = rep.xmlStr();

	return Message(elem);
}

//////////////////////////////////////////////////////////////////////////
//generate a commit type message
Message MessageHandler::CommitMessage()
{
	const MsgType::Value type = MsgType::Commit;
	strVal typeTag = MsgType::EnumToString(type);
	strVal nameTag = "Name";
	
	//commit selected pack
	if (_form->CommitAll == false)
	{
		if (_form->ListCheckin->SelectedItems->Count<=0)return Message();

		System::String^ pack = _form->ListCheckin->SelectedItem->ToString();
		
		xmlRep rep(xmlElem(nameTag,Convert(pack)));
		rep.makeParent(typeTag);

		return Message(rep.xmlStr());
	}

	//commit all package	
	System::Windows::Forms::ListBox::ObjectCollection^ items
		= _form->ListCheckin->Items;

	xmlRep rep;
	for (int i=0; i<items->Count; ++i )
		rep.addSibling(xmlElem(nameTag,Convert(items[i]->ToString())));
	rep.makeParent(typeTag);

	return Message(rep.xmlStr());
}

//////////////////////////////////////////////////////////////////////////
//generate a package type message
Message MessageHandler::PackageMessage()
{
	if(_form->ListDependency->SelectedItems->Count <=0)return Message();

	const MsgType::Value type = MsgType::Package;
	strVal typeTag = MsgType::EnumToString(type);
	strVal nameTag = "Name";

	strVal pack =Convert(_form->ListDependency->SelectedItem->ToString());
	
	xmlRep rep(xmlElem(nameTag,pack));
	rep.makeParent(typeTag);

	return Message(rep.xmlStr());
}

//////////////////////////////////////////////////////////////////////////
//Show warning message on message box
void MessageHandler::ShowWarning(strVal warning)
{
	_form->ShowMessageBox(Convert(warning));
}


#ifdef CLIENT_MESSAGE_HANDLER_TEST

void main()
{
	Message msg;
	MessageHandler mh;

	msg = mh.MessageForSending(MsgType::Login);
	cout<<msg.ToString()<<endl;
	cout<<"Message type: "<<msg.Type()<<endl<<endl;

	msg = mh.MessageForSending(MsgType::Dependency);
	cout<<msg.ToString()<<endl;
	cout<<"Message type: "<<msg.Type()<<endl<<endl;

	msg = mh.MessageForSending(MsgType::File);
	cout<<msg.ToString()<<endl;
	cout<<"Message type: "<<msg.Type()<<endl<<endl;

	//cout<<msg.ToString()<<endl<<endl;
}

#endif

