#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H
/////////////////////////////////////////////////////////////////////////
//  UserInterface.h    -  User interface form on client side   		   //
//  ver 1.0                                                            //
//  Language:       Visual C++, ver 2010                               //
//  Platform:       MacBook Pro, Windows7 Pro				           //
//  Application:    CSE687 project #4	                               //
//  Author:         Ider Zheng, Syracuse University					   //
//                  (315) 560-4977, ider.cs@gmail.com				   //
/////////////////////////////////////////////////////////////////////////
/*
Module Operations:
==================
This module is a windows form class.
It provide client interface for sending message to
and requesting information from server,


Public Interface:
=================
UserInterface^ ui = gcnew UserInterface();
ui->SetMessageHandler(mh);
_form->Invoke(_form->ShowCheckinListBox,packages);
_form->Invoke(_form->ShowPackageListBox,packages);


Build Process:
==============
Required Files:
ClientMessageHandler.h, ClientProcess.cpp, ClientProcess.h

*/

#include "ClientMessageHandler.h"
#include "ClientProcess.h"

class MessageHandler;

#pragma once

namespace Client 
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;


	/// <summary>
	/// Summary for UserInterface
	/// </summary>
	public ref class UserInterface : public System::Windows::Forms::Form
	{
	public:
		//delegate
		delegate System::Void ShowListBoxDelegate(System::Collections::Generic::List<System::String^>^ items);
		delegate System::Void ShowLabelDelegate(System::String^ content);
		ShowListBoxDelegate^ ShowPackageListBox;
		ShowListBoxDelegate^ ShowCheckinListBox;
		ShowLabelDelegate^ ShowPackageLabel;

	public: 

		//constructor
		UserInterface(void)
		{
			this->selectionTrack = gcnew System::Collections::Generic::List<String^>();
			InitializeComponent();
			InitializeDelegate();
		}

		//set message handler for form
		System::Void SetMessageHandler(Ider::IMessageHandler* mh)
		{
			this->mh=mh;
		}

		//show message on message box
		System::Void ShowMessageBox(System::String^ msg)
		{
			System::Windows::Forms::MessageBox::Show(msg, "Message", MessageBoxButtons::OK, MessageBoxIcon::Information);
		}

		//send message to server side
		System::Boolean SendMessage(Ider::MsgType::Value type)
		{
			if (ConnectServer())
			{
				Ider::Message msg = this->mh->MessageForSending(type);

				if (msg.Type()==Ider::MsgType::Unknown)return false;

				this->sender->postMessage(msg);
				CloseConnection();
				return true;
			}

			return false;
		}

		System::Void ShowPackageDetail(System::String^ content)
		{
			txtDetail->Lines = content->Split('\n');
		}

		//get selected package name on listbox
		System::String^ SelectedPackageName()
		{
			//first request, ask for all packages
			if (this->selectionTrack->Count<=0)return L"*.*";

			System::String^ preName = 
				this->selectionTrack[this->selectionTrack->Count-1];

			//no selection, refresh current package
			if(this->listDep->SelectedItems->Count<=0)
				return preName;

			//go back up
			System::String^ name;
			if(this->listDep->SelectedIndex==0)
			{
				if (this->selectionTrack->Count<=1)
				{
					//"*.*" will not be pop up, to indicate the highest top
					ShowMessageBox(L"Could not go back any more.");
					return System::String::Empty;
				}
				else
				{
					//get last requested name
					int last = this->selectionTrack->Count-2;
					name = this->selectionTrack[last];
					return name;
				}
			}

			//request the selected package 
			name = this->listDep->SelectedItem->ToString();
			if (name == preName)
				return System::String::Empty;

			return name;
		}

		//get username for login
		System::String^ UserName()
		{
			System::String^ name = this->txtUsrName->Text->Trim();
			if (name->Length<=0)
			{
				ShowMessageBox("Please enter user name.");
				return System::String::Empty;
			}
			return name;
		}

		//show packages on listbox
		System::Void ShowDependencies(System::Collections::Generic::List<System::String^>^ packages)
		{
			if (this->selectionTrack->Count<=0)
			{
				this->selectionTrack->Add(L"*.*");
				this->pnlLogin->Visible = false;
				this->Text = "Hello, "+this->txtUsrName->Text->Trim();
			}
			else if (this->listDep->SelectedItems->Count>0)
			{
				//go back up, remove last one in track
				if (this->listDep->SelectedIndex==0)
				{
					int last = this->selectionTrack->Count-1;
					this->selectionTrack->RemoveAt(last);
				}
				else
				{
					//push selection into tack to for go back
					this->selectionTrack->Add(this->listDep->SelectedItem->ToString());
				}
			}
			//else { /* refresh current package dependency */}

			txtDetail->Text = System::String::Empty;
			this->listDep->Items->Clear();
			this->listDep->Items->Add("..");

			for (System::Int32 i=0; i<packages->Count; ++i)
				this->listDep->Items->Add(packages[i]);

			this->lblPack->Text = this->selectionTrack[this->selectionTrack->Count-1];
			if (this->lblPack->Text =="*.*") this->lblPack->Text = "All Packages:";
			else this->lblPack->Text = "Dependencies of "+ this->lblPack->Text+":" ;
		}

		//show checked in on listbox
		System::Void ShowCheckedin(System::Collections::Generic::List<System::String^>^ packages)
		{
			this->listCheckin->Items->Clear();

			for (System::Int32 i=0; i<packages->Count; ++i)
				this->listCheckin->Items->Add(packages[i]);
		}

		//post files to server
		void UploadFiles(std::vector<std::string> files)
		{
			if (ConnectServer())
			{
				for (size_t i=0; i<files.size(); ++i)
					this->sender->postFile(files[i]);

				System::Threading::Thread::Sleep(700);
				this->requestChickedin = true;
				SendMessage(Ider::MsgType::Checkin);

				CloseConnection();
			}
		}

#pragma region properties

	public:

		property array<System::String^>^ FilesForUpload
		{
			array<System::String^>^ get(){return this->fileDialog->FileNames;}
		}

		property System::Boolean CommitAll
		{
			System::Boolean get(){return (this->cbAll->Checked);}
		}

		property System::Windows::Forms::ListBox^ ListCheckin
		{
			System::Windows::Forms::ListBox^ get(){return this->listCheckin;}
		}

		property System::Windows::Forms::ListBox^ ListDependency
		{
			System::Windows::Forms::ListBox^ get(){return this->listDep;}
		}

		property System::Boolean RequestChickedin
		{
			System::Boolean get()
			{
				System::Boolean temp = requestChickedin;
				requestChickedin = false;
				return temp;
			}
			System::Void set(System::Boolean value){requestChickedin = value;}
		}

#pragma endregion

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~UserInterface()
		{
			if (components)
			{
				delete components;
			}

			if(sender)
			{
				delete sender;
				delete msgProc;
				delete fileProc;
			}
		}

		//Initialize delegate for cross-thread control refresh
		System::Void InitializeDelegate()
		{
			ShowPackageListBox += gcnew ShowListBoxDelegate(this,&UserInterface::ShowDependencies);
			ShowCheckinListBox += gcnew ShowListBoxDelegate(this,&UserInterface::ShowCheckedin);
			ShowPackageLabel += gcnew ShowLabelDelegate(this,&UserInterface::ShowPackageDetail);
		}

		//create a communicator socket
		System::Void CreateCommunicator()
		{
			if (sender!=NULL)return;

			System::String^ text = this->txtCitIP->Text->Trim();
			int port =0;
			if (this->txtSvrPort->Text->Trim() == this->txtCitPort->Text->Trim()
				|| !System::Int32::TryParse(this->txtCitPort->Text->Trim(), port))
				ShowMessageBox("Number only!");

			std::string ip;
			for(int i=0; i<text->Length; ++i)
				ip += (char)text[i];

			EndPoint addr(ip,port);

			sender = new Communicator(addr);

			msgProc = new MsgHandler<ClientMessage_Proc>();
			msgProc->setCommunicator(sender);
			sender->attachMsgHandler(msgProc);

			fileProc =new FileHandler<ClientFile_Proc>();
			fileProc->setCommunicator(sender);
			sender->attachFileHandler(fileProc);

			sender->setFileSource("");
			fileProc->setFileDestination(".\\FilePostTest\\Receiver\\");

			sender->listen();
		}

		//connect to server
		System::Boolean ConnectServer()
		{
			if (this->connected)return true;

			if (this->sender == NULL)CreateCommunicator();

			System::String^ text = this->txtSvrIP->Text->Trim();
			int port =0;
			if (!System::Int32::TryParse(this->txtSvrPort->Text->Trim(), port))
				ShowMessageBox("Invalid port number");

			std::string ip;
			for(int i=0; i<text->Length; ++i)
				ip += (char)text[i];

			if (!this->sender->connect(ip ,port))
			{
				ShowMessageBox("Failed to connect repository server.\nPlease try again later.");
				return this->connected=false;
			}

			return this->connected=true;
		}

		//Close server connection if connection is open
		System::Void CloseConnection()
		{
			if (!this->connected)return;

			this->sender->disconnect();

			this->connected = false;
		}

		//Login button event
		System::Void btnLogin_Click(System::Object^ sender, System::EventArgs^ e)
		{
			if (ConnectServer())
			{
				SendMessage(Ider::MsgType::Login);
				CloseConnection();
			}
		}

		//Dependency button event
		System::Void btnDep_Click(System::Object^ sender, System::EventArgs^ e)
		{
			if (this->listDep->SelectedItems->Count<=0
				|| this->listDep->SelectedIndex == 0)
				ShowMessageBox("Please select a package name.");
			if (this->listDep->SelectedIndex == 0)return;

			SendMessage(Ider::MsgType::Package);
		}

		//Extraction button event
		System::Void btnExt_Click(System::Object^ sender, System::EventArgs^ e) 
		{
			if (this->listDep->SelectedItems->Count<=0
				|| this->listDep->SelectedIndex<=0)
			{
				ShowMessageBox("Please select a file to download.");
			}
			else
			{
				System::Windows::Forms::DialogResult result = folderDialog->ShowDialog();
				if (result == System::Windows::Forms::DialogResult::Cancel)return;

				System::String^ text = folderDialog->SelectedPath;

				std::string path;
				for(int i=0; i<text->Length; ++i)
					path += (char)text[i];
				path+='\\';

				fileProc->setFileDestination(path);
				SendMessage(Ider::MsgType::File);
			}
		}

		//Upload button event
		System::Void btnUpload_Click(System::Object^ sender, System::EventArgs^ e) 
		{
			System::Windows::Forms::DialogResult result = this->fileDialog->ShowDialog();

			if (result == System::Windows::Forms::DialogResult::Cancel)return;
			int count = this->fileDialog->FileNames->Length;
			if (count<=0)return;

			//set both flag to false, so that MessageHanlder would send Chickin message for upload
			SendMessage(Ider::MsgType::Checkin);
		}

		//Checkin close button event
		System::Void btnClose_Click(System::Object^ sender, System::EventArgs^ e) 
		{
			if (listCheckin->Items->Count<=0)
			{
				ShowMessageBox("All packages are closed.");
				return;
			}

			if (!cbAll->Checked && listCheckin->SelectedItems->Count<=0)
			{
				ShowMessageBox("Please select a package.");
				return;
			}

			listDep->SelectedItems->Clear();
			SendMessage(Ider::MsgType::Commit);
		}

		//Dependency listbox event
		System::Void listDep_DoubleClick(System::Object^ sender, System::EventArgs^ e) 
		{
			if (this->listDep->SelectedItems->Count<=0)
				ShowMessageBox("Please select a package name.");
			else
				SendMessage(Ider::MsgType::Dependency);
		}

		//Refresh dependency listbox button event
		System::Void btnRefdep_Click(System::Object^ sender, System::EventArgs^ e)
		{
			this->listDep->SelectedItems->Clear();
			SendMessage(Ider::MsgType::Dependency);
		}

		//Refresh checked-in listbox button event
		System::Void btnRefresh_Click(System::Object^ sender, System::EventArgs^ e) 
		{
			this->requestChickedin = true;
			SendMessage(Ider::MsgType::Checkin);
		}

		System::Collections::Generic::List<String^>^ selectionTrack;
		System::Boolean connected;
		System::Boolean requestChickedin;
		ICommunicator* sender;
		Ider::IMessageHandler* mh;
		MsgHandler<ClientMessage_Proc>* msgProc;
		FileHandler<ClientFile_Proc>* fileProc;

#pragma region Windows Form Controls

	private: System::Windows::Forms::Panel^  pnlLogin;

	private: System::Windows::Forms::FolderBrowserDialog^ folderDialog;
	private: System::Windows::Forms::OpenFileDialog^ fileDialog;

	private: System::Windows::Forms::TabControl^  tabClient;

	private: System::Windows::Forms::TabPage^  tabPackage;
	private: System::Windows::Forms::TabPage^  tabCheckin;

	private: System::Windows::Forms::ListBox^  listDep;
	private: System::Windows::Forms::ListBox^  listCheckin;

	private: System::Windows::Forms::Button^  btnDep;
	private: System::Windows::Forms::Button^  btnExt;
	private: System::Windows::Forms::Button^  btnLogin;
	private: System::Windows::Forms::Button^  btnClose;
	private: System::Windows::Forms::Button^  btnUpload;
	private: System::Windows::Forms::Button^  btnRefresh;
	private: System::Windows::Forms::Button^  btnRefdep;

	private: System::Windows::Forms::TextBox^  txtCitIP;
	private: System::Windows::Forms::TextBox^  txtCitPort;
	private: System::Windows::Forms::TextBox^  txtSvrPort;
	private: System::Windows::Forms::TextBox^  txtSvrIP;
	private: System::Windows::Forms::TextBox^  txtUsrName;
	private: System::Windows::Forms::TextBox^  txtDetail;

	private: System::Windows::Forms::Label^  lblCli;
	private: System::Windows::Forms::Label^  lblSvr;
	private: System::Windows::Forms::Label^  lblUsrName;
	private: System::Windows::Forms::Label^  lblCitColon;
	private: System::Windows::Forms::Label^  lblSvrColon;
	private: System::Windows::Forms::Label^  lblCheckin;
	private: System::Windows::Forms::Label^  lblPack;
	private: System::Windows::Forms::Label^  lblDetail;

	private: System::Windows::Forms::CheckBox^  cbAll;

#pragma endregion

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(UserInterface::typeid));
			this->tabClient = (gcnew System::Windows::Forms::TabControl());
			this->tabPackage = (gcnew System::Windows::Forms::TabPage());
			this->btnInfo2 = (gcnew System::Windows::Forms::Button());
			this->txtDetail = (gcnew System::Windows::Forms::TextBox());
			this->lblDetail = (gcnew System::Windows::Forms::Label());
			this->btnRefdep = (gcnew System::Windows::Forms::Button());
			this->lblPack = (gcnew System::Windows::Forms::Label());
			this->btnDep = (gcnew System::Windows::Forms::Button());
			this->btnExt = (gcnew System::Windows::Forms::Button());
			this->listDep = (gcnew System::Windows::Forms::ListBox());
			this->tabCheckin = (gcnew System::Windows::Forms::TabPage());
			this->btnInfo3 = (gcnew System::Windows::Forms::Button());
			this->cbAll = (gcnew System::Windows::Forms::CheckBox());
			this->btnRefresh = (gcnew System::Windows::Forms::Button());
			this->lblCheckin = (gcnew System::Windows::Forms::Label());
			this->btnClose = (gcnew System::Windows::Forms::Button());
			this->btnUpload = (gcnew System::Windows::Forms::Button());
			this->listCheckin = (gcnew System::Windows::Forms::ListBox());
			this->pnlLogin = (gcnew System::Windows::Forms::Panel());
			this->btnInfo1 = (gcnew System::Windows::Forms::Button());
			this->txtUsrName = (gcnew System::Windows::Forms::TextBox());
			this->lblUsrName = (gcnew System::Windows::Forms::Label());
			this->lblCitColon = (gcnew System::Windows::Forms::Label());
			this->lblSvrColon = (gcnew System::Windows::Forms::Label());
			this->lblCli = (gcnew System::Windows::Forms::Label());
			this->lblSvr = (gcnew System::Windows::Forms::Label());
			this->txtCitIP = (gcnew System::Windows::Forms::TextBox());
			this->txtCitPort = (gcnew System::Windows::Forms::TextBox());
			this->txtSvrPort = (gcnew System::Windows::Forms::TextBox());
			this->txtSvrIP = (gcnew System::Windows::Forms::TextBox());
			this->btnLogin = (gcnew System::Windows::Forms::Button());
			this->folderDialog = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->fileDialog = (gcnew System::Windows::Forms::OpenFileDialog());
			this->tabClient->SuspendLayout();
			this->tabPackage->SuspendLayout();
			this->tabCheckin->SuspendLayout();
			this->pnlLogin->SuspendLayout();
			this->SuspendLayout();
			// 
			// tabClient
			// 
			this->tabClient->Controls->Add(this->tabPackage);
			this->tabClient->Controls->Add(this->tabCheckin);
			this->tabClient->Font = (gcnew System::Drawing::Font(L"Georgia", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->tabClient->Location = System::Drawing::Point(0, 0);
			this->tabClient->Name = L"tabClient";
			this->tabClient->SelectedIndex = 0;
			this->tabClient->Size = System::Drawing::Size(580, 340);
			this->tabClient->TabIndex = 0;
			// 
			// tabPackage
			// 
			this->tabPackage->Controls->Add(this->btnInfo2);
			this->tabPackage->Controls->Add(this->txtDetail);
			this->tabPackage->Controls->Add(this->lblDetail);
			this->tabPackage->Controls->Add(this->btnRefdep);
			this->tabPackage->Controls->Add(this->lblPack);
			this->tabPackage->Controls->Add(this->btnDep);
			this->tabPackage->Controls->Add(this->btnExt);
			this->tabPackage->Controls->Add(this->listDep);
			this->tabPackage->Location = System::Drawing::Point(4, 24);
			this->tabPackage->Name = L"tabPackage";
			this->tabPackage->Padding = System::Windows::Forms::Padding(3);
			this->tabPackage->Size = System::Drawing::Size(572, 312);
			this->tabPackage->TabIndex = 0;
			this->tabPackage->Text = L"Packages";
			this->tabPackage->UseVisualStyleBackColor = true;
			// 
			// btnInfo2
			// 
			this->btnInfo2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnInfo2.Image")));
			this->btnInfo2->Location = System::Drawing::Point(540, 280);
			this->btnInfo2->Name = L"btnInfo2";
			this->btnInfo2->Size = System::Drawing::Size(25, 25);
			this->btnInfo2->TabIndex = 10;
			this->btnInfo2->UseVisualStyleBackColor = true;
			this->btnInfo2->Click += gcnew System::EventHandler(this, &UserInterface::btnInfo2_Click);
			// 
			// txtDetail
			// 
			this->txtDetail->Location = System::Drawing::Point(350, 90);
			this->txtDetail->Multiline = true;
			this->txtDetail->Name = L"txtDetail";
			this->txtDetail->ReadOnly = true;
			this->txtDetail->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->txtDetail->Size = System::Drawing::Size(215, 135);
			this->txtDetail->TabIndex = 9;
			// 
			// lblDetail
			// 
			this->lblDetail->AutoSize = true;
			this->lblDetail->Location = System::Drawing::Point(350, 71);
			this->lblDetail->Name = L"lblDetail";
			this->lblDetail->Size = System::Drawing::Size(96, 15);
			this->lblDetail->TabIndex = 8;
			this->lblDetail->Text = L"Pakcage Detail:";
			// 
			// btnRefdep
			// 
			this->btnRefdep->Font = (gcnew System::Drawing::Font(L"Georgia", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->btnRefdep->Location = System::Drawing::Point(350, 260);
			this->btnRefdep->Name = L"btnRefdep";
			this->btnRefdep->Size = System::Drawing::Size(151, 23);
			this->btnRefdep->TabIndex = 7;
			this->btnRefdep->Text = L"Refresh List";
			this->btnRefdep->UseVisualStyleBackColor = true;
			this->btnRefdep->Click += gcnew System::EventHandler(this, &UserInterface::btnRefdep_Click);
			// 
			// lblPack
			// 
			this->lblPack->AutoSize = true;
			this->lblPack->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblPack->Location = System::Drawing::Point(22, 11);
			this->lblPack->Name = L"lblPack";
			this->lblPack->Size = System::Drawing::Size(78, 18);
			this->lblPack->TabIndex = 6;
			this->lblPack->Text = L"Packages:";
			// 
			// btnDep
			// 
			this->btnDep->Font = (gcnew System::Drawing::Font(L"Georgia", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->btnDep->Location = System::Drawing::Point(350, 40);
			this->btnDep->Name = L"btnDep";
			this->btnDep->Size = System::Drawing::Size(151, 23);
			this->btnDep->TabIndex = 4;
			this->btnDep->Text = L"View Package Detail";
			this->btnDep->UseVisualStyleBackColor = true;
			this->btnDep->Click += gcnew System::EventHandler(this, &UserInterface::btnDep_Click);
			// 
			// btnExt
			// 
			this->btnExt->Font = (gcnew System::Drawing::Font(L"Georgia", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->btnExt->Location = System::Drawing::Point(350, 232);
			this->btnExt->Name = L"btnExt";
			this->btnExt->Size = System::Drawing::Size(151, 23);
			this->btnExt->TabIndex = 3;
			this->btnExt->Text = L"Extract File";
			this->btnExt->UseVisualStyleBackColor = true;
			this->btnExt->Click += gcnew System::EventHandler(this, &UserInterface::btnExt_Click);
			// 
			// listDep
			// 
			this->listDep->Font = (gcnew System::Drawing::Font(L"Georgia", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->listDep->FormattingEnabled = true;
			this->listDep->ItemHeight = 15;
			this->listDep->Items->AddRange(gcnew cli::array< System::Object^  >(2) {L"..", L"*.*"});
			this->listDep->Location = System::Drawing::Point(20, 40);
			this->listDep->Name = L"listDep";
			this->listDep->Size = System::Drawing::Size(292, 244);
			this->listDep->TabIndex = 0;
			this->listDep->DoubleClick += gcnew System::EventHandler(this, &UserInterface::listDep_DoubleClick);
			// 
			// tabCheckin
			// 
			this->tabCheckin->Controls->Add(this->btnInfo3);
			this->tabCheckin->Controls->Add(this->cbAll);
			this->tabCheckin->Controls->Add(this->btnRefresh);
			this->tabCheckin->Controls->Add(this->lblCheckin);
			this->tabCheckin->Controls->Add(this->btnClose);
			this->tabCheckin->Controls->Add(this->btnUpload);
			this->tabCheckin->Controls->Add(this->listCheckin);
			this->tabCheckin->Location = System::Drawing::Point(4, 24);
			this->tabCheckin->Name = L"tabCheckin";
			this->tabCheckin->Padding = System::Windows::Forms::Padding(3);
			this->tabCheckin->Size = System::Drawing::Size(572, 312);
			this->tabCheckin->TabIndex = 1;
			this->tabCheckin->Text = L"Checkin";
			this->tabCheckin->UseVisualStyleBackColor = true;
			// 
			// btnInfo3
			// 
			this->btnInfo3->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnInfo3.Image")));
			this->btnInfo3->Location = System::Drawing::Point(541, 8);
			this->btnInfo3->Name = L"btnInfo3";
			this->btnInfo3->Size = System::Drawing::Size(25, 25);
			this->btnInfo3->TabIndex = 11;
			this->btnInfo3->UseVisualStyleBackColor = true;
			this->btnInfo3->Click += gcnew System::EventHandler(this, &UserInterface::btnInfo3_Click);
			// 
			// cbAll
			// 
			this->cbAll->AutoSize = true;
			this->cbAll->Location = System::Drawing::Point(426, 189);
			this->cbAll->Name = L"cbAll";
			this->cbAll->Size = System::Drawing::Size(75, 19);
			this->cbAll->TabIndex = 7;
			this->cbAll->Text = L"Close All";
			this->cbAll->UseVisualStyleBackColor = true;
			// 
			// btnRefresh
			// 
			this->btnRefresh->Font = (gcnew System::Drawing::Font(L"Georgia", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->btnRefresh->Location = System::Drawing::Point(350, 260);
			this->btnRefresh->Name = L"btnRefresh";
			this->btnRefresh->Size = System::Drawing::Size(151, 23);
			this->btnRefresh->TabIndex = 6;
			this->btnRefresh->Text = L"Refresh List";
			this->btnRefresh->UseVisualStyleBackColor = true;
			this->btnRefresh->Click += gcnew System::EventHandler(this, &UserInterface::btnRefresh_Click);
			// 
			// lblCheckin
			// 
			this->lblCheckin->AutoSize = true;
			this->lblCheckin->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblCheckin->Location = System::Drawing::Point(20, 10);
			this->lblCheckin->Name = L"lblCheckin";
			this->lblCheckin->Size = System::Drawing::Size(230, 18);
			this->lblCheckin->TabIndex = 5;
			this->lblCheckin->Text = L"Unclosed Checked In Packages:";
			// 
			// btnClose
			// 
			this->btnClose->Font = (gcnew System::Drawing::Font(L"Georgia", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->btnClose->Location = System::Drawing::Point(350, 160);
			this->btnClose->Name = L"btnClose";
			this->btnClose->Size = System::Drawing::Size(151, 23);
			this->btnClose->TabIndex = 3;
			this->btnClose->Text = L"Close Package";
			this->btnClose->UseVisualStyleBackColor = true;
			this->btnClose->Click += gcnew System::EventHandler(this, &UserInterface::btnClose_Click);
			// 
			// btnUpload
			// 
			this->btnUpload->Font = (gcnew System::Drawing::Font(L"Georgia", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->btnUpload->Location = System::Drawing::Point(350, 40);
			this->btnUpload->Name = L"btnUpload";
			this->btnUpload->Size = System::Drawing::Size(151, 23);
			this->btnUpload->TabIndex = 4;
			this->btnUpload->Text = L"Upload Files";
			this->btnUpload->UseVisualStyleBackColor = true;
			this->btnUpload->Click += gcnew System::EventHandler(this, &UserInterface::btnUpload_Click);
			// 
			// listCheckin
			// 
			this->listCheckin->Font = (gcnew System::Drawing::Font(L"Georgia", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->listCheckin->FormattingEnabled = true;
			this->listCheckin->ItemHeight = 15;
			this->listCheckin->Items->AddRange(gcnew cli::array< System::Object^  >(2) {L"..", L"*.*"});
			this->listCheckin->Location = System::Drawing::Point(20, 40);
			this->listCheckin->Name = L"listCheckin";
			this->listCheckin->Size = System::Drawing::Size(292, 244);
			this->listCheckin->TabIndex = 0;
			// 
			// pnlLogin
			// 
			this->pnlLogin->Controls->Add(this->btnInfo1);
			this->pnlLogin->Controls->Add(this->txtUsrName);
			this->pnlLogin->Controls->Add(this->lblUsrName);
			this->pnlLogin->Controls->Add(this->lblCitColon);
			this->pnlLogin->Controls->Add(this->lblSvrColon);
			this->pnlLogin->Controls->Add(this->lblCli);
			this->pnlLogin->Controls->Add(this->lblSvr);
			this->pnlLogin->Controls->Add(this->txtCitIP);
			this->pnlLogin->Controls->Add(this->txtCitPort);
			this->pnlLogin->Controls->Add(this->txtSvrPort);
			this->pnlLogin->Controls->Add(this->txtSvrIP);
			this->pnlLogin->Controls->Add(this->btnLogin);
			this->pnlLogin->Location = System::Drawing::Point(0, 0);
			this->pnlLogin->Name = L"pnlLogin";
			this->pnlLogin->Size = System::Drawing::Size(580, 360);
			this->pnlLogin->TabIndex = 1;
			// 
			// btnInfo1
			// 
			this->btnInfo1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnInfo1.Image")));
			this->btnInfo1->Location = System::Drawing::Point(529, 315);
			this->btnInfo1->Name = L"btnInfo1";
			this->btnInfo1->Size = System::Drawing::Size(25, 25);
			this->btnInfo1->TabIndex = 12;
			this->btnInfo1->UseVisualStyleBackColor = true;
			this->btnInfo1->Click += gcnew System::EventHandler(this, &UserInterface::btnInfo1_Click);
			// 
			// txtUsrName
			// 
			this->txtUsrName->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->txtUsrName->Location = System::Drawing::Point(200, 200);
			this->txtUsrName->MaxLength = 100;
			this->txtUsrName->Name = L"txtUsrName";
			this->txtUsrName->Size = System::Drawing::Size(165, 26);
			this->txtUsrName->TabIndex = 10;
			this->txtUsrName->Text = L"Ider";
			// 
			// lblUsrName
			// 
			this->lblUsrName->AutoSize = true;
			this->lblUsrName->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblUsrName->Location = System::Drawing::Point(40, 200);
			this->lblUsrName->Name = L"lblUsrName";
			this->lblUsrName->Size = System::Drawing::Size(93, 18);
			this->lblUsrName->TabIndex = 9;
			this->lblUsrName->Text = L"User Name:";
			// 
			// lblCitColon
			// 
			this->lblCitColon->AutoSize = true;
			this->lblCitColon->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblCitColon->Location = System::Drawing::Point(300, 140);
			this->lblCitColon->Name = L"lblCitColon";
			this->lblCitColon->Size = System::Drawing::Size(13, 18);
			this->lblCitColon->TabIndex = 8;
			this->lblCitColon->Text = L":";
			// 
			// lblSvrColon
			// 
			this->lblSvrColon->AutoSize = true;
			this->lblSvrColon->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblSvrColon->Location = System::Drawing::Point(300, 70);
			this->lblSvrColon->Name = L"lblSvrColon";
			this->lblSvrColon->Size = System::Drawing::Size(13, 18);
			this->lblSvrColon->TabIndex = 7;
			this->lblSvrColon->Text = L":";
			// 
			// lblCli
			// 
			this->lblCli->AutoSize = true;
			this->lblCli->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblCli->Location = System::Drawing::Point(40, 140);
			this->lblCli->Name = L"lblCli";
			this->lblCli->Size = System::Drawing::Size(137, 18);
			this->lblCli->TabIndex = 6;
			this->lblCli->Text = L"Client IP Address:";
			// 
			// lblSvr
			// 
			this->lblSvr->AutoSize = true;
			this->lblSvr->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->lblSvr->Location = System::Drawing::Point(40, 70);
			this->lblSvr->Name = L"lblSvr";
			this->lblSvr->Size = System::Drawing::Size(144, 18);
			this->lblSvr->TabIndex = 5;
			this->lblSvr->Text = L"Server IP Address:";
			// 
			// txtCitIP
			// 
			this->txtCitIP->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->txtCitIP->Location = System::Drawing::Point(200, 140);
			this->txtCitIP->Name = L"txtCitIP";
			this->txtCitIP->ReadOnly = true;
			this->txtCitIP->Size = System::Drawing::Size(100, 26);
			this->txtCitIP->TabIndex = 4;
			this->txtCitIP->Text = L"127.0.0.1";
			// 
			// txtCitPort
			// 
			this->txtCitPort->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->txtCitPort->Location = System::Drawing::Point(315, 140);
			this->txtCitPort->Name = L"txtCitPort";
			this->txtCitPort->Size = System::Drawing::Size(52, 26);
			this->txtCitPort->TabIndex = 3;
			this->txtCitPort->Text = L"0311";
			// 
			// txtSvrPort
			// 
			this->txtSvrPort->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->txtSvrPort->Location = System::Drawing::Point(315, 70);
			this->txtSvrPort->Name = L"txtSvrPort";
			this->txtSvrPort->ReadOnly = true;
			this->txtSvrPort->Size = System::Drawing::Size(50, 26);
			this->txtSvrPort->TabIndex = 2;
			this->txtSvrPort->Text = L"2107";
			// 
			// txtSvrIP
			// 
			this->txtSvrIP->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->txtSvrIP->Location = System::Drawing::Point(200, 70);
			this->txtSvrIP->Name = L"txtSvrIP";
			this->txtSvrIP->ReadOnly = true;
			this->txtSvrIP->Size = System::Drawing::Size(100, 26);
			this->txtSvrIP->TabIndex = 1;
			this->txtSvrIP->Text = L"127.0.0.1";
			// 
			// btnLogin
			// 
			this->btnLogin->Font = (gcnew System::Drawing::Font(L"Georgia", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->btnLogin->Location = System::Drawing::Point(384, 196);
			this->btnLogin->Name = L"btnLogin";
			this->btnLogin->Size = System::Drawing::Size(100, 26);
			this->btnLogin->TabIndex = 0;
			this->btnLogin->Text = L"Log In";
			this->btnLogin->UseVisualStyleBackColor = true;
			this->btnLogin->Click += gcnew System::EventHandler(this, &UserInterface::btnLogin_Click);
			// 
			// fileDialog
			// 
			this->fileDialog->Filter = L"Package Files(*.cpp, *.h)|*.cpp;*.h|Header Files(*.h)|*.h|Implement Files(*.cpp)|" 
				L"*.cpp";
			this->fileDialog->Multiselect = true;
			// 
			// UserInterface
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(584, 362);
			this->Controls->Add(this->pnlLogin);
			this->Controls->Add(this->tabClient);
			this->Name = L"UserInterface";
			this->Text = L"UI";
			this->tabClient->ResumeLayout(false);
			this->tabPackage->ResumeLayout(false);
			this->tabPackage->PerformLayout();
			this->tabCheckin->ResumeLayout(false);
			this->tabCheckin->PerformLayout();
			this->pnlLogin->ResumeLayout(false);
			this->pnlLogin->PerformLayout();
			this->ResumeLayout(false);

		}

		/*this->Controls->Add(this->pnlLogin);*/


#pragma endregion

	private: System::Windows::Forms::Button^  btnInfo1;
	private: System::Windows::Forms::Button^  btnInfo2;
	private: System::Windows::Forms::Button^  btnInfo3;
	private:

		System::Void btnInfo1_Click(System::Object^ sender, System::EventArgs^ e)
		{
			System::String^ info = "Hello, do you looking for register? \n"
				+"Sorry, no such functionality.\n"
				+"You can use follow user name to log in:\n"
				+"Ider\nAdmin\nJim Fawcett\nHello World\nC++\n\n"
				+"No password required.\n\n"
				+"When you log in, no way to log out, think carefully.";

				ShowMessageBox(info);
		}
		System::Void btnInfo2_Click(System::Object^ sender, System::EventArgs^ e) 
		{
			System::String^ info = "You can do following actions:\n"
				+"Double click to navigate the dependencies of package;\n"
				+"View button could see the detail information of package;\n"
				+"Extract button allow you to download a package from server;\n"
				+"Refresh button would refresh the package list.";

			ShowMessageBox(info);

		}
		System::Void btnInfo3_Click(System::Object^ sender, System::EventArgs^ e)
		{
			System::String^ info = "So happy you clicked it, I change the position of button\n"
				+ "just want you know what I have done here. :-)\n\n"
				+ "You can do following actions:\n"
				+ "Upload button allow you to upload packages to server(.h,.cpp files only);\n"
				+ "Close button let you close specific packages;\n"
				+"Refresh button would refresh the package list.";

			ShowMessageBox(info);
		}
	};
}


#endif
