#include <vector>

class BasicApplication : public wxApp {
 public:
    virtual bool OnInit();
};


class MyFrame : public wxFrame {
 protected:
    wxMenuBar  *menuBar;//main menu bar
    wxMenu     *fileMenu;//file menu
    wxStatusBar *statusBar;
    wxBitmap *back_bitmap; // offscreen memory buffer for drawing
    wxToolBar *toolbar;//tollbar not necessary to use
    int oldWidth, oldHeight; // save old dimensions
    //undoing
    bool undoing;
    std::vector<wxBitmap> changes;
    wxRect rect;
    int x;
    int y;


    wxBitmap bitmap;  //structure for the edited image
    wxImage *loadedImage; // image loaded from file
    int imgWidth, imgHeight; // image dimensions
    int stuffToDraw;

   /* declear message handler */
    void OnInvertImage(wxCommandEvent & event);
    void OnScaleImage(wxCommandEvent & event);
    void OnSaveImage(wxCommandEvent & event);
    void OnShiftImage(wxCommandEvent & event);
    void OnConvolution(wxCommandEvent & event);
    void OrderStatisticsFiltering(wxCommandEvent & event);
    void SaltAndPepper(wxImage *image);
    void Grayscale(wxCommandEvent & event);
    void OnLogarithmic(wxCommandEvent & event);
    void OnPowerLaw(wxCommandEvent & event);
    void OnRandomLookUp(wxCommandEvent & event);
    void OnReadRaw(wxCommandEvent & event);
    void OnEqualise(wxCommandEvent & event);
    void SimpleThresholding(wxCommandEvent & event);
    void AutomatedThresholding(wxCommandEvent & event);
    int getMeanDeviation(wxImage *image);
    int getStandardDeviation(wxImage *image);
    void Undo(wxCommandEvent & event);
    void MouseDown(wxMouseEvent & event);
    void MouseUp(wxMouseEvent & event);
    void Motion(wxMouseEvent & event);
    int GetStartX();
    int GetStartY();
    int GetEndX();
    int GetEndY();
    int GetWidth();
    int GetHeight();


 public:
    MyFrame(const wxString title, int xpos, int ypos, int width, int height);
    virtual ~MyFrame();

    void OnExit(wxCommandEvent & event);
    void OnOpenFile(wxCommandEvent & event);
    void OnPaint(wxPaintEvent & event);

    DECLARE_EVENT_TABLE()

};

enum { NOTHING = 0,
       ORIGINAL_IMG,
};

enum { EXIT_ID = wxID_HIGHEST + 1,
       LOAD_FILE_ID,
       INVERT_IMAGE_ID,
       SCALE_IMAGE_ID,
       SAVE_IMAGE_ID,
       SHIFT_IMAGE,
       CONVOLUTION,
       OSF,
       GRAYSCALE,
       LOGARITHMIC,
       POWERLAW,
       RANDOMLOOKUP,
       READRAW,
       EQUALISE,
       SIMPLETHRESHOLDING,
       AUTOMATEDTHRESHOLDING,
       UNDO,

};
