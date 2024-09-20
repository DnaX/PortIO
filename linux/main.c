#include <stdio.h>
#include <time.h>
#include <unistd.h>
//#include <asm/io.h>
#include <sys/io.h>
#include <linux/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <fcntl.h>
#include <math.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "uber-buffer.h"

#include <cairo-xlib.h>

#include "uber-graph.h"


#define SPPDATAPORT    0x378
#define SPPSTATUSPORT  (SPPDATAPORT + 1)
#define SPPCONTROLPORT (SPPDATAPORT + 2)

#define OUTPUTENABLE 0x02
#define OUTPUTLATCH  0x04

#define setta 0x37A  /* permette di scrivere sul registro di controllo */
#define riposo 0x1B
#define DAC 0x0B
#define ADC 0x1A
#define start1 0x3E
#define start0 0x3A  /* impostazioni registro di controllo */
#define IN3 0xF5
#define IN2 0xF7
#define IN1 0xFD
#define IN0 0xFF

GtkWidget *main_window;

GtkWidget *label1;
GtkWidget *label_dac_voltage;
GtkWidget *slider_dac_voltage;
GtkWidget *adj_dac_voltage;

GtkWidget *check_dac_function;
GtkWidget *radio_square;
GtkWidget *radio_triangle;
GtkWidget *radio_sine;
GtkWidget *label_dac_period;
GtkWidget *slider_dac_period;
GtkWidget *label_dac_duty_cycle;
GtkWidget *spin_dac_duty_cycle;
GtkAdjustment *adj_dac_period;

GtkWidget *label_adc_raw;
GtkWidget *label_adc_voltage;
GtkWidget *check_adc_ch1;
GtkWidget *check_adc_ch2;
GtkWidget *check_adc_ch3;
GtkWidget *check_adc_ch4;
GtkWidget *label_adc_ch1_raw;
GtkWidget *label_adc_ch2_raw;
GtkWidget *label_adc_ch3_raw;
GtkWidget *label_adc_ch4_raw;
GtkWidget *label_adc_ch1_voltage;
GtkWidget *label_adc_ch2_voltage;
GtkWidget *label_adc_ch3_voltage;
GtkWidget *label_adc_ch4_voltage;
GtkWidget *progress_adc_ch1;
GtkWidget *progress_adc_ch2;
GtkWidget *progress_adc_ch3;
GtkWidget *progress_adc_ch4;
GtkWidget *check_adc_oscope;
GtkWidget *adc_graph;
GdkWindow *adc_graph_window;
GdkRectangle adc_graph_rect;

// funtion data
guint triangle_values[512];
guint sine_values[512];

gboolean adc_timer_enable = FALSE;
guint adc_timer_delay = 500000;

gboolean adc_oscope_enable = FALSE;
guint adc_oscope_delay = 10000;

gboolean dac_timer_enable = FALSE;
guint timer_delay = 10000;

guint dac_timer_delay_on = 10000;
guint dac_timer_delay_off = 10000;

gfloat dac_duty_cycle = 0.5;

cairo_surface_t *graph_image = NULL;

// DAC functions
guint16 i = 0;
guint8 output_data = 0x00;

gchar str_buffer[20];
guint8 ring_buffer_ch1[101];
guint8 ring_buffer_ch2[101];


static gboolean
get_ch (UberGraph *graph,
         gint       line,
         gdouble   *value,
         gpointer   user_data)
{
	switch (line) {
	case 1:
		*value = 10;
		break;
	case 2:
		*value = 30;
		break;
	default:
		g_assert_not_reached();
	}
	return TRUE;
}



static void
clear_graph()
{
    if (graph_image) {
		cairo_surface_destroy(graph_image);
		graph_image = NULL;
	}
}

static void draw_graph()
{
    cairo_t *cr;
    gint c;

    graph_image = cairo_image_surface_create( CAIRO_FORMAT_ARGB32,
	                                         adc_graph->allocation.width,
					                         adc_graph->allocation.height);
	cr = cairo_create(graph_image);

	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    cairo_set_line_width (cr, 1);
    cairo_rectangle (cr, 10.5, 10.5, adc_graph->allocation.width - 20, adc_graph->allocation.height - 20);
    cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
    cairo_stroke(cr);

    cairo_set_line_width (cr, 3);
    cairo_move_to(cr, 10, 10);

    for(c = 0; c <= 100; c++)
        cairo_line_to (cr, 10+(adc_graph->allocation.width-20)/100.0*c, 10+ring_buffer_ch1[c]/255.0*(adc_graph->allocation.height-20));
    /*gdk_draw_line (widget->window,
                    widget->style->fg_gc[gtk_widget_get_state (widget)],
                    c*2, ring_buffer[c]/255.0*100, c*2+1, ring_buffer[c+1]/255.0*100);*/
    // upload line color
    cairo_set_source_rgba (cr, 0.38, 0.69, 0.90, 0.3);
    cairo_stroke_preserve(cr);

    cairo_set_line_width (cr, 1);
    cairo_set_source_rgb (cr, 0.38, 0.69, 0.90);
    cairo_stroke(cr);

    cairo_set_line_width (cr, 3);
    cairo_move_to(cr, 10, 10);
    for(c = 0; c <= 100; c++)
        cairo_line_to (cr, 10+(adc_graph->allocation.width-20)/100.0*c, 10+ring_buffer_ch2[c]/255.0*(adc_graph->allocation.height-20));
    /*gdk_draw_line (widget->window,
                    widget->style->fg_gc[gtk_widget_get_state (widget)],
                    c*2, ring_buffer[c]/255.0*100, c*2+1, ring_buffer[c+1]/255.0*100);*/

    cairo_set_source_rgba (cr, 0.52, 0.28, 0.60, 0.3);
    cairo_stroke_preserve(cr);

    cairo_set_line_width (cr, 1);
    cairo_set_source_rgb (cr, 0.52, 0.28, 0.60);
    cairo_stroke(cr);

    cairo_destroy(cr);
}

gpointer dac_timer_square (gpointer data)
{
    while(dac_timer_enable)
    {
        outb(output_data, SPPDATAPORT);
        //output_data = ~output_data;
        if(output_data) {
            g_usleep(dac_timer_delay_on);
            output_data = 0x00;
        } else {
            g_usleep(dac_timer_delay_off);
            output_data = 0xFF;
        }
    }
    return NULL;
}

gpointer dac_timer_triangle (gpointer data)
{
    while(dac_timer_enable)
    {
        outb(triangle_values[i], SPPDATAPORT);
        if(i == 512) i = 0;
        i++;
        g_usleep(timer_delay);
    }
    return NULL;
}

gpointer dac_timer_sine (gpointer data)
{
    while(dac_timer_enable)
    {
        outb(sine_values[i], SPPDATAPORT);
        if(i == 512) i = 0;
        i++;
        g_usleep(timer_delay);
    }
    return NULL;
}


gpointer adc_oscope_callback (gpointer data)
{
    guint i = 0;
    guint8 input_data = 0x00;


    while(adc_oscope_enable)
    {
        // select CH1 and send SOC signal
        outb(IN0 & start0, SPPCONTROLPORT);
        g_usleep(5);
        outb(IN0 & start1, SPPCONTROLPORT);
        g_usleep(5);
        outb(IN0 & start0, SPPCONTROLPORT);

        // wait for EOC signal (pin 11)
        //while((inb(SPPSTATUSPORT) & 128) == 1);
        g_usleep(100);

        input_data = inb(SPPDATAPORT);

        ring_buffer_ch1[i] = input_data;

        // select CH1 and send SOC signal
        outb(IN1 & start0, SPPCONTROLPORT);
        g_usleep(5);
        outb(IN1 & start1, SPPCONTROLPORT);
        g_usleep(5);
        outb(IN1 & start0, SPPCONTROLPORT);

        // wait for EOC signal (pin 11)
        //while((inb(SPPSTATUSPORT) & 128) == 1);
        g_usleep(100);

        input_data = inb(SPPDATAPORT);

        ring_buffer_ch2[i++] = input_data;

        if(i >= 101){
            i = 0;
            //gdk_window_process_updates(adc_graph->window, FALSE);
            clear_graph();
            gtk_widget_queue_draw(adc_graph);
            //gdk_window_invalidate_rect(adc_graph->window, &adc_graph_rect, FALSE);
        }

        g_usleep(adc_oscope_delay);
    }

    return NULL;
}

gpointer adc_timer_callback (gpointer data)
{
    guint8 input_data = 0x00;

    while(adc_timer_enable)
    {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_adc_ch1)))
        {
            // select CH1 and send SOC signal
            outb(IN0 & start0, SPPCONTROLPORT);
            g_usleep(5);
            outb(IN0 & start1, SPPCONTROLPORT);
            g_usleep(5);
            outb(IN0 & start0, SPPCONTROLPORT);

            // wait for EOC signal (pin 11)
            //while((inb(SPPSTATUSPORT) & 128) == 1);
            g_usleep(100);

            input_data = inb(SPPDATAPORT);

            g_sprintf(str_buffer, "0x%1.2X", input_data);
            gtk_label_set_text(GTK_LABEL(label_adc_ch1_raw), str_buffer);
            g_sprintf(str_buffer, "%1.2f V", input_data/255.0*5);
            gtk_label_set_text(GTK_LABEL(label_adc_ch1_voltage), str_buffer);

            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_adc_ch1), input_data/255.0);
        }

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_adc_ch2)))
        {
            // select CH2 and send SOC signal
            outb(IN1 & start0, SPPCONTROLPORT);
            g_usleep(5);
            outb(IN1 & start1, SPPCONTROLPORT);
            g_usleep(5);
            outb(IN1 & start0, SPPCONTROLPORT);

            // wait for EOC signal (pin 11)
            //while((inb(SPPSTATUSPORT) & 128) == 1);
            g_usleep(100);

            input_data = inb(SPPDATAPORT);

            g_sprintf(str_buffer, "0x%1.2X", input_data);
            gtk_label_set_text(GTK_LABEL(label_adc_ch2_raw), str_buffer);
            g_sprintf(str_buffer, "%1.2f V", input_data/255.0*5);
            gtk_label_set_text(GTK_LABEL(label_adc_ch2_voltage), str_buffer);

            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_adc_ch2), input_data/255.0);
        }

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_adc_ch3)))
        {
            // select CH2 and send SOC signal
            outb(IN2 & start0, SPPCONTROLPORT);
            g_usleep(5);
            outb(IN2 & start1, SPPCONTROLPORT);
            g_usleep(5);
            outb(IN2 & start0, SPPCONTROLPORT);

            // wait for EOC signal (pin 11)
            //while((inb(SPPSTATUSPORT) & 128) == 1);
            g_usleep(100);

            input_data = inb(SPPDATAPORT);

            g_sprintf(str_buffer, "0x%1.2X", input_data);
            gtk_label_set_text(GTK_LABEL(label_adc_ch3_raw), str_buffer);
            g_sprintf(str_buffer, "%1.2f V", input_data/255.0*5);
            gtk_label_set_text(GTK_LABEL(label_adc_ch3_voltage), str_buffer);

            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_adc_ch3), input_data/255.0);
        }

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_adc_ch4)))
        {
            // select CH2 and send SOC signal
            outb(IN3 & start0, SPPCONTROLPORT);
            g_usleep(5);
            outb(IN3 & start1, SPPCONTROLPORT);
            g_usleep(5);
            outb(IN3 & start0, SPPCONTROLPORT);

            // wait for EOC signal (pin 11)
            //while((inb(SPPSTATUSPORT) & 128) == 1);
            g_usleep(100);

            input_data = inb(SPPDATAPORT);

            g_sprintf(str_buffer, "0x%1.2X", input_data);
            gtk_label_set_text(GTK_LABEL(label_adc_ch4_raw), str_buffer);
            g_sprintf(str_buffer, "%1.2f V", input_data/255.0*5);
            gtk_label_set_text(GTK_LABEL(label_adc_ch4_voltage), str_buffer);

            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_adc_ch4), input_data/255.0);
        }

        g_usleep(adc_timer_delay);

    }
    return NULL;
}

void generate_values()
{
    guint i;

    for(i = 0; i <= 255; i++)
        triangle_values[i] = (guint8) i;

    for(i = 256; i <= 511; i++)
        triangle_values[i] = 511 - i;

    for(i = 0; i <= 511; i++)
        sine_values[i] = (guint8) ceil( (sin(M_PI/256 * i) + 1) * 127.5);

    //g_print("i=%u, cos(c)=%f\n", i, (guint8) round( (cos(M_PI/256 * i) + 1) * 127.5) );
}

gboolean
adc_graph_configure_event (GtkWidget         *widget,
		                         GdkEventConfigure *event,
		                         gpointer           data_ptr)
{
    adc_graph_rect.x = 0;
    adc_graph_rect.y = 0;
    adc_graph_rect.width = widget->allocation.width;
    adc_graph_rect.height = widget->allocation.height;

    clear_graph();

	return TRUE;
}

gboolean
adc_graph_expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    cairo_t *cr;

    cr = gdk_cairo_create (widget->window);

    if(graph_image == NULL)
        draw_graph ();

    g_print("Expose!\n");
    cairo_rectangle (cr,
                         event->area.x, event->area.y,
                         event->area.width, event->area.height);
    cairo_clip (cr);

    // draw graph
    cairo_set_source_surface(cr, graph_image, 0.0, 0.0);
    cairo_paint(cr);

    cairo_destroy (cr);

    return TRUE;
}

// manually set the value of DAC output in Volts
void on_adj_dac_voltage_change (GtkAdjustment *adjustment, gpointer user_data)
{
    guint8 value;

    value = gtk_adjustment_get_value(adjustment);

    outb(value, SPPDATAPORT);

    gchar *str;
    str = g_strdup_printf("<b>%1.2f V</b>", value * 10.0 / 255);
    gtk_label_set_markup(GTK_LABEL(label_dac_voltage), str);
    g_free(str);
}

// manually set the value of DAC output in Volts
void on_adj_dac_duty_cycle_change (GtkAdjustment *adjustment, gpointer user_data)
{
    dac_duty_cycle = gtk_adjustment_get_value(adjustment) * 2 / 100;

    dac_timer_delay_on = timer_delay * dac_duty_cycle;
    dac_timer_delay_off = timer_delay*2 - dac_timer_delay_on;
}

// value in microseconds
void set_label_dac_period_value(guint value)
{
    gchar *str;

    if(value >= 1000000)
        str = g_strdup_printf("%1.2f s", value / 1000000.0);
    else if(value >= 1000)
        str = g_strdup_printf("%1.2f ms", value / 1000.0);
    else
        str = g_strdup_printf("%1.2f μs", (gdouble) value);

    gtk_label_set_markup(GTK_LABEL(label_dac_period), str);
    g_free(str);
}

// set the period of the current function (DAC)
void on_adj_dac_period_change (GtkAdjustment *adjustment, gpointer user_data)
{
    timer_delay = gtk_adjustment_get_value(adjustment);

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_square)))
        set_label_dac_period_value(timer_delay * 2);
    else
        set_label_dac_period_value(timer_delay * 512);

    dac_timer_delay_on = timer_delay * dac_duty_cycle;
    dac_timer_delay_off = timer_delay*2 - dac_timer_delay_on;

}

void dac_manual_set_sensitive (gboolean value)
{
    gtk_widget_set_sensitive(slider_dac_voltage, value);
    gtk_widget_set_sensitive(label_dac_voltage, value);
    gtk_widget_set_sensitive(label1, value);
}

void dac_function_set_sensitive (gboolean value)
{
    gtk_widget_set_sensitive(radio_square, value);
    gtk_widget_set_sensitive(radio_triangle, value);
    gtk_widget_set_sensitive(radio_sine, value);
    gtk_widget_set_sensitive(slider_dac_period, value);
    gtk_widget_set_sensitive(label_dac_period, value);
    gtk_widget_set_sensitive(label_dac_duty_cycle, value);
    gtk_widget_set_sensitive(spin_dac_duty_cycle, value);
}

void adc_channel_set_sensitive (gboolean value, guint8 ch)
{
    switch(ch)
    {
        case 1:
            gtk_widget_set_sensitive(label_adc_ch1_raw, value);
            gtk_widget_set_sensitive(label_adc_ch1_voltage, value);
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_adc_ch1), 0.0);
            gtk_label_set_text(GTK_LABEL(label_adc_ch1_raw), "0x00");
            gtk_label_set_text(GTK_LABEL(label_adc_ch1_voltage), "0,00 V");
            break;
        case 2:
            gtk_widget_set_sensitive(label_adc_ch2_raw, value);
            gtk_widget_set_sensitive(label_adc_ch2_voltage, value);
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_adc_ch2), 0.0);
            gtk_label_set_text(GTK_LABEL(label_adc_ch2_raw), "0x00");
            gtk_label_set_text(GTK_LABEL(label_adc_ch2_voltage), "0,00 V");
            break;
        case 3:
            gtk_widget_set_sensitive(label_adc_ch3_raw, value);
            gtk_widget_set_sensitive(label_adc_ch3_voltage, value);
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_adc_ch3), 0.0);
            gtk_label_set_text(GTK_LABEL(label_adc_ch3_raw), "0x00");
            gtk_label_set_text(GTK_LABEL(label_adc_ch3_voltage), "0,00 V");
            break;
        case 4:
            gtk_widget_set_sensitive(label_adc_ch4_raw, value);
            gtk_widget_set_sensitive(label_adc_ch4_voltage, value);
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_adc_ch4), 0.0);
            gtk_label_set_text(GTK_LABEL(label_adc_ch4_raw), "0x00");
            gtk_label_set_text(GTK_LABEL(label_adc_ch4_voltage), "0,00 V");
            break;
    }
}

void on_adc_ch_toggle (GtkToggleButton *togglebutton, gpointer user_data)
{
    gboolean state;

    state = gtk_toggle_button_get_active(togglebutton);

    adc_channel_set_sensitive(state, GPOINTER_TO_UINT(user_data));
}

void adc_set_sensitive (gboolean value)
{
    if(value == TRUE)
    {
        gtk_widget_set_sensitive(label_adc_raw, TRUE);
        gtk_widget_set_sensitive(label_adc_voltage, TRUE);

        gtk_widget_set_sensitive(check_adc_ch1, TRUE);
        gtk_widget_set_sensitive(check_adc_ch2, TRUE);
        gtk_widget_set_sensitive(check_adc_ch3, TRUE);
        gtk_widget_set_sensitive(check_adc_ch4, TRUE);

        on_adc_ch_toggle(GTK_TOGGLE_BUTTON(check_adc_ch1), GINT_TO_POINTER(1));
        on_adc_ch_toggle(GTK_TOGGLE_BUTTON(check_adc_ch2), GINT_TO_POINTER(2));
        on_adc_ch_toggle(GTK_TOGGLE_BUTTON(check_adc_ch3), GINT_TO_POINTER(3));
        on_adc_ch_toggle(GTK_TOGGLE_BUTTON(check_adc_ch4), GINT_TO_POINTER(4));

        gtk_widget_set_sensitive(check_adc_oscope, TRUE);
    }
    else
    {
        gtk_widget_set_sensitive(label_adc_raw, FALSE);
        gtk_widget_set_sensitive(label_adc_voltage, FALSE);
        gtk_widget_set_sensitive(check_adc_ch1, FALSE);
        gtk_widget_set_sensitive(check_adc_ch2, FALSE);
        gtk_widget_set_sensitive(check_adc_ch3, FALSE);
        gtk_widget_set_sensitive(check_adc_ch4, FALSE);

        adc_channel_set_sensitive(FALSE, 1);
        adc_channel_set_sensitive(FALSE, 2);
        adc_channel_set_sensitive(FALSE, 3);
        adc_channel_set_sensitive(FALSE, 4);

        gtk_widget_set_sensitive(check_adc_oscope, value);

    }
}

void on_dac_toggle (GtkToggleButton *togglebutton, gpointer user_data)
{
    if(gtk_toggle_button_get_active(togglebutton))
    {
        // stop data acquisition
        adc_timer_enable = FALSE;

        dac_manual_set_sensitive (TRUE);
        adc_set_sensitive (FALSE);
        gtk_widget_set_sensitive(check_dac_function, TRUE);
        outb(riposo, SPPCONTROLPORT);
        outb(DAC, SPPCONTROLPORT);

    }
    else
    {
        dac_manual_set_sensitive (FALSE);
        adc_set_sensitive (TRUE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_dac_function), FALSE);
        gtk_widget_set_sensitive(check_dac_function, FALSE);

        outb(0x374, SPPCONTROLPORT);
		outb(ADC, SPPCONTROLPORT);
        // start data acquisition
        adc_timer_enable = TRUE;
        g_thread_create(adc_timer_callback, NULL, TRUE, NULL);
    }
}

void on_dac_function_toggle (GtkToggleButton *togglebutton, gpointer user_data)
{
    if(gtk_toggle_button_get_active(togglebutton))
    {
        dac_manual_set_sensitive (FALSE);
        dac_function_set_sensitive (TRUE);

        dac_timer_enable = TRUE;

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_square)))
            g_thread_create(dac_timer_square, NULL, TRUE, NULL);
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_triangle)))
            g_thread_create(dac_timer_triangle, NULL, TRUE, NULL);
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_sine)))
            g_thread_create(dac_timer_sine, NULL, TRUE, NULL);
    }
    else
    {
        dac_manual_set_sensitive (TRUE);
        dac_function_set_sensitive (FALSE);

        dac_timer_enable = FALSE;
    }
}

void on_adc_oscope_toggle (GtkToggleButton *togglebutton, gpointer user_data)
{
    if(gtk_toggle_button_get_active(togglebutton))
    {
        gtk_widget_set_sensitive(label_adc_raw, FALSE);
        gtk_widget_set_sensitive(label_adc_voltage, FALSE);
        gtk_widget_set_sensitive(check_adc_ch1, FALSE);
        gtk_widget_set_sensitive(check_adc_ch2, FALSE);
        gtk_widget_set_sensitive(check_adc_ch3, FALSE);
        gtk_widget_set_sensitive(check_adc_ch4, FALSE);

        adc_channel_set_sensitive(FALSE, 1);
        adc_channel_set_sensitive(FALSE, 2);
        adc_channel_set_sensitive(FALSE, 3);
        adc_channel_set_sensitive(FALSE, 4);

        adc_timer_enable = FALSE;

        adc_oscope_enable = TRUE;
        g_thread_create(adc_oscope_callback, NULL, TRUE, NULL);
    }
    else
    {
        gtk_widget_set_sensitive(label_adc_raw, TRUE);
        gtk_widget_set_sensitive(label_adc_voltage, TRUE);
        gtk_widget_set_sensitive(check_adc_ch1, TRUE);
        gtk_widget_set_sensitive(check_adc_ch2, TRUE);
        gtk_widget_set_sensitive(check_adc_ch3, TRUE);
        gtk_widget_set_sensitive(check_adc_ch4, TRUE);

        on_adc_ch_toggle(GTK_TOGGLE_BUTTON(check_adc_ch1), GINT_TO_POINTER(1));
        on_adc_ch_toggle(GTK_TOGGLE_BUTTON(check_adc_ch2), GINT_TO_POINTER(2));
        on_adc_ch_toggle(GTK_TOGGLE_BUTTON(check_adc_ch3), GINT_TO_POINTER(3));
        on_adc_ch_toggle(GTK_TOGGLE_BUTTON(check_adc_ch4), GINT_TO_POINTER(4));

        adc_oscope_enable = FALSE;

        adc_timer_enable = TRUE;
        g_thread_create(adc_timer_callback, NULL, TRUE, NULL);
    }
}


void on_dac_func_toggle (GtkToggleButton *togglebutton, gpointer user_data)
{
    dac_timer_enable = FALSE;

    g_usleep(10000);

    dac_timer_enable = TRUE;

    switch(GPOINTER_TO_UINT(user_data))
    {
        case 1:
            set_label_dac_period_value(timer_delay * 2);
            gtk_adjustment_set_upper(adj_dac_period, 1000000);
            g_thread_create(dac_timer_square, NULL, TRUE, NULL);
            break;
        case 2:
            if(timer_delay > 500) {
                gtk_adjustment_set_value(adj_dac_period, 500);
                timer_delay = 500;
            }
            set_label_dac_period_value(timer_delay * 512);
            gtk_adjustment_set_upper(adj_dac_period, 500);
            g_thread_create(dac_timer_triangle, NULL, TRUE, NULL);
            break;
        case 3:
            if(timer_delay > 500) {
                gtk_adjustment_set_value(adj_dac_period, 500);
                timer_delay = 500;
            }
            set_label_dac_period_value(timer_delay * 512);
            gtk_adjustment_set_upper(adj_dac_period, 500);
            g_thread_create(dac_timer_sine, NULL, TRUE, NULL);
            break;
    }


}

int main(int argc, char *argv[])
{
    GtkBuilder* builder;
    GError* error = NULL;

    gtk_init(&argc, &argv);

    builder = gtk_builder_new ();
    if (!gtk_builder_add_from_file (builder, "./portio.glade", &error))
    {
        g_error ("Couldn't load builder file: %s", error->message);
        g_error_free (error);
    }
    main_window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    label1 = GTK_WIDGET (gtk_builder_get_object (builder, "label1"));
    label_dac_voltage = GTK_WIDGET (gtk_builder_get_object (builder, "label_dac_voltage"));
    slider_dac_voltage = GTK_WIDGET (gtk_builder_get_object (builder, "slider_dac_voltage"));
    slider_dac_period = GTK_WIDGET (gtk_builder_get_object (builder, "slider_dac_period"));
    check_dac_function = GTK_WIDGET (gtk_builder_get_object (builder, "check_dac_function"));
    adj_dac_period = GTK_ADJUSTMENT (gtk_builder_get_object (builder, "adj_dac_period"));
    label_dac_period = GTK_WIDGET (gtk_builder_get_object (builder, "label_dac_period"));
    radio_square = GTK_WIDGET (gtk_builder_get_object (builder, "radioSquare"));
    radio_triangle = GTK_WIDGET (gtk_builder_get_object (builder, "radioTriangle"));
    radio_sine = GTK_WIDGET (gtk_builder_get_object (builder, "radioSine"));
    label_dac_duty_cycle = GTK_WIDGET (gtk_builder_get_object (builder, "label_dac_duty_cycle"));
    spin_dac_duty_cycle = GTK_WIDGET (gtk_builder_get_object (builder, "dac_duty_cycle"));

    label_adc_raw = GTK_WIDGET (gtk_builder_get_object (builder, "label_adc_raw"));
    label_adc_voltage = GTK_WIDGET (gtk_builder_get_object (builder, "label_adc_voltage"));
    check_adc_ch1 = GTK_WIDGET (gtk_builder_get_object (builder, "check_adc_ch1"));
    check_adc_ch2 = GTK_WIDGET (gtk_builder_get_object (builder, "check_adc_ch2"));
    check_adc_ch3 = GTK_WIDGET (gtk_builder_get_object (builder, "check_adc_ch3"));
    check_adc_ch4 = GTK_WIDGET (gtk_builder_get_object (builder, "check_adc_ch4"));
    label_adc_ch1_raw = GTK_WIDGET (gtk_builder_get_object (builder, "label_adc_ch1_raw"));
    label_adc_ch2_raw = GTK_WIDGET (gtk_builder_get_object (builder, "label_adc_ch2_raw"));
    label_adc_ch3_raw = GTK_WIDGET (gtk_builder_get_object (builder, "label_adc_ch3_raw"));
    label_adc_ch4_raw = GTK_WIDGET (gtk_builder_get_object (builder, "label_adc_ch4_raw"));
    label_adc_ch1_voltage = GTK_WIDGET (gtk_builder_get_object (builder, "label_adc_ch1_voltage"));
    label_adc_ch2_voltage = GTK_WIDGET (gtk_builder_get_object (builder, "label_adc_ch2_voltage"));
    label_adc_ch3_voltage = GTK_WIDGET (gtk_builder_get_object (builder, "label_adc_ch3_voltage"));
    label_adc_ch4_voltage = GTK_WIDGET (gtk_builder_get_object (builder, "label_adc_ch4_voltage"));
    progress_adc_ch1 = GTK_WIDGET (gtk_builder_get_object (builder, "progress_adc_ch1"));
    progress_adc_ch2 = GTK_WIDGET (gtk_builder_get_object (builder, "progress_adc_ch2"));
    progress_adc_ch3 = GTK_WIDGET (gtk_builder_get_object (builder, "progress_adc_ch3"));
    progress_adc_ch4 = GTK_WIDGET (gtk_builder_get_object (builder, "progress_adc_ch4"));
    check_adc_oscope = GTK_WIDGET (gtk_builder_get_object (builder, "check_adc_oscope"));
    adc_graph = GTK_WIDGET (gtk_builder_get_object (builder, "adc_graph"));
    adc_graph_window = gtk_widget_get_window(adc_graph);

    /*gtk_widget_destroy(adc_graph);


    GtkWidget *graph;
    UberRange ch_range = { 0., 100., 100. };

    graph = uber_graph_new();

    gtk_container_add(gtk_builder_get_object (builder, "alignment4"), graph);

	uber_graph_set_show_xlabel(UBER_GRAPH(graph), TRUE);
	uber_graph_set_format(UBER_GRAPH(graph), UBER_GRAPH_PERCENT);
	uber_graph_set_yautoscale(UBER_GRAPH(graph), FALSE);
	uber_graph_set_yrange(UBER_GRAPH(graph), &ch_range);
	//uber_graph_add_line(UBER_GRAPH(cpu_graph));
	//SET_LINE_COLOR(cpu_graph, 1, "#2e3436");
	uber_graph_set_value_func(UBER_GRAPH(graph), get_ch, NULL, NULL);*/


    g_signal_connect(G_OBJECT(main_window), "delete-event",
                         G_CALLBACK(gtk_main_quit), NULL);

    g_signal_connect(gtk_builder_get_object (builder, "adj_dac_voltage"), "value-changed",
                         G_CALLBACK(on_adj_dac_voltage_change), NULL);
    g_signal_connect(gtk_builder_get_object (builder, "adj_dac_period"), "value-changed",
                         G_CALLBACK(on_adj_dac_period_change), NULL);
    g_signal_connect(gtk_builder_get_object (builder, "adj_dac_duty_cycle"), "value-changed",
                         G_CALLBACK(on_adj_dac_duty_cycle_change), NULL);
    g_signal_connect(gtk_builder_get_object (builder, "check_dac"), "toggled",
                         G_CALLBACK(on_dac_toggle), NULL);
    g_signal_connect(gtk_builder_get_object (builder, "check_dac_function"), "toggled",
                         G_CALLBACK(on_dac_function_toggle), NULL);

    g_signal_connect(gtk_builder_get_object (builder, "radioSquare"), "toggled",
                         G_CALLBACK(on_dac_func_toggle), GINT_TO_POINTER(1));
    g_signal_connect(gtk_builder_get_object (builder, "radioTriangle"), "toggled",
                         G_CALLBACK(on_dac_func_toggle), GINT_TO_POINTER(2));
    g_signal_connect(gtk_builder_get_object (builder, "radioSine"), "toggled",
                         G_CALLBACK(on_dac_func_toggle), GINT_TO_POINTER(3));

    g_signal_connect(gtk_builder_get_object (builder, "check_adc_ch1"), "toggled",
                         G_CALLBACK(on_adc_ch_toggle), GINT_TO_POINTER(1));
    g_signal_connect(gtk_builder_get_object (builder, "check_adc_ch2"), "toggled",
                         G_CALLBACK(on_adc_ch_toggle), GINT_TO_POINTER(2));
    g_signal_connect(gtk_builder_get_object (builder, "check_adc_ch3"), "toggled",
                         G_CALLBACK(on_adc_ch_toggle), GINT_TO_POINTER(3));
    g_signal_connect(gtk_builder_get_object (builder, "check_adc_ch4"), "toggled",
                         G_CALLBACK(on_adc_ch_toggle), GINT_TO_POINTER(4));

    g_signal_connect(gtk_builder_get_object (builder, "check_adc_oscope"), "toggled",
                         G_CALLBACK(on_adc_oscope_toggle), NULL);

    g_signal_connect (gtk_builder_get_object (builder, "adc_graph"), "expose_event",
                    G_CALLBACK (adc_graph_expose_event), NULL);
    g_signal_connect (gtk_builder_get_object (builder, "adc_graph"), "configure_event",
                    G_CALLBACK (adc_graph_configure_event), NULL);



    if(ioperm(SPPDATAPORT, 3, 1))
	{
		perror("ioperm");
		return 1;
	}


	outb(0x00, SPPDATAPORT);

	outb(riposo, SPPCONTROLPORT);
    outb(DAC, SPPCONTROLPORT);

    generate_values();

    adc_set_sensitive(FALSE);
    dac_function_set_sensitive (FALSE);

    gtk_widget_show_all(main_window);

    gtk_main();

    outb(0x00, SPPDATAPORT);
    outb(riposo, SPPCONTROLPORT);

	return 0;
}


