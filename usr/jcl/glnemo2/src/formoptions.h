// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2010                                  
// e-mail:   Jean-Charles.Lambert@oamp.fr                                      
// address:  Dynamique des galaxies                                            
//           Laboratoire d'Astrophysique de Marseille                          
//           P�le de l'Etoile, site de Ch�teau-Gombert                         
//           38, rue Fr�d�ric Joliot-Curie                                     
//           13388 Marseille cedex 13 France                                   
//           CNRS U.M.R 6110                                                   
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".        
// ============================================================================
/**
	@author Jean-Charles Lambert <jean-charles.lambert@oamp.fr>
 */
#ifndef GLNEMOFORMOPTIONS_H
#define GLNEMOFORMOPTIONS_H
#include <QTime>
#include <QTimer>
#include <QColorDialog>
#include <iostream>
#include "ui_formoptions.h"
#include "globaloptions.h"
namespace glnemo {

class FormOptions: public QDialog {
  Q_OBJECT
  public:
    FormOptions(GlobalOptions * ,QWidget *parent = 0);
    ~FormOptions();
  public slots:
    void update();
    void updateFrame(const int, const int);
    void updateParticlesSelect(const int);
  private:
    Ui::FormOptions form;
    GlobalOptions * go;
    bool start;
    QTime time;
    QTimer * limited_timer;
    static int windows_size[][2];
  private slots:
    void leaveEvent ( QEvent * event ) {
      if (event) {;}
      emit leaveEvent();
    }
    //                       
    // bench tab             
    void on_bench_button_pressed();
    void stop_bench();
    // interactive select tab
    void on_save_button_pressed()    { emit save_selected(); }
    void on_obj_button_pressed()     { emit create_obj_selected(); }
    void on_sel_zoom_radio_pressed() { emit select_and_zoom(true);}
    void on_sel_only_radio_pressed() { emit select_and_zoom(false);}
    //                     
    // camera selection tab
    void on_cam_pts_display_clicked() { 
      emit setCamDisplay(form.cam_pts_display->isChecked(),
                         form.cam_path_display->isChecked());
    }
    void on_cam_path_display_clicked() { 
      emit setCamDisplay(form.cam_pts_display->isChecked(),
                         form.cam_path_display->isChecked());
    }
    void on_spline_points_valueChanged(int v) {
      emit setSplineParam(v,form.spline_scale->value());
    }
    void on_spline_scale_valueChanged(double v) {
      emit setSplineParam(form.spline_points->value(),v);
    }
    void on_cam_play_pressed();
    //                   
    // play selection tab
    void on_play_pressed();
    void on_com_clicked() { go->auto_com = form.com->isChecked();}
    //                   
    // auto-screenshot selection tab
    void on_frame_name_pressed();
    void on_reset_pressed() { go->frame_index = 0; }
    void on_auto_save_clicked() { go->auto_play_screenshot = form.auto_save->isChecked();}
    void on_auto_save_gl_clicked() { go->auto_gl_screenshot = form.auto_save_gl->isChecked();}
    void on_screen_size_activated(int index);
    void on_frame_png_clicked() { go->base_frame_ext = "png";}
    void on_frame_jpg_clicked() { go->base_frame_ext = "jpg";}
    // OSD tab
    //
    void on_show_osd_checkb_clicked(bool b) {
      go->show_osd = b;
      emit update_osd(true);
    }
    void on_osd_time_clicked(bool b) {
      go->osd_time = b;
      emit update_osd(true);
    }
    void on_osd_nbody_clicked(bool b) {
      go->osd_nbody = b;
      emit update_osd(true);
    }
    void on_osd_title_clicked(bool b) {
      go->osd_title = b;
      emit update_osd(true);
    }
    void on_osd_datatype_clicked(bool b) {
      go->osd_data_type = b;
      emit update_osd(true);
    }
    void on_osd_zoom_clicked(bool b) {
      go->osd_zoom = b;
      emit update_osd(true);
    }
    void on_osd_trans_clicked(bool b) {
      go->osd_trans = b;
      emit update_osd(true);
    }
    void on_osd_rot_clicked(bool b) {
      go->osd_rot = b;
      emit update_osd(true);
    }
    void on_spin_font_size_valueChanged(double value) {
      go->osd_font_size = (float ) value;
      emit update_osd_font();
    }
    // change font color button
    void on_font_color_clicked() {
      QPalette pal = form.font_color->palette();
      QColor color=QColorDialog::getColor(pal.color(QPalette::Button));
      QString css=QString("background:rgb(%1,%2,%3)").
                  arg(color.red()).
                  arg(color.green()).
                  arg(color.blue());  
      form.font_color->setStyleSheet(css); 
      go->osd_color = color;
      emit update_osd_font();
    }
    // change background color button
    void on_background_color_clicked() {
      QPalette pal = form.font_color->palette();
      QColor color=QColorDialog::getColor(pal.color(QPalette::Button));
      QString css=QString("background:rgb(%1,%2,%3)").
                  arg(color.red()).
                  arg(color.green()).
                  arg(color.blue());  
      form.background_color->setStyleSheet(css); 
      go->background_color = color;
      emit update_gl();
    }
    // change title
    void on_title_name_returnPressed() {
      go->osd_title_name = form.title_name->text();
      emit update_osd(true);
    }

    //
    // grids tab
    void on_show_grid_checkb_clicked(bool b) {
      go->show_grid = b;
      emit update_grid();
    }
    void on_xy_checkb_clicked(bool b) {
      go->xy_grid = b;
      emit update_grid();
    }
    void on_yz_checkb_clicked(bool b) {
      go->yz_grid = b;
      emit update_grid();
    }
    void on_xz_checkb_clicked(bool b) {
      go->xz_grid = b;
      emit update_grid();
    }
    void on_mesh_length_spin_valueChanged(double d) {
      go->mesh_length = d;
      emit rebuild_grid();
    }
    void on_mesh_nb_spin_valueChanged(int d) {
      go->nb_meshs = d;
      emit rebuild_grid();
    }
    void on_cube_checkb_clicked(bool b) {
      go->show_cube = b;
      emit update_grid();
    }
    // change xy grid color button
    void on_xy_grid_color_clicked() {
      QPalette pal = form.xy_grid_color->palette();
      QColor color=QColorDialog::getColor(pal.color(QPalette::Button));
      QString css=QString("background:rgb(%1,%2,%3)").
                  arg(color.red()).
                  arg(color.green()).
                  arg(color.blue());  
      form.xy_grid_color->setStyleSheet(css);
      form.xy_grid_color->setPalette(pal);
      go->col_x_grid = color;
      emit update_grid();
    }
    // change yz grid color button
    void on_yz_grid_color_clicked() {
      QPalette pal = form.yz_grid_color->palette();
      QColor color=QColorDialog::getColor(pal.color(QPalette::Button));
      QString css=QString("background:rgb(%1,%2,%3)").
                  arg(color.red()).
                  arg(color.green()).
                  arg(color.blue());  
      form.yz_grid_color->setStyleSheet(css);
      go->col_y_grid = color;
      emit update_grid();
    }
    // change xz grid color button
    void on_xz_grid_color_clicked() {
      QPalette pal = form.xz_grid_color->palette();
      QColor color=QColorDialog::getColor(pal.color(QPalette::Button));
      QString css=QString("background:rgb(%1,%2,%3)").
                  arg(color.red()).
                  arg(color.green()).
                  arg(color.blue());  
      form.xz_grid_color->setStyleSheet(css);  
      go->col_z_grid = color;
      emit update_grid();
    }
    // change cube color button
    void on_cube_color_clicked() {
      QPalette pal = form.cube_color->palette();
      QColor color=QColorDialog::getColor(pal.color(QPalette::Button));
      QString css=QString("background:rgb(%1,%2,%3)").
                  arg(color.red()).
                  arg(color.green()).
                  arg(color.blue());  
      form.cube_color->setStyleSheet(css);  
      go->col_cube = color;
      emit update_grid();
    }
  signals:
    void start_bench(const bool);
    void select_and_zoom(const bool);
    void save_selected();
    void create_obj_selected();
    void leaveEvent();
    void update_grid();
    void rebuild_grid();
    void update_osd(bool b);
    void update_osd_font();
    void update_gl();
    //           
    // camera tab
    void setCamDisplay(const bool, const bool);
    void setSplineParam(const int, const double);
    void startStopPlay();
    //         
    // play tab
    void playPressed();
};

}

#endif
