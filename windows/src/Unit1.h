//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include "diio.cpp"
#include "centronics.h"
#include <math.h>
#include <string.h>
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Chart.hpp>
#include <Series.hpp>
#include <TeEngine.hpp>
#include <TeeProcs.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
        TButton *Button1;
        TButton *Button2;
        TTimer *Timer1;
        TButton *Button3;
        TEdit *Edit1;
        TTrackBar *TrackBar1;
        TEdit *Edit2;
        TButton *Button4;
        TGroupBox *GroupBox1;
        TLabel *Label1;
        TImage *Image1;
        TTimer *Timer2;
        TGroupBox *GroupBox2;
        TButton *Button5;
        TEdit *Edit3;
        TButton *Button6;
        TGroupBox *GroupBox3;
        TTimer *Timer3;
        TRadioButton *radSine;
        TRadioButton *radSquare;
        TRadioButton *radTriangle;
        TRadioButton *radExp;
        TGroupBox *GroupBox4;
        TTimer *Timer4;
        TProgressBar *bar0;
        TProgressBar *bar1;
        TProgressBar *bar2;
        TProgressBar *bar3;
        TLabel *lbl0;
        TLabel *lbl1;
        TLabel *lbl2;
        TLabel *lbl3;
        TLabel *lbl_bin0;
        TLabel *lbl_bin1;
        TLabel *lbl_bin2;
        TLabel *lbl_bin3;
        TCheckBox *CheckBox2;
        TCheckBox *CheckBox1;
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall Timer1Timer(TObject *Sender);
        void __fastcall Button3Click(TObject *Sender);
        void __fastcall TrackBar1Change(TObject *Sender);
        void __fastcall Button4Click(TObject *Sender);
        void __fastcall Timer2Timer(TObject *Sender);
        void __fastcall Button5Click(TObject *Sender);
        void __fastcall Button6Click(TObject *Sender);
        void __fastcall radSineClick(TObject *Sender);
        void __fastcall radSquareClick(TObject *Sender);
        void __fastcall radTriangleClick(TObject *Sender);
        void __fastcall radExpClick(TObject *Sender);
        void __fastcall CheckBox1Click(TObject *Sender);
        void __fastcall Timer3Timer(TObject *Sender);
        void __fastcall Timer4Timer(TObject *Sender);
        void __fastcall CheckBox2Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
 