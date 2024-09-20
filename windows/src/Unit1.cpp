/*

Author:  DnaX
www:     http://dnax.netsons.org
email:   dnax88@gmail.com

Controllo interfaccia AD/DA su porta parallela

*/

#define ENABLE_OFF 0
#define ENABLE_ON 1
#define I_REGISTER 0
#define D_REGISTER 4
#define WRITE_DATA 2
#define READ_DATA 0

#define FUNCTION_8BIT 0x30
#define FUNCTION_4BIT 0x20
#define FUNCTION_2LINE 0x28
#define FUNCTION_1LINE 0x20

#define PARALLEL_ADDR	0x378					// indirizzo base porta centronics
#define DATA_REG		(PARALLEL_ADDR + 0)
#define STATUS_REG		(PARALLEL_ADDR + 1)
#define CONTROL_REG		(PARALLEL_ADDR + 2)

#define setta 0x37A  /* permette di scrivere sul registro di controllo */
#define riposo 0x1B  /* impostazioni registro di controllo */
#define DAC 0x0B
#define ADC 0x1A
#define start1 0x3E
#define start0 0x3A  /* impostazioni registro di controllo */
#define IN3 0xF5
#define IN2 0xF7
#define IN1 0xFD
#define IN0 0xFF

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
int i,flag;
double tmp;
float tmp2;
int buffer;
unsigned char bin_data0, bin_data1, bin_data2, bin_data3;
unsigned int a0, a1, b0, b1, c0, c1, d0, d1 ;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button1Click(TObject *Sender)
{

Outp(0x00, 0x378);

}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormCreate(TObject *Sender)
{
if(OpenIO(DRIVERNAME)==FALSE)
{
        MessageBox(Form1->Handle,"Driver ISAKerPlug non trovato","PortIO", 16);
        exit(0);
}
Outp(0x00, DATA_REG);
TrackBar1->Position = 500;
Outp(riposo, CONTROL_REG);
Outp(DAC, CONTROL_REG);

                lbl0->Caption = "--";
                lbl1->Caption = "--";
                lbl2->Caption = "--";
                lbl3->Caption = "--";
                lbl_bin0->Caption = "--";
                lbl_bin1->Caption = "--";
                lbl_bin2->Caption = "--";
                lbl_bin3->Caption = "--";

}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
Outp(0x00, DATA_REG);
Outp(0x0F, CONTROL_REG);
CloseIO();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button2Click(TObject *Sender)
{
Outp(0xFF, 0x378);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer1Timer(TObject *Sender)
{

Outp(i, 0x378);
Edit1->Text = i;

if(radSine->Checked)
{
   if(tmp>=100) flag = 0;
   else if(tmp<=0) flag = 1;

   if(flag) tmp += 0.01;
   else tmp -= 0.01;

   i = (unsigned char) (cos(tmp)*128 + 128);
}
else if(radSquare->Checked)
{
   i = (unsigned char) ~i;
}
else if(radTriangle->Checked)
{
   if(i==255) flag = 0;
   else if(i==0) flag = 1;

   if(flag) i++;
   else i--;

}
else if(radExp->Checked)
{
   if(!i) i=1;
   i = i*2;
   if(i>=255) { i=1; }
}

}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button3Click(TObject *Sender)
{
if(Timer1->Enabled)
{
        Timer1->Enabled = false;
        Button3->Caption = "Start";
        i = 0;
        Outp(0x00, 0x378);
        Edit1->Text = "";
        Button1->Enabled = true;
        Button2->Enabled = true;
        Button4->Enabled = true;
        Button5->Enabled = true;
        CheckBox1->Enabled = true;
}
else
{
        Timer1->Enabled = true;
        Button3->Caption = "Stop";
        Button1->Enabled = false;
        Button2->Enabled = false;
        Button4->Enabled = false;
        Button5->Enabled = false;
        CheckBox1->Enabled = false;
        i = 1;
}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TrackBar1Change(TObject *Sender)
{
Timer1->Interval = TrackBar1->Position;
Label1->Caption = TrackBar1->Position;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Button4Click(TObject *Sender)
{
if(Edit2->Text == "") Edit2->Text = "0";
Outp(Edit2->Text.ToInt(), 0x378);
Edit2->Text = "";
Edit2->SetFocus();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Timer2Timer(TObject *Sender)
{
   char buffer[16] =  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

   if(buffer != Edit3->Text.c_str())
   {

        strcpy(buffer,Edit3->Text.c_str());


        for(i=0;i<16;i++)
        {
                if(buffer[i] == 0x00)
                buffer[i] = 0x20;
    	        Outp(D_REGISTER + WRITE_DATA, CONTROL_REG); 	//Set register to Write Data
	        Outp(buffer[i], DATA_REG); 				// Set the character code
	        Outp(ENABLE_ON + D_REGISTER + WRITE_DATA, CONTROL_REG); 	//Enable High
	        Sleep(1); 					//Wait while Enable (C2) is high so LCD will have time to load the info
	        Outp(D_REGISTER + WRITE_DATA, CONTROL_REG); 	//Enable Low

        }

        // return home
        Outp(2, DATA_REG);					//Turn on D1
	Outp(I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Turn off C1 and C3
	Outp(ENABLE_ON + I_REGISTER + WRITE_DATA, CONTROL_REG);	//Turn on C2 while C1 and C3 remains off
	Sleep(1); 					//Wait while Enable (C2) is high so LCD will have time to load the info
	Outp(I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Reset Control pins
   }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button5Click(TObject *Sender)
{
Button1->Enabled = false;
Button2->Enabled = false;
Button3->Enabled = false;
Button4->Enabled = false;
Edit1->Enabled = false;
Edit2->Enabled = false;
TrackBar1->Enabled = false;
CheckBox1->Enabled = false;


// function set
Outp(56, DATA_REG);
	Sleep(1);// Bus 8 bit, 2 lines, font 5x7
	Outp(I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Turn off C1 and C3
	Sleep(1);
	Outp(ENABLE_ON + I_REGISTER + WRITE_DATA, CONTROL_REG);	//Turn on C2 while C1 and C3 remains off
	Sleep(1); 					//Wait while Enable (C2) is high so LCD will have time to load the info
	Outp(!I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Reset Control pins
	Sleep(1);

// entry mode

Outp(6, DATA_REG); 					//Turn on D1 and D2
	Outp(I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Turn off C1 and C3
	Outp(ENABLE_ON + I_REGISTER + WRITE_DATA, CONTROL_REG);	//Turn on C2 while C1 and C3 remains off
	Sleep(1); 					//Wait while Enable (C2) is high so LCD will have time to load the info
	Outp(I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Reset Control pins
	Sleep(0);
// display on

	Outp(12, DATA_REG); //Turn on D3 and D4
	Sleep(1);
	Outp(I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Turn off C1 and C3
	Sleep(1);
        Outp(ENABLE_ON + I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Turn on C2 while C1 and C3 remains off
	Sleep(1); 					//Wait while Enable (C2) is high so LCD will have time to load the info
	Outp(I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Reset Control pins
	Sleep(1);
// clear display
Outp(1, DATA_REG); 					//Turn on D0
	Outp(I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Turn off C1 and C3
	Outp(ENABLE_ON + I_REGISTER + WRITE_DATA, CONTROL_REG);	//Turn on C2 while C1 and C3 remains off
	Sleep(1); 					//Wait while Enable (C2) is high so LCD will have time to load the info
	Outp(I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Reset Control pins
	Sleep(0);

Timer2->Enabled = true;
Button6->Enabled = true;
Button5->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button6Click(TObject *Sender)
{
        Timer2->Enabled = false;

        // clear display
        Outp(1, DATA_REG); 					//Turn on D0
	Outp(I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Turn off C1 and C3
	Outp(ENABLE_ON + I_REGISTER + WRITE_DATA, CONTROL_REG);	//Turn on C2 while C1 and C3 remains off
	Sleep(1); 					//Wait while Enable (C2) is high so LCD will have time to load the info
	Outp(I_REGISTER + WRITE_DATA, CONTROL_REG); 	//Reset Control pins
	Sleep(0);

        Outp(0x00,DATA_REG);
        Button6->Enabled = false;
        Button5->Enabled = true;

        Button1->Enabled = true;
        Button2->Enabled = true;
        Button3->Enabled = true;
        Button4->Enabled = true;
        Edit1->Enabled = true;
        Edit2->Enabled = true;
        TrackBar1->Enabled = true;
        CheckBox1->Enabled = true;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::radSineClick(TObject *Sender)
{
i = 0;        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::radSquareClick(TObject *Sender)
{
i = 0;        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::radTriangleClick(TObject *Sender)
{
i = 0;        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::radExpClick(TObject *Sender)
{
i = 0;        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox1Click(TObject *Sender)
{
        if(CheckBox1->State == cbChecked)
        {
                Button1->Enabled = false;
                Button2->Enabled = false;
                Button3->Enabled = false;
                Button4->Enabled = false;
                Button5->Enabled = false;
                Edit1->Enabled = false;
                Edit2->Enabled = false;
                TrackBar1->Enabled = false;

                CheckBox2->Enabled = true;

                Timer4->Enabled = true;

                Timer3->Enabled = true;
                Outp(0x374,CONTROL_REG);
		Outp(ADC,CONTROL_REG); 

        }
        else
        {
                Button1->Enabled = true;
                Button2->Enabled = true;
                Button3->Enabled = true;
                Button4->Enabled = true;
                Button5->Enabled = true;
                Edit1->Enabled = true;
                Edit2->Enabled = true;
                TrackBar1->Enabled = true;

                CheckBox2->Enabled = false;

                Timer4->Enabled = false;
                bar0->Position = 0;
                bar1->Position = 0;
                bar2->Position = 0;
                bar3->Position = 0;

                Timer3->Enabled = false;

                lbl0->Caption = "--";
                lbl1->Caption = "--";
                lbl2->Caption = "--";
                lbl3->Caption = "--";
                lbl_bin0->Caption = "--";
                lbl_bin1->Caption = "--";
                lbl_bin2->Caption = "--";
                lbl_bin3->Caption = "--";

                bin_data0 = 0;
                bin_data1 = 0;
                bin_data2 = 0;
                bin_data3 = 0;
                Outp(riposo,CONTROL_REG);
                Outp(DAC,CONTROL_REG);
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer3Timer(TObject *Sender)
{
      Outp(IN0 & start0, CONTROL_REG);
      Sleep(5);
      Outp(IN0 & start1, CONTROL_REG);
      Sleep(5);
      Outp(IN0 & start0, CONTROL_REG);

      Sleep(10);

      bin_data0 = Inp(DATA_REG);

      lbl_bin0->Caption = bin_data0;

      tmp2 = bin_data0 * 5;
      tmp2 = tmp2 / 255;

      lbl0->Caption = AnsiString().sprintf("%1.2f Volt",tmp2);

      Outp(IN1 & start0, CONTROL_REG);
      Sleep(5);
      Outp(IN1 & start1, CONTROL_REG);
      Sleep(5);
      Outp(IN1 & start0, CONTROL_REG);

      Sleep(10);

      bin_data1 = Inp(DATA_REG);

      lbl_bin1->Caption = bin_data1;

      tmp2 = bin_data1 * 5;
      tmp2 = tmp2 / 255;

      lbl1->Caption = AnsiString().sprintf("%1.2f Volt",tmp2);

      Outp(IN2 & start0, CONTROL_REG);
      Sleep(5);
      Outp(IN2 & start1, CONTROL_REG);
      Sleep(5);
      Outp(IN2 & start0, CONTROL_REG);

      Sleep(10);

      bin_data2 = Inp(DATA_REG);

      lbl_bin2->Caption = bin_data2;

      tmp2 = bin_data2 * 5;
      tmp2 = tmp2 / 255;

      lbl2->Caption = AnsiString().sprintf("%1.2f Volt",tmp2);

      Outp(IN3 & start0, CONTROL_REG);
      Sleep(5);
      Outp(IN3 & start1, CONTROL_REG);
      Sleep(5);
      Outp(IN3 & start0, CONTROL_REG);

      Sleep(10);

      bin_data3 = Inp(DATA_REG);

      lbl_bin3->Caption = bin_data3;

      tmp2 = bin_data3 * 5;
      tmp2 = tmp2 / 255;

      lbl3->Caption = AnsiString().sprintf("%1.2f Volt",tmp2);

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer4Timer(TObject *Sender)
{
      bar0->Position = 0.193*bin_data0 + 0.193*a1 + 0.611*a0;

      a0 = bar0->Position;
      a1 = bin_data0;


      bar1->Position = 0.193*bin_data1 + 0.193*b1 + 0.611*b0;

      b0 = bar1->Position;
      b1 = bin_data1;


      bar2->Position = 0.193*bin_data2 + 0.193*c1 + 0.611*c0;

      c0 = bar2->Position;
      c1 = bin_data2;

      bar3->Position = 0.193*bin_data3 + 0.193*d1 + 0.611*d0;

      d0 = bar3->Position;
      d1 = bin_data3;

}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox2Click(TObject *Sender)
{
if(CheckBox2->Checked) Timer4->Enabled = true;
else
{
        Timer4->Enabled = false;
        bar0->Position = 0;
        bar1->Position = 0;
        bar2->Position = 0;
        bar3->Position = 0;
}

}
//---------------------------------------------------------------------------

