/*Data structures for supporting multiple windows.*/

struct _WINDOW;
typedef struct _WINDOW WINDOW;
typedef struct _WINDOW FRAMEWINDOW;
#define MAX_CHILDREN 10
#define WINDOW_MARKER "SD386 Window Data Structure"

/*Define known WINDOW classes.*/
#define TEXT_WINDOW               1
#define SCROLLBAR_WINDOW          2
#define SCROLLUPARROW             3
#define SCROLLDNARROW             4
#define SCROLLSLIDER              5
#define STRING_WINDOW             6
#define PUTRC_TABLE_WINDOW        7
#define BUTTON_WINDOW             8
#define DATA_WINDOW               9

#define FRAME_WINDOW            100
#define FRAMEUL_WINDOW          101
#define FRAMEUR_WINDOW          103
#define FRAMELL_WINDOW          104
#define FRAMELR_WINDOW          105
#define FRAMETOP_WINDOW         106
#define FRAMEBOT_WINDOW         107
#define FRAMELEFT_WINDOW        108
#define FRAMERIGHT_WINDOW       109
#define FRAMEMOVER_WINDOW       110
#define FRAMETITLE_WINDOW       111

#define REGISTER_WINDOW         200
#define COREGISTER_WINDOW       201

//Ordinal numbers for Frame Children
#define FRAMEUL_CHILD              0
#define FRAMEUR_CHILD              1
#define FRAMELL_CHILD              2
#define FRAMELR_CHILD              3
#define FRAMETOP_CHILD             4
#define FRAMEBOT_CHILD             5
#define FRAMELEFT_CHILD            6
#define FRAMERIGHT_CHILD           7
#define FRAMEMOVER_CHILD           0 //Children of frame top
#define FRAMETITLE_CHILD           1 //Children of frame top

#define VIEW_NORMAL      0
#define VIEW_SHOW        1
#define VIEW_DONT_SHOW   2
typedef struct {
    int    rc;
    WINDOW *Window;
    PEVENT Event;
    int    func;
    int    iview; /*Used to indicate whether output is written to screen.*/
                  /*putrc tests. iview=0=> normal;iview=1=>write; iview=2=>dont't write*/
} WINDOWEVENT;


struct _WINDOW {
     char    *marker;
     WINDOW *self;  /*This is used to guarantee that we are looking at a window.*/
     WINDOW *parent;
     int row;
     int height;
     int col, width;
     int num_children;
     WINDOW *children[MAX_CHILDREN];
     int itype;
     int is_active;
     uchar attrib;
     char *title, *heading;
     WINDOWEVENT *(*eventhandler)(WINDOWEVENT *);
     WINDOW  *(*painter)(WINDOW *);
     void *user_data;

 };

typedef struct {
        int topLine;
        int linesPer;
        int videoCols;
        int videoWidth;
        int colStart;
        int rowStart;
        int reg_Display;
        int asmRows;
        int asmTop;
        int vioStartOffSet;
        int iview;
        uchar boundPtr[1000];
} VioContext;

#define NUM_WINDOWS        9
#define MIN_MSH_LINES      0
#define START_MSH_LINES    5
#define SOURCE_SAVE_WIN    0
#define SOURCE_WIN         3
#define MSHPROMPT_WIN      2
#define MSHCOMMAND_WIN     5
#define MSHDATA_WIN        1
#define DIVIDER_WIN        4
#define REGISTER_WIN       6
#define COREGISTER_WIN     7
#define DATA_WIN           8
#define MSHCOMMANDPROMPT "MSH Command "

extern uchar    ScrollShade1[];         /* attribute string for scroll bar.  */
extern uchar    ScrollShade2[];         /* attribute string for scroll bar.  */

WINDOW *GetEventWindow(WINDOW *w) ;
AFILE *ResetWindow(WINDOW *w);
void PaintWindow(WINDOW *w);
void LowerWindow(WINDOW *w);
void RaiseWindow(WINDOW *w);
void ScrollWindow(WINDOW *w, int line);
void ResetScrollbar(WINDOW *parent);
WINDOW *CreateScrollbar(WINDOW *parent);

FRAMEWINDOW *CreateFrame(WINDOW *parent, char *heading);
void ResizeFrame(FRAMEWINDOW *w,int row, int col, int height, int width);


WINDOW *CreateDivider(WINDOW *parent,char *title,
                      char **buttons,
                      WINDOWEVENT *(*buttonfns[])(WINDOWEVENT *windowevent)
                      );
void SetDivider(int row);

WINDOW *PaintScrollbar(WINDOW *w);
WINDOW *PaintFrame(WINDOW *w);
WINDOW *PaintData(WINDOW *w);

WINDOWEVENT *SourceHandler(WINDOWEVENT *windowevent);
WINDOWEVENT *MSHLOGHandler(WINDOWEVENT *windowevent);
WINDOWEVENT *ScrollHandler(WINDOWEVENT *windowevent);
WINDOWEVENT *TextHandler  (WINDOWEVENT *windowevent);
WINDOWEVENT *DividerHandler  (WINDOWEVENT *windowevent);
WINDOWEVENT *FrameHandler  (WINDOWEVENT *windowevent);
WINDOWEVENT *DataHandler  (WINDOWEVENT *windowevent);

WINDOW *CreateWindow(WINDOW *parent, char *title);
WINDOW *GetScrollbar(WINDOW *w);
WINDOW *GetSlider(WINDOW *scrollbar);
WINDOW *GetUpArrow(WINDOW *scrollbar);
WINDOW *GetDnArrow(WINDOW *scrollbar);
WINDOW *GetTextParent(WINDOW *w);
WINDOW *GetTextOrData(WINDOW *w);

extern WINDOW *root, *Window[NUM_WINDOWS], *framemsh, *framesrc;

WINDOW *focuswin;
void SetFocusWin(WINDOW *win);
void PutCsrWin(WINDOW *win);
void DoVscr(WINDOW *w, int amt);
void DoVcsr(WINDOW *w, int amt);
void FmtTxt(WINDOW *w);
void fmtstr(char *str,uchar attrib, WINDOW *w);
extern USHORT ColStart, RowStart;


uint GetWtime(uchar *filename);

void showvRegs(WINDOW *window);
void showCoRegs(WINDOW *window);

typedef struct
    { char *videoBuffer; int vlen; }
VideoBuffer;

VideoBuffer *SaveVideoBuffer(void);

SEL RestoreVideoBuffer(VideoBuffer *videoBuffer);

VioContext *SaveVioContext(VioContext *vioContext);
void *RestoreVioContext(VioContext *vioContext) ;
extern VioContext MainVioContext;
extern int iview;
extern uint   VideoWidth;
