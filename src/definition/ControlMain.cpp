#include "../header/ControlMain.h"
#include "../header/ModelDAOInterface.h"
#include "../header/ViewGuiBuilderInterface.h"
#include "../header/ContainerException.h"
#include "../header/ContainerFileInfo.h"
#include "../header/InnerConfig.h"


#include <string>
#include <vector>
#include <list>
#include <algorithm>
using namespace FtpClient;

ControlMain::ControlMain(ViewGuiBuilderInterface* viewGuiBuilderObject, ModelDAOInterface* modelDAOObject, InnerConfig* innerConfigObject):viewGuiBuilderObject(viewGuiBuilderObject), modelDAOObject(modelDAOObject), innerConfigObject(innerConfigObject), exceptionManagerObject(viewGuiBuilderObject, innerConfigObject){
	this->localFilesList = NULL;
}

void ControlMain::startFtpClient(void) {
	this->viewGuiBuilderObject->initializeMainWindow();
	this->viewGuiBuilderObject->bindMainWindowEvents(this);
	this->initLocalBrowser("F:/test");
	try {
		this->currentConnectionID = this->modelDAOObject->createNewConnection("ftp-bujnyj.ogicom.pl", "21", "ftpclienttest.bujnyj", "Test1234");
	} catch (ContainerException &e) {
		this->viewGuiBuilderObject->spawnExceptionWindow("Error", e.level);
	}
	try {
		std::list<ContainerFileInfo>* filesList = this->modelDAOObject->serverGetDirectoryContent("/", this->currentConnectionID);
		this->serverFilesList = filesList;
		this->viewGuiBuilderObject->showListInServerTree(filesList);
		//this->viewGuiBuilderObject->showListInLocalTree();
	} catch (ContainerException &e) {
		this->viewGuiBuilderObject->spawnExceptionWindow("Error2", e.level);
	}
	//this->viewGuiBuilderObject->spawnExceptionWindow("ERROR!", ExceptionLevel::EXCEPTIONLEVEL_CRITICAL);
	//this->viewGuiBuilderObject->spawnConnectWindow();

}


void ControlMain::initLocalBrowser(std::string startPath) {
	if(startPath.compare("/") == 0 || startPath.compare("\\") == 0 || startPath.compare(" ") == 0 ) {
		this->localFilesList = this->modelDAOObject->getLogicalDrives();
	} else {
		this->localFilesList = this->modelDAOObject->getDirectoryContent(startPath);
	}
	this->viewGuiBuilderObject->showListInLocalTree(this->localFilesList);
}



void ControlMain::connectWindowButtonConnectClicked(std::string host, std::string port, std::string login, std::string password) {

}

void ControlMain::localTreeCellDoubleClick(std::string name) {
	if(name.compare("..") == 0) {
		ContainerFileInfo fileObject = (*this->localFilesList).front();
		if(this->modelDAOObject->isPathLogicalPartition(fileObject.filePath)) {
			this->localFilesList = this->modelDAOObject->getLogicalDrives();
		} else {

			try {
				this->localFilesList = this->modelDAOObject->getDirectoryContent(this->modelDAOObject->goUpInDirPath(fileObject.filePath));
			} catch (ContainerException &e) {
				this->exceptionManagerObject.manageException(e);
			}
		}
	} else {
		ContainerFileInfo fileObject = (*this->localFilesList).front();
		for (std::list<ContainerFileInfo>::iterator it=(*this->localFilesList).begin(); it != (*this->localFilesList).end(); ++it){
			if(it->fileName.compare(name) == 0) {
				fileObject = *it;
				break;
			}
		}
		if(fileObject.isDir) {
			if(fileObject.filePath.size() == 2) {
				try {
					this->localFilesList = this->modelDAOObject->getDirectoryContent(fileObject.filePath);
				} catch (ContainerException &e) {
					this->exceptionManagerObject.manageException(e);
				}
			} else {
				try {
					this->localFilesList = this->modelDAOObject->getDirectoryContent((fileObject.filePath + fileObject.fileName));
				} catch (ContainerException &e) {
					this->exceptionManagerObject.manageException(e);
				}
			}
		} else {
			//TODO
		}

	}
	this->localFilesList = this->modelDAOObject->orderFilesListDirecrotiesFiles(this->localFilesList);
	this->viewGuiBuilderObject->showListInLocalTree(this->localFilesList);
}


void ControlMain::serverTreeCellDoubleClick(std::string name) {
	if(name.compare("..") == 0) {
		ContainerFileInfo fileObject = (*this->serverFilesList).front();
		try {
			this->serverFilesList = this->modelDAOObject->serverGetDirectoryContent(this->modelDAOObject->goUpInDirPath(fileObject.filePath), this->currentConnectionID);
		} catch (ContainerException &e) {
			this->exceptionManagerObject.manageException(e);
		}
	} else {
		ContainerFileInfo fileObject = (*this->serverFilesList).front();
		for (std::list<ContainerFileInfo>::iterator it=(*this->serverFilesList).begin(); it != (*this->serverFilesList).end(); ++it){
			if(it->fileName.compare(name) == 0) {
				fileObject = *it;
				break;
			}
		}
		if(fileObject.isDir) {
			try {
				this->serverFilesList = this->modelDAOObject->serverGetDirectoryContent((fileObject.filePath + fileObject.fileName + "/"), this->currentConnectionID);
			} catch (ContainerException &e) {
				this->exceptionManagerObject.manageException(e);
			}
		} else {
			//TODO
		}

	}
	this->viewGuiBuilderObject->showListInServerTree(this->serverFilesList);
}


void ControlMain::serverDeleteButton(ContainerFileInfo* file) {

}

void ControlMain::localDeleteButton(ContainerFileInfo* file) {
	if(this->viewGuiBuilderObject->spawnAreYouSureWindow()) {
		if(this->modelDAOObject->deleteLocalFile(file)) {
			try {
				this->localFilesList = this->modelDAOObject->getDirectoryContent(file->filePath);
			} catch (ContainerException &e) {
				this->exceptionManagerObject.manageException(e);
			}
			this->localFilesList = this->modelDAOObject->orderFilesListDirecrotiesFiles(this->localFilesList);
			this->viewGuiBuilderObject->showListInLocalTree(this->localFilesList);
		} else {
			if(file->isDir) {
				this->viewGuiBuilderObject->spawnExceptionWindow("Directory inaccessible or not empty!", ExceptionLevel::EXCEPTIONLEVEL_STANDARD);
			} else {
				this->viewGuiBuilderObject->spawnExceptionWindow("You cannot delete this file!", ExceptionLevel::EXCEPTIONLEVEL_STANDARD);
			}
		}
	}
}

ControlMain::~ControlMain(void){
}
