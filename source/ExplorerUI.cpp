#include <SDL.h>
#include <UI.h>
#include <string>
#include <switch.h>
#include <Utils.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
using namespace std;

class ExplorerUI : public UIWindow
{
	private:
	//vars
	int HeaderHeight = 0;
	public:
	//vars
	ScrollList *FileList;
	vector <dirent> Files = vector <dirent>(0);
	string HighlightedPath = "";
	string *ChosenFile;
	string DirPath = "sdmc:/";
	//functions
	ExplorerUI();
	void GetInput();
	void DrawUI();
	void OpenFile(string);
	void LoadListDirs(string);
};

ExplorerUI::ExplorerUI()
{
	//Set up shared font
	PlFontData standardFontData;
	plGetSharedFontByType(&standardFontData, PlSharedFontType_Standard);
	//Get a list of the files
	FileList = new ScrollList();
	//Set up the file list
	//FileList->TouchListX = &TouchX;
	//FileList->TouchListY = &TouchY;
	FileList->ListFont = GetSharedFont(32); //Load the list font
	FileList->ListingsOnScreen = 10;
	FileList->ListHeight = Height - HeaderHeight;
	FileList->ListWidth = Width;
	FileList->ListYOffset = HeaderHeight;
	FileList->IsActive = true;
	
	//Populate list
	LoadListDirs(DirPath);
}

void ExplorerUI::GetInput()
{
	//ScanInput
	while (SDL_PollEvent(Event))
	{
		 switch (Event->type)
		 {
			 //Joycon button down
			  case SDL_JOYBUTTONDOWN:
			  {
				  if (Event->jbutton.which == 0)
				  {
					  //Plus or Y pressed
					  if(Event->jbutton.button == 10 || Event->jbutton.button == 3)
					  {
						  //Update highlighted file if one exists
						  if(!FileList->ListingTextVec.empty())
						  {
							  HighlightedPath = DirPath + Files.at(FileList->SelectedIndex).d_name;
						  }
						  else
						  {
							  HighlightedPath = "";
						  }
						  *WindowState = 1;
					  }
					  //Up pressed
					  else if(Event->jbutton.button == 13)
					  {
						  FileList->CursorIndex--;
						  FileList->SelectedIndex--;
					  }
					  //Down pressed
					  else if(Event->jbutton.button == 15)
					  {
						  FileList->CursorIndex++;
						  FileList->SelectedIndex++;
					  }
					  //A pressed
					  else if(Event->jbutton.button == 0)
					  {
						  //Check if directory. If not open file.
						  string FilePath = DirPath + Files.at(FileList->SelectedIndex).d_name;
						  if(CheckIsDir(FilePath))
						  {
								DirPath = FilePath + "/";
								LoadListDirs(DirPath);
						  }
						  else
						  {
								OpenFile(DirPath + Files.at(FileList->SelectedIndex).d_name);
						  }
					  }
					  //B pressed
					  else if(Event->jbutton.button == 1)
					  {
						  DirPath = GoUpDir(DirPath) + "/";
						  LoadListDirs(DirPath);
						  //Reset the cursor
						  FileList->SelectedIndex = 0;
						  FileList->CursorIndex = 0;
					  }
				  }
			  }
			  break;
		 }
	}
}

void ExplorerUI::DrawUI()
{
	//Draw the BG
	SDL_SetRenderDrawColor(Renderer, 94, 94, 94, 255);
	SDL_Rect BGRect = {0,0, Width, Height};
	SDL_RenderFillRect(Renderer, &BGRect);
	//Draw the list
	FileList->DrawList();
	//Update file list
	//LoadListDirs(DirPath);
}

void ExplorerUI::OpenFile(string Path)
{
	int ExtensionStart = Path.find(".")+1;
	string FileSuffix;
	FileSuffix.assign(Path, ExtensionStart, Path.size() - ExtensionStart);
	if(FileSuffix == "png" || FileSuffix == "jpg" || FileSuffix == "jpeg" || FileSuffix == "bmp")
	{
		*WindowState = 3;
		*ChosenFile = Path;
	}
	else if(FileSuffix == "mp3" || FileSuffix == "wav")
	{
		*WindowState = 4;
		*ChosenFile = Path;
	}
	else if(FileSuffix == "txt" || FileSuffix == "ini" || FileSuffix == "json" || FileSuffix == "plist" || FileSuffix == "cfg" || FileSuffix == "log")
	{
		*WindowState = 2;
		*ChosenFile = Path;
	}
}

void ExplorerUI::LoadListDirs(string DirPath)
{
	FileList->ListingTextVec.clear();
	Files = LoadDirs(DirPath);
	for(int i = 0; i < Files.size(); i++)
	{
		FileList->ListingTextVec.push_back(Files.at(i).d_name);
	}
}

class MenuUI : public UIWindow
{
	private:
	//vars
	string ClipboardPath = "";
	string ClipboardFileName = "";
	public:
	//vars
	ScrollList *MenuList;
	ExplorerUI *Explorer;
	//Functions
	MenuUI();
	void GetInput();
	void DrawUI();
};

MenuUI::MenuUI()
{
	//Init the list
	MenuList = new ScrollList();
	MenuList->ListFont = GetSharedFont(32); //Load the list font
	MenuList->ListHeight = Height / 2;
	MenuList->ListWidth = Width / 6;
	MenuList->ListXOffset = Width - MenuList->ListWidth;
	MenuList->ListYOffset = (Height - MenuList->ListHeight) / 2;
	MenuList->IsActive = true;
	MenuList->CenterText = true;
	//populate list
	MenuList->ListingTextVec.push_back("Copy");
	MenuList->ListingTextVec.push_back("Paste");
	MenuList->ListingTextVec.push_back("Move");
	MenuList->ListingTextVec.push_back("Rename");
	MenuList->ListingTextVec.push_back("New folder");
	MenuList->ListingTextVec.push_back("New file");
	MenuList->ListingTextVec.push_back("Delete");
	MenuList->ListingTextVec.push_back("Exit");
	MenuList->ListingsOnScreen = MenuList->ListingTextVec.size();
}

void MenuUI::DrawUI()
{
	//Draw the bg rectangle
	SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
	SDL_Rect BGRect = {MenuList->ListXOffset - 3, MenuList->ListYOffset , MenuList->ListWidth , MenuList->ListHeight + 3};
	SDL_RenderFillRect(Renderer, &BGRect);
	//Draw the list
	MenuList->DrawList();
}

void MenuUI::GetInput()
{
	//ScanInput
	while (SDL_PollEvent(Event))
	{
		switch (Event->type)
		{
			//Joycon button down
			case SDL_JOYBUTTONDOWN:
			{
				if (Event->jbutton.which == 0)
				{
					//Plus or B pressed
					if(Event->jbutton.button == 10 || Event->jbutton.button == 1)
					{
					  *WindowState = 0;
					}
					//Up pressed
					else if(Event->jbutton.button == 13)
					{
					  MenuList->CursorIndex--;
					  MenuList->SelectedIndex--;
					}
					//Down pressed
					else if(Event->jbutton.button == 15)
					{
					  MenuList->CursorIndex++;
					  MenuList->SelectedIndex++;
					}
					//A pressed
					else if(Event->jbutton.button == 0)
					{
						switch(MenuList->SelectedIndex)
						{
							//Copy
							case 0:
							{
								//Only copy the file if the file exists
								if(!Explorer->FileList->ListingTextVec.empty())
								{
									ClipboardPath = Explorer->HighlightedPath.c_str();
									ClipboardFileName = Explorer->FileList->ListingTextVec.at(Explorer->FileList->SelectedIndex);
								}
							}
							break;
							//paste
							case 1:
							{
							  //Not implemented
							}
							break;
							//Move
							case 2:
							{
								string PathToMoveTo = Explorer->DirPath + ClipboardFileName;
								rename(ClipboardPath.c_str(), PathToMoveTo.c_str());
							}
							break;
							//Rename
							case 3:
							{
								//Make sure the dir isn't empty
								if(!Explorer->FileList->ListingTextVec.empty())
								{
								  	string ExistingFileName = Explorer->FileList->ListingTextVec.at(Explorer->FileList->SelectedIndex);
									string NewFileName = Explorer->DirPath + GetKeyboardInput("Rename", "Rename " + ExistingFileName, ExistingFileName);
									rename(Explorer->HighlightedPath.c_str(), NewFileName.c_str());
								}
							}
							break;
							//New dir
							case 4:
							{
								string NewDirName = Explorer->DirPath + GetKeyboardInput("New folder", "Enter folder name", "New Folder");
								mkdir(NewDirName.c_str(), 0);
							}
							break;
							//New file
							case 5:
							{
								string NewFileName = Explorer->DirPath + GetKeyboardInput("New file", "Enter file name", "New File");
								if(!CheckFileExists(NewFileName))
								{
									ofstream outfile (NewFileName.c_str());
									outfile.close();
								}
							}
							break;
							//Delete
							case 6:
							{
								//Don't nuke the /switch/ directory if the user presses delete in an empty dir
								if(Explorer->HighlightedPath.empty()) break;
								//Check if directory
								if(CheckIsDir(Explorer->HighlightedPath.c_str()))
								{
									//Recursive delete function provided by libnx
									fsdevDeleteDirectoryRecursively(Explorer->HighlightedPath.c_str());
								}
								else
								{
									//Remove individual file with standard C++ function
									remove(Explorer->HighlightedPath.c_str());
								}
							}
							break;
							//Exit
							case 7:
							{
								*IsDone = 1;
							}
						  break;
						}
						//Return to explorer and reload the dir list
						*WindowState = 0;
						Explorer->LoadListDirs(Explorer->DirPath);
					  }
				  }
			  }
			  break;
		}
	}	
}