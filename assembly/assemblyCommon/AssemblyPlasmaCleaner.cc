/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//               Copyright (C) 2011-2017 - The DESY CMS Group                  //
//                           All rights reserved                               //
//                                                                             //
//      The CMStkModLab source code is licensed under the GNU GPL v3.0.        //
//      You have the right to modify and/or redistribute this source code      //
//      under the terms specified in the license, which may be found online    //
//      at http://www.gnu.org/licenses or at License.txt.                      //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <iostream>
#include <unistd.h>

#include <QFormLayout>
#include <QFileDialog>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QTimer>
#include <QLabel>
#include <QApplication>
#include <QProgressBar>
#include <sstream>
#include <iostream>
#include <nqlogger.h>
#include "AssemblyPlasmaCleaner.h"


//relay card
#include "../../devices/Conrad/ConradCommunication.h"
#include "../../devices/Conrad/ConradController.h"

#include "../../devices/Arduino/ArduinoMotor.h"

using namespace std;

AssemblyPlasmaCleaner::AssemblyPlasmaCleaner(QWidget *parent)
    : QWidget(parent)
{

 
  
//   bool comm = motor->IsCommunication();
//   cout <<"comms on?"<<  comm<<"   "   <<endl;
 
    //    for (int t = 0; t < 20; t++){

      //      cout << " pressure A " << motor->GetPressureA()  << endl; 
    //}

      
    QGridLayout *l = new QGridLayout(this);
    setLayout(l);

    QGridLayout *g0 = new QGridLayout(this);
    l->addLayout(g0,0,0);   

    QPalette palette;
    palette.setColor(QPalette::Background, QColor(220, 220, 220));
    
    //QPushButton *btn1 = new QPushButton(this);
    //btn1->setText("Upload Firmware");
    //g0->addWidget(btn1,0,0);

    CleanerCommander * cmdr1 = new CleanerCommander(this, "Scan",20,50,200,8);
    g0->addWidget(cmdr1,0,0);

    //ql = new QLabel("", this);
    //g0->addWidget(ql,0,1);
    
    //    QPixmap pixmap(100,100);
    //pixmap.fill(QColor("transparent"));
    
    //QPainter painter(&pixmap);
    //painter.setBrush(QBrush(Qt::red));
    //painter.drawEllipse(0, 0, 30, 30);
    
    //ql->setPixmap(pixmap);
    //ql->setAlignment(Qt::AlignCenter);
    //ql->setText("READY");
    //ql->setStyleSheet("QLabel { background-color : orange; color : black; }");
     
}

CleanerCommander::CleanerCommander(QWidget *parent, std::string string, double target_speed, double max_accel, double scan_distance, double steps_per_mm)
: QWidget(parent)
{
    QGridLayout *l = new QGridLayout(this);
    setLayout(l);

    NQLog("CleanerCommander") << ": in mode";
    motor = new ArduinoMotor("/dev/cu.usbmodem641");
    bool comm = motor->IsCommunication();
    
 
    
    std::ostringstream strs;
    strs.str("");
    strs.clear();
    strs << target_speed;
    target_speed_str = strs.str();
    strs.str("");
    strs.clear();
    strs << max_accel;
    max_accel_str = strs.str();
    strs.str("");
    strs.clear();
    strs << scan_distance;
    scan_distance_str = strs.str();
    strs.str("");
    strs.clear();
    strs << steps_per_mm;
    steps_per_mm_str = strs.str();
    strs.str("");
    strs.clear();


    QString target_speed_qstr = QString::fromStdString(target_speed_str);
    QString max_accel_qstr = QString::fromStdString(max_accel_str);
    QString scan_distance_qstr = QString::fromStdString(scan_distance_str);
    QString steps_per_mm_qstr = QString::fromStdString(steps_per_mm_str);

    QString qname = QString::fromStdString(string);
    
    
    button1 = new QPushButton(qname, this);
    l->addWidget(button1,0,0);
    
    ql_0 = new QLabel("Arduino comm. OFF", this);  
    l->addWidget(ql_0,0,1);

    QPixmap pixmap(100,100);
    pixmap.fill(QColor("transparent"));

    QPainter painter(&pixmap);
    painter.setBrush(QBrush(Qt::red));

    ql_0->setPixmap(pixmap);
    ql_0->setText("Controller communication OFF");
    ql_0->setStyleSheet("QLabel { background-color : red; color : black; }");

    if (comm == TRUE){
      ql_0->setText("Controller communitcation ON");
      ql_0->setStyleSheet("QLabel { background-color : green; color : black; }");
}


    
    button2 = new QPushButton("Send free text command", this);
    l->addWidget(button2,1,0);
    
    lineEdit1 = new QLineEdit();
    lineEdit1->setText("#");
    l->addWidget(lineEdit1,1,1);
    
    
    ql_2 = new QLabel("target speed (mm/s)", this);
    l->addWidget(ql_2,2,0);

    lineEdit2 = new QLineEdit();
    lineEdit2->setText(target_speed_qstr);
    l->addWidget(lineEdit2,2,1);

    ql_3 = new QLabel("Ramp up/down distance (mm)", this);
    l->addWidget(ql_3,3,0);

    lineEdit3 = new QLineEdit();
    lineEdit3->setText(max_accel_qstr);
    l->addWidget(lineEdit3,3,1);

    ql_4 = new QLabel("scan distance (mm)", this);
    l->addWidget(ql_4,4,0);

    lineEdit4 = new QLineEdit();
    lineEdit4->setText(scan_distance_qstr);
    l->addWidget(lineEdit4,4,1);

    ql_5 = new QLabel("steps per mm (mm^-1)", this);
    l->addWidget(ql_5,5,0);

    lineEdit5 = new QLineEdit();
    lineEdit5->setText(steps_per_mm_qstr);
    l->addWidget(lineEdit5,5,1);

    light = new LightWidget(Qt::red);
    l->addWidget(light,6,0);
    light->setOn(true);


    progressBar = new QProgressBar();
    progressBar->setMinimum(0);
    progressBar->setMaximum(99);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    l->addWidget(progressBar,6,1);

    connect(button1, SIGNAL(clicked()),
          this, SLOT(sendCommand()));
    
    connect(button2, SIGNAL(clicked()),
            this, SLOT(sendFreeCommand()));
}





void CleanerCommander::sendFreeCommand(){
    
    std::string str_5 =  lineEdit5->text().toStdString();

    std::cout << "[CleanerCommander::checkText]. "   <<  str_5 <<std::endl;
    motor->SendCommand(str_5.c_str());
    
}


void CleanerCommander::sendCommand(){
std::cout << "[CleanerCommander::sendCommand]."   << str <<std::endl;

//    stringstream convert;
//   convert << steps;
//   const char* cmd = convert.str().c_str();

 std::string str_1 =  lineEdit2->text().toStdString();
 std::string str_2 =  lineEdit3->text().toStdString();
 std::string str_3 =  lineEdit4->text().toStdString();
 std::string str_4 =  lineEdit5->text().toStdString();
 std::string comm = str_1 + "," + str_2 + ":" +  str_3 + "," +  str_4;



 float fl_1 = stof(str_1);
 float fl_2 = stof(str_2);
 float fl_3 = stof(str_3);
 float fl_4 = stof(str_4);

 scan_time = fl_3/fl_1;

 // std::cout << "[CleanerCommander::checkText]. "   << comm <<", scan time =   "<< scan_time <<std::endl;
 //motor->SendCommand(comm.c_str());
 sec = 0;
 progress = 0;
 QTimer *timer = new QTimer(this);
 connect(timer, SIGNAL(timeout()), this, SLOT(update()));
 timer->start(1000);

}


void CleanerCommander::update(){
  sec++;
  progress = progress + (100/24.51); 

  std::cout << "[CleanerCommander::update]. " << progress <<std::endl;
  progressBar->setValue(progress);


}


    
  







