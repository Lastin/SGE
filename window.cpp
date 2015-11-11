#include <wx/wx.h>
#include "window.h"
#include <wx/image.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <wx/dc.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <wx/numdlg.h>
#include <wx/choicdlg.h>
#include <wx/textdlg.h>
#include <stdlib.h>
#include <fstream>
using namespace std;

static const wxChar *FILETYPES = _T("All files (*.*)|*.*|BMP files (*.bmp)|*.bmp");

IMPLEMENT_APP(BasicApplication)

bool BasicApplication::OnInit()
{
  wxInitAllImageHandlers();
  MyFrame *frame = new MyFrame(_("MakEditor"), 50, 50, 700, 400);
  frame->Show(TRUE);
  SetTopWindow(frame);
  return TRUE;
}

MyFrame::MyFrame(const wxString title, int xpos, int ypos, int width, int height): wxFrame((wxFrame *) NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height)){

  fileMenu = new wxMenu;
  fileMenu->Append(LOAD_FILE_ID, _T("&Open file\tCTRL-O"));
  fileMenu->Append(READRAW, _T("Open raw file\tCTRL-ALT-O"));
  fileMenu->Append(UNDO, _T("Undo\tCTRL-Z"));
  fileMenu->AppendSeparator();
//###########################################################//
//----------------------START MY MENU -----------------------//
//###########################################################//
  fileMenu->AppendSeparator();
  fileMenu->Append(INVERT_IMAGE_ID, _T("&Invert image"));
  fileMenu->Append(SCALE_IMAGE_ID, _T("&Scale image"));
  fileMenu->Append(SHIFT_IMAGE, _T("S&hift Image"));
  fileMenu->AppendSeparator();
  fileMenu->Append(CONVOLUTION, _T("&Convolution"));
  fileMenu->Append(GRAYSCALE, _T("&Grayscale"));
  fileMenu->AppendSeparator();
  fileMenu->Append(OSF, _T("Order Statistics &Filtering"));
  fileMenu->Append(LOGARITHMIC, _T("&Logarithmic function"));
  fileMenu->AppendSeparator();
  fileMenu->Append(POWERLAW, _T("&Power-law"));
  fileMenu->Append(RANDOMLOOKUP, _T("&Random lookup"));
  fileMenu->AppendSeparator();
  fileMenu->Append(EQUALISE, _T("&Equalise"));
  fileMenu->AppendSeparator();
  fileMenu->Append(SIMPLETHRESHOLDING, _T("Simple thresholding"));
  fileMenu->Append(AUTOMATEDTHRESHOLDING, _T("Automated thresholding"));


//###########################################################//
//----------------------END MY MENU -------------------------//
//###########################################################//

  fileMenu->AppendSeparator();
  fileMenu->Append(SAVE_IMAGE_ID, _T("&Save image"));
  fileMenu->Append(EXIT_ID, _T("E&xit"));

  menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, _T("&File"));

  SetMenuBar(menuBar);
  statusBar = CreateStatusBar(1);
  statusBar->PushStatusText(_T("Ready"));
  oldWidth = 0;
  oldHeight = 0;
  loadedImage = 0;

/* initialise the variables that we added */
  imgWidth = imgHeight = 0;
  stuffToDraw = 0;
}

MyFrame::~MyFrame(){
/* release resources */
  if(loadedImage){
    loadedImage->Destroy();
    loadedImage = 0;
  }

}

void MyFrame::OnOpenFile(wxCommandEvent & event){
  wxFileDialog *openFileDialog = new wxFileDialog (this, _T("Open file"), _T(""), _T(""), FILETYPES, wxOPEN, wxDefaultPosition);
  if(openFileDialog->ShowModal() == wxID_OK){
    changes.clear();
    undoing = FALSE;
    wxString filename = openFileDialog->GetFilename();
    wxString path = openFileDialog->GetPath();
    statusBar->PushStatusText(_T("Loading image form file..."));

    loadedImage = new wxImage(path); //Image Loaded form file
    if(loadedImage->Ok()){
      stuffToDraw = ORIGINAL_IMG;    // set the display flag
      statusBar->PopStatusText();
    }
    else {
      statusBar->PopStatusText();
      statusBar->PushStatusText(_T("Error, file could not be loaded"));
      sleep(1);
      statusBar->PopStatusText();
      loadedImage->Destroy();
      loadedImage = 0;
    }
    Refresh();
  }
}


//###########################################################//
//-----------------------------------------------------------//
//---------- DO NOT MODIFY THE CODE ABOVE--------------------//
//-----------------------------------------------------------//
//###########################################################//

void MyFrame::MouseDown(wxMouseEvent & event) {
  SetFocus();
  if(stuffToDraw == NOTHING) return;
  CaptureMouse();
  rect.SetPosition(event.GetPosition());
  rect.SetSize(wxSize(0, 0));
  x = event.GetX();
  y = event.GetY();
  undoing = TRUE;
  Refresh();
}
void MyFrame::MouseUp(wxMouseEvent & event) {
  ReleaseMouse();
}
void MyFrame::Motion(wxMouseEvent & event) {
  if(HasCapture()){
    int xDiff = event.GetX()-x;
    int yDiff = event.GetY()-y;
    rect.SetWidth(abs(xDiff));
    rect.SetHeight(abs(yDiff));
    if(xDiff < 0)
      rect.SetX(x + xDiff);
    if(yDiff < 0)
      rect.SetY(y + yDiff);
    undoing = TRUE;
    Refresh();
  }
}

int MyFrame::GetStartX() {
  return rect.IsEmpty() ? 0 : max(0, rect.GetX());
}

int MyFrame::GetStartY() {
  return rect.IsEmpty() ? 0 : max(0, rect.GetY());
}

int MyFrame::GetEndX() {
  return rect.IsEmpty() ? imgWidth : min(imgWidth, rect.GetX() + rect.GetWidth());
}

int MyFrame::GetEndY() {
  return rect.IsEmpty() ? imgHeight : min(imgHeight, rect.GetY() + rect.GetHeight());
}

int MyFrame::GetWidth() {
  return GetEndX() - GetStartX();
}

int MyFrame::GetHeight() {
  return GetEndY() - GetStartY();
}




//READ RAW
void MyFrame::OnReadRaw(wxCommandEvent & event){;
  wxFileDialog *openFileDialog = new wxFileDialog (this, _T("Open file"), _T(""), _T(""), FILETYPES, wxOPEN, wxDefaultPosition);
  if(openFileDialog->ShowModal() == wxID_OK){
    changes.clear();
    undoing = FALSE;
    wxString filename = openFileDialog->GetFilename();
    wxString path = openFileDialog->GetPath();
    ifstream file(path.mb_str(), ios::in | ios::binary | ios::ate);
    if(file.is_open()){
      streampos size = file.tellg();
      char * cont = new char[size];
      file.seekg(0, ios::beg);
      file.read(cont, size);
      file.close();
      int side = sqrt(size);
      loadedImage = new wxImage(side, side);
      for(int i=0; i<side; i++){
        for(int j=0; j<side; j++){
          int grayValue = cont[j * side + i];
          loadedImage->SetRGB(i, j, grayValue, grayValue, grayValue);
        }
      }
      delete[] cont;
      stuffToDraw = ORIGINAL_IMG;
    }
    Refresh();
  }
}

void MyFrame::Undo(wxCommandEvent & event){
  if(changes.size() > 1){
    changes.pop_back();
    bitmap = changes.back();
    free(loadedImage);
    loadedImage = new wxImage(bitmap.ConvertToImage());
    undoing = TRUE;
    Refresh();
  }
}

//INVERT IMAGE
void MyFrame::OnInvertImage(wxCommandEvent & event){
    statusBar->PushStatusText(_T("Inverting"));
    free(loadedImage);
    loadedImage = new wxImage(bitmap.ConvertToImage());
    for(int i=GetStartX(); i<GetEndX(); i++){
       for(int j=GetStartY(); j<GetEndY();j++){
 	      loadedImage->SetRGB(i,j,
         255-loadedImage->GetRed(i,j),
				 255-loadedImage->GetGreen(i,j),
         255-loadedImage->GetBlue(i,j));
       }
    }
    statusBar->PopStatusText();
    Refresh();
}

//IMAGE SCALEING
void MyFrame::OnScaleImage(wxCommandEvent & event){
    printf("Scaling...");
    free(loadedImage);
    loadedImage = new wxImage(bitmap.ConvertToImage());

    for(int i=GetStartX(); i<GetEndX(); i++){
      for(int j=GetStartY(); j<GetEndY();j++){
         loadedImage->SetRGB(i,j,
         2.5* loadedImage->GetRed(i,j),
			   2.5* loadedImage->GetGreen(i,j),
			   2.5* loadedImage->GetBlue(i,j));
      }
    }
    printf(" Finished scaling.\n");
    Refresh();
}

void MyFrame::OnShiftImage(wxCommandEvent & event){
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  int factor = wxGetNumberFromUser(_("Enter factor"), wxT(""), wxT(""), 0, -255, 255, this);
  statusBar->PushStatusText(_T("Shifting..."));
  unsigned char r,g,b;
  int temp;
  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY(); j++){
      temp = loadedImage->GetRed(i, j);
      r = max(0, min(255, loadedImage->GetRed(i, j) + factor));
      g = max(0, min(255, loadedImage->GetGreen(i, j) + factor));
      b = max(0, min(255, loadedImage->GetBlue(i, j) + factor));
      loadedImage->SetRGB(i, j, r, g, b);
    }
  }
  statusBar->PopStatusText();
  Refresh();
}

//WEEK 5

void MyFrame::OnConvolution(wxCommandEvent & event){
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  wxImage *originalImage = new wxImage(bitmap.ConvertToImage());
  wxArrayString choices;
  choices.Add(_("Averaging"));
  choices.Add(_("Weighted averaging"));
  choices.Add(_("4-neighbour Laplacian"));
  choices.Add(_("8-neighbour Laplacian"));
  choices.Add(_("4-neighbour Laplacian Enhancement"));
  choices.Add(_("8-neighbour Laplacian Enhancement"));
  choices.Add(_("Roberts"));
  choices.Add(_("Roberts absolute value conversion"));
  choices.Add(_("Sobel X"));
  choices.Add(_("Sobel Y"));
  int choice = wxGetSingleChoiceIndex(_("Select convolution"), wxT(""), choices, this);
  bool absolute = false;
  if(choice > 5) absolute = true;
  int matrix[3][3];
  int divisor = 1;
  statusBar->PushStatusText(_("Convoluting"));
  switch(choice){
      case -1:
        free(originalImage);
        statusBar->PopStatusText();
        return;
      case 0:
        divisor = 9;
        matrix[0][0] = 1;
        matrix[0][1] = 1;
        matrix[0][2] = 1;
        matrix[1][0] = 1;
        matrix[1][1] = 1;
        matrix[1][2] = 1;
        matrix[2][0] = 1;
        matrix[2][1] = 1;
        matrix[2][2] = 1;
        break;
      case 1:
        divisor = 16;
        matrix[0][0] = 1;
        matrix[0][1] = 2;
        matrix[0][2] = 1;
        matrix[1][0] = 2;
        matrix[1][1] = 4;
        matrix[1][2] = 2;
        matrix[2][0] = 1;
        matrix[2][1] = 2;
        matrix[2][2] = 1;
        break;
      case 2:
        matrix[0][0] = 0;
        matrix[0][1] = -1;
        matrix[0][2] = 0;
        matrix[1][0] = -1;
        matrix[1][1] = 4;
        matrix[1][2] = -1;
        matrix[2][0] = 0;
        matrix[2][1] = -1;
        matrix[2][2] = 0;
        break;
      case 3:
        matrix[0][0] = -1;
        matrix[0][1] = -1;
        matrix[0][2] = -1;
        matrix[1][0] = -1;
        matrix[1][1] = 8;
        matrix[1][2] = -1;
        matrix[2][0] = -1;
        matrix[2][1] = -1;
        matrix[2][2] = -1;
        break;
      case 4:
        matrix[0][0] = 0;
        matrix[0][1] = -1;
        matrix[0][2] = 0;
        matrix[1][0] = -1;
        matrix[1][1] = 5;
        matrix[1][2] = -1;
        matrix[2][0] = 0;
        matrix[2][1] = -1;
        matrix[2][2] = 0;
        break;
      case 5:
        matrix[0][0] = -1;
        matrix[0][1] = -1;
        matrix[0][2] = -1;
        matrix[1][0] = -1;
        matrix[1][1] = 9;
        matrix[1][2] = -1;
        matrix[2][0] = -1;
        matrix[2][1] = -1;
        matrix[2][2] = -1;
        break;
      case 6:
        matrix[0][0] = 0;
        matrix[0][1] = 0;
        matrix[0][2] = 0;
        matrix[1][0] = 0;
        matrix[1][1] = 0;
        matrix[1][2] = -1;
        matrix[2][0] = 0;
        matrix[2][1] = 1;
        matrix[2][2] = 0;
        break;
      case 7:
        matrix[0][0] = 0;
        matrix[0][1] = 0;
        matrix[0][2] = 0;
        matrix[1][0] = 0;
        matrix[1][1] = -1;
        matrix[1][2] = 0;
        matrix[2][0] = 0;
        matrix[2][1] = 0;
        matrix[2][2] = 1;
        break;
      case 8:
        matrix[0][0] = -1;
        matrix[1][1] = 0;
        matrix[1][2] = 2;
        matrix[2][0] = -1;
        matrix[2][1] = 0;
        matrix[2][2] = 1;
        break;
      case 9:
        matrix[0][0] = -1;
        matrix[0][1] = -2;
        matrix[0][2] = -1;
        matrix[1][0] = 0;
        matrix[1][1] = 0;
        matrix[1][2] = 0;
        matrix[2][0] = 1;
        matrix[2][1] = 2;
        matrix[2][2] = 1;
    }

    for(int i=GetStartX()+1; i<GetEndX()-1; i++){
      for(int j=GetStartY()+1; j<GetEndY()-1;j++){
        int r = matrix[0][0] * originalImage->GetRed(i-1, j-1) +
        matrix[0][1] * originalImage->GetRed(i-1, j) +
        matrix[0][2] * originalImage->GetRed(i-1, j+1) +
        matrix[1][0] * originalImage->GetRed(i, j-1) +
        matrix[1][1] * originalImage->GetRed(i, j) +
        matrix[1][2] * originalImage->GetRed(i, j+1) +
        matrix[2][0] * originalImage->GetRed(i+1, j-1) +
        matrix[2][1] * originalImage->GetRed(i+1, j) +
        matrix[2][2] * originalImage->GetRed(i+1, j+1);
        r /= divisor;
        int g = matrix[0][0] * originalImage->GetGreen(i-1, j-1) +
        matrix[0][1] * originalImage->GetGreen(i-1, j) +
        matrix[0][2] * originalImage->GetGreen(i-1, j+1) +
        matrix[1][0] * originalImage->GetGreen(i, j-1) +
        matrix[1][1] * originalImage->GetGreen(i, j) +
        matrix[1][2] * originalImage->GetGreen(i, j+1) +
        matrix[2][0] * originalImage->GetGreen(i+1, j-1) +
        matrix[2][1] * originalImage->GetGreen(i+1, j) +
        matrix[2][2] * originalImage->GetGreen(i+1, j+1);
        g /= divisor;
        int b = matrix[0][0] * originalImage->GetBlue(i-1, j-1) +
        matrix[0][1] * originalImage->GetBlue(i-1, j) +
        matrix[0][2] * originalImage->GetBlue(i-1, j+1) +
        matrix[1][0] * originalImage->GetBlue(i, j-1) +
        matrix[1][1] * originalImage->GetBlue(i, j) +
        matrix[1][2] * originalImage->GetBlue(i, j+1) +
        matrix[2][0] * originalImage->GetBlue(i+1, j-1) +
        matrix[2][1] * originalImage->GetBlue(i+1, j) +
        matrix[2][2] * originalImage->GetBlue(i+1, j+1);
        b /= divisor;
        if(absolute){
          r = abs(r);
          g = abs(g);
          b = abs(b);
        } else {
          r += 100;
          g += 100;
          b += 100;
        }
        r = max(0, min(255, r));
        g = max(0, min(255, g));
        b = max(0, min(255, b));
        loadedImage->SetRGB(i, j, r, g, b);
      }
    }
    free(originalImage);
    Refresh();
    statusBar->PopStatusText();
}

//WEEK 6

void MyFrame::SaltAndPepper(wxImage *image){
  free(loadedImage);
  statusBar->PushStatusText(_("Salting and peppering"));
  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      int val = rand() % 20;
      if(val == 0)
        image->SetRGB(i, j, 0, 0 ,0);
      else if(val == 1)
        image->SetRGB(i, j, 0, 0 ,0);
    }
  }
}

void MyFrame::Grayscale(wxCommandEvent & event){
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      int grayValue = round((loadedImage->GetRed(i, j)+loadedImage->GetGreen(i, j)+loadedImage->GetBlue(i, j))/3);
      loadedImage->SetRGB(i, j, grayValue, grayValue, grayValue);
    }
  }
  Refresh();
}

void MyFrame::OrderStatisticsFiltering(wxCommandEvent & event){
  wxArrayString choices;
  choices.Add(_("Salt-and-Pepper Noise"));
  choices.Add(_("Min Filtering"));
  choices.Add(_("Max Filtering"));
  choices.Add(_("Midpoint Filtering"));
  int choice = wxGetSingleChoiceIndex(_("Select filter"), wxT(""), choices, this);
  if(choice == 0){
    SaltAndPepper(loadedImage);
  }
  else {
    //Grayscale(event);
    int rMin, gMin, bMin, rMax, gMax, bMax, val;
    free(loadedImage);
    loadedImage = new wxImage(bitmap.ConvertToImage());
    wxImage *imageCopy = new wxImage(bitmap.ConvertToImage());
    for(int i=GetStartX(); i<GetEndX(); i++){
      for(int j=GetStartY(); j<GetEndY();j++){
        int rMin = gMin = bMin = 255;
        int rMax = gMax = bMax = 0;
        for(int x=max(0, i-1); x<min(imgWidth, i+1); x++){
          for(int y=max(0, j-1); y<min(imgHeight, j+1); y++){
            int val = imageCopy->GetRed(x, y);
            rMin = min(rMin, val);
            rMax = max(rMax, val);
            val = imageCopy->GetGreen(x, y);
            gMin = min(gMin, val);
            gMax = max(gMax, val);
            val = imageCopy->GetBlue(x, y);
            bMin = min(bMin, val);
            bMax = max(bMax, val);
          }
        }
        switch(choice){
          case 1:
            loadedImage->SetRGB(i, j, rMin, gMin, bMin);
            break;
          case 2:
            loadedImage->SetRGB(i, j, rMax, gMax, bMax);
            break;
          case 3:
            int rAvg = round((rMin+rMax)/2);
            int gAvg = round((gMin+gMax)/2);
            int bAvg = round((bMin+bMax)/2);
            loadedImage->SetRGB(i, j, rAvg, gAvg, bAvg);
        }
      }
    }
    free(imageCopy);
  }
  statusBar->PopStatusText();
  Refresh();
}

void MyFrame::OnLogarithmic(wxCommandEvent & event){
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  int factor = wxGetNumberFromUser(_("Enter factor"), wxT(""), wxT(""), 20, 10, 255, this);
  if(factor == -1) return;
  statusBar->PushStatusText(_T("Logarithming"));
  unsigned char r,g,b;
  int temp;
  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      r = max(0, min(255, (int)round(log(1+ loadedImage->GetRed(i, j)) * factor)));
      g = max(0, min(255, (int)round(log(1+ loadedImage->GetGreen(i, j)) * factor)));
      b = max(0, min(255, (int)round(log(1+ loadedImage->GetBlue(i, j)) * factor)));
      loadedImage->SetRGB(i, j, r, g, b);
    }
  }
  statusBar->PopStatusText();
  Refresh();
}

void MyFrame::OnPowerLaw(wxCommandEvent & event){
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  wxString text = wxGetTextFromUser(_("Enter exponent 0.01-25.0"), wxT(""), wxT("0.01"), this);
  int factor = wxGetNumberFromUser(_("Enter factor"), wxT(""), wxT(""), 20, 0, 255, this);
  double exponent;
  int r, g, b;
  if(text.IsEmpty()){
    statusBar->PopStatusText();
    return;
  }
  if(!text.ToDouble(&exponent)){
    exponent = 1.0;
  }
  exponent = min(25.0, max(0.01, exponent));
  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      r = min(255, max(0, (int)round(pow(loadedImage->GetRed(i, j), exponent) * factor)));
      g = min(255, max(0, (int)round(pow(loadedImage->GetGreen(i, j), exponent) * factor)));
      b = min(255, max(0, (int)round(pow(loadedImage->GetBlue(i, j), exponent) * factor)));
      loadedImage->SetRGB(i, j, r, g, b);
    }
  }
  statusBar->PushStatusText(_T("Logarithming"));
  statusBar->PopStatusText();
  Refresh();
}

void MyFrame::OnRandomLookUp(wxCommandEvent & event){
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  int lookup[256];
  for(int i=0; i<256; i++){
    lookup[i] = rand() % 256;
  }
  int r,g,b;
  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      r = loadedImage->GetRed(i, j);
      g = loadedImage->GetGreen(i, j);
      b = loadedImage->GetBlue(i, j);
      loadedImage->SetRGB(i, j, lookup[r], lookup[g], lookup[b]);
    }
  }
  Refresh();
}

void MyFrame::OnEqualise(wxCommandEvent & event){
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  int red[256];
  int green[256];
  int blue[256];

  for(int i=0; i < 256; i++){
    red[i] = 0;
    green[i] = 0;
    blue[i] = 0;
  }

  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      red[loadedImage->GetRed(i, j)]++;
      green[loadedImage->GetGreen(i, j)]++;
      blue[loadedImage->GetBlue(i, j)]++;
    }
  }

  for(int i=1; i < 256; i++){
    red[i] += red[i-1];
    green[i] += green[i-1];
    blue[i] += blue[i-1];
  }

  int pixNum = GetWidth() * GetHeight();
  for(int i=0; i< 256; i++){
    red[i] = (int)round(red[i] * 255 / pixNum);
    green[i] = (int)round(green[i] * 255 / pixNum);
    blue[i] = (int)round(blue[i] * 255 / pixNum);
  }

  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      int r = red[loadedImage->GetRed(i, j)];
      int g = green[loadedImage->GetGreen(i, j)];
      int b = blue[loadedImage->GetBlue(i, j)];
      loadedImage->SetRGB(i, j, r, g, b);
    }
  }
  Refresh();
}

void MyFrame::SimpleThresholding(wxCommandEvent & event){
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  int threshold = wxGetNumberFromUser(_("Select thresholding value"),  wxT(""), wxT(""), 20, 0, 255, this);
  if(threshold == -1) return;
  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      int r = loadedImage->GetRed(i, j) >= threshold ? 255 : 0;
      int g = loadedImage->GetGreen(i, j) >= threshold ? 255 : 0;
      int b = loadedImage->GetBlue(i, j) >= threshold ? 255 : 0;
      loadedImage->SetRGB(i, j, r, g, b);
    }
  }
  Refresh();
}

void MyFrame::AutomatedThresholding(wxCommandEvent & event){
  free(loadedImage);
  loadedImage = new wxImage(bitmap.ConvertToImage());
  int uBack = 0;
  int uObj = 0;
  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      if((i==0 || i==GetEndX()-1) && (j==0 || j==GetEndY()-1))
        uBack += loadedImage->GetRed(i, j);
      else
        uObj += loadedImage->GetRed(i, j);
    }
  }
  uBack /= 4;
  uObj /= GetWidth() * GetHeight() - 4;
  int initialT = (uBack + uObj) / 2;
  int currT, countBack, countObj;
  int nextT = initialT;
  do {
    currT = nextT;
    uBack = 0;
    uObj = 0;
    countBack = 0;
    countObj = 0;
    for(int i=GetStartX(); i<GetEndX(); i++){
      for(int j=GetStartY(); j<GetEndY();j++){
        int r = loadedImage->GetRed(i, j);
        if (r >= currT) {
          uObj += r;
          countObj++;
        } else {
          uBack += r;
          countBack++;
        }
      }
    }
    nextT = (uObj / countObj + uBack / countBack) / 2;
  } while(nextT != currT);
  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      int r = loadedImage->GetRed(i, j) >= currT ? 255 : 0;
      int g = loadedImage->GetGreen(i, j) >= currT ? 255 : 0;
      int b = loadedImage->GetBlue(i, j) >= currT ? 255 : 0;
      loadedImage->SetRGB(i, j, r, g, b);
    }
  }
  Refresh();
}

int MyFrame::getMeanDeviation(wxImage *image){
  int redMean = 0;
  int greenMean = 0;
  int blueMean = 0;
  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      redMean += image->GetRed(i, j);
      greenMean += image->GetGreen(i, j);
      blueMean += image->GetBlue(i, j);
    }
  }
  redMean /= GetWidth()*GetHeight();
  greenMean /= GetWidth()*GetHeight();
  blueMean /= GetWidth()*GetHeight();

  int redMeanDev = 0;
  int greenMeanDev = 0;
  int blueMeanDev = 0;

  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      redMeanDev += abs(redMean - image->GetRed(i, j));
      greenMeanDev += abs(greenMean - image->GetGreen(i, j));
      blueMeanDev += abs(blueMean - image->GetBlue(i, j));
    }
  }

  redMeanDev /= GetWidth() * GetHeight();
  greenMeanDev /= GetWidth() * GetHeight();
  blueMeanDev /= GetWidth() * GetHeight();

  return (redMeanDev + greenMeanDev + blueMeanDev) / 3;
}

int MyFrame::getStandardDeviation(wxImage *image){
  int redMean = 0;
  int greenMean = 0;
  int blueMean = 0;
  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      redMean += image->GetRed(i, j);
      greenMean += image->GetGreen(i, j);
      blueMean += image->GetBlue(i, j);
    }
  }
  redMean /= GetWidth()*GetHeight();
  greenMean /= GetWidth()*GetHeight();
  blueMean /= GetWidth()*GetHeight();

  int redMeanDev = 0;
  int greenMeanDev = 0;
  int blueMeanDev = 0;

  for(int i=GetStartX(); i<GetEndX(); i++){
    for(int j=GetStartY(); j<GetEndY();j++){
      redMeanDev += pow(abs(redMean - image->GetRed(i, j)), 2);
      greenMeanDev += pow(abs(greenMean - image->GetGreen(i, j)), 2);
      blueMeanDev += pow(abs(blueMean - image->GetBlue(i, j)), 2);
    }
  }

  redMeanDev /= GetWidth() * GetHeight();
  greenMeanDev /= GetWidth() * GetHeight();
  blueMeanDev /= GetWidth() * GetHeight();
  return sqrt((redMeanDev + greenMeanDev + blueMeanDev) / 3);
}

//###########################################################//
//-----------------------------------------------------------//
//---------- DO NOT MODIFY THE CODE BELOW--------------------//
//-----------------------------------------------------------//
//###########################################################//


//IMAGE SAVING
void MyFrame::OnSaveImage(wxCommandEvent & event){

    printf("Saving image...");
    free(loadedImage);
    loadedImage = new wxImage(bitmap.ConvertToImage());

    loadedImage->SaveFile(wxT("Saved_Image.bmp"), wxBITMAP_TYPE_BMP);

    printf("Finished Saving.\n");
}


void MyFrame::OnExit (wxCommandEvent & event){
  Close(TRUE);
}


void MyFrame::OnPaint(wxPaintEvent & event){
  wxPaintDC dc(this);
  int cWidth, cHeight;
  GetSize(&cWidth, &cHeight);
  if ((back_bitmap == NULL) || (oldWidth != cWidth) || (oldHeight != cHeight)) {
    if (back_bitmap != NULL)
      delete back_bitmap;
    back_bitmap = new wxBitmap(cWidth, cHeight);
    oldWidth = cWidth;
    oldHeight = cHeight;
  }
  wxMemoryDC *temp_dc = new wxMemoryDC(&dc);
  temp_dc->SelectObject(*back_bitmap);
  // We can now draw into the offscreen DC...
  temp_dc->Clear();
  if(loadedImage)
    temp_dc->DrawBitmap(wxBitmap(*loadedImage), 0, 0, false);//given bitmap xcoord y coord and transparency

  switch(stuffToDraw){
     case NOTHING:
        break;
     case ORIGINAL_IMG:
       bitmap.CleanUpHandlers; // clean the actual image header
       bitmap = wxBitmap(*loadedImage); // Update the edited/loaded image
       break;
  }

// update image size
imgWidth  = (bitmap.ConvertToImage()).GetWidth();
imgHeight = (bitmap.ConvertToImage()).GetHeight();



 temp_dc->SelectObject(bitmap);//given bitmap

  //end draw all the things
  // Copy from this DC to another DC.
  dc.Blit(0, 0, cWidth, cHeight, temp_dc, 0, 0);
  delete temp_dc; // get rid of the memory DC
  if(!undoing)
    changes.push_back(bitmap);
  else
    undoing = FALSE;

  if(!rect.IsEmpty()) {
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetLogicalFunction(wxXOR);
    dc.SetPen(wxPen(*wxWHITE, 1));
    dc.DrawRectangle(rect.GetPosition(), rect.GetSize());
  }
}

BEGIN_EVENT_TABLE (MyFrame, wxFrame)
  EVT_MENU ( LOAD_FILE_ID,  MyFrame::OnOpenFile)
  EVT_MENU ( EXIT_ID,  MyFrame::OnExit)

//###########################################################//
//----------------------START MY EVENTS ---------------------//
//###########################################################//

  EVT_MENU ( INVERT_IMAGE_ID,  MyFrame::OnInvertImage)
  EVT_MENU ( SCALE_IMAGE_ID,  MyFrame::OnScaleImage)
  EVT_MENU ( SAVE_IMAGE_ID,  MyFrame::OnSaveImage)
  EVT_MENU ( SHIFT_IMAGE,  MyFrame::OnShiftImage)
  EVT_MENU ( CONVOLUTION,  MyFrame::OnConvolution)
  EVT_MENU ( OSF,  MyFrame::OrderStatisticsFiltering)
  EVT_MENU ( GRAYSCALE,  MyFrame::Grayscale)
  EVT_MENU ( LOGARITHMIC,  MyFrame::OnLogarithmic)
  EVT_MENU ( POWERLAW,  MyFrame::OnPowerLaw)
  EVT_MENU ( RANDOMLOOKUP,  MyFrame::OnRandomLookUp)
  EVT_MENU ( READRAW,  MyFrame::OnReadRaw)
  EVT_MENU ( EQUALISE,  MyFrame::OnEqualise)
  EVT_MENU ( SIMPLETHRESHOLDING,  MyFrame::SimpleThresholding)
  EVT_MENU ( AUTOMATEDTHRESHOLDING,  MyFrame::AutomatedThresholding)
  EVT_MENU ( UNDO,  MyFrame::Undo)
  EVT_LEFT_DOWN (MyFrame::MouseDown)
  EVT_LEFT_UP (MyFrame::MouseUp)
  EVT_MOTION (MyFrame::Motion)

//###########################################################//
//----------------------END MY EVENTS -----------------------//
//###########################################################//

  EVT_PAINT (MyFrame::OnPaint)
END_EVENT_TABLE()
