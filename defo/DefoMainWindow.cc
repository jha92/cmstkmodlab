#include <QGroupBox>

#include "DefoMainWindow.h"

DefoMainWindow::DefoMainWindow(QWidget *parent) :
    QMainWindow(parent)
{
  // Set up current directory for saving images
  QDateTime dt = QDateTime::currentDateTimeUtc();
  QString measurementDirPath("/home/tkmodlab/Desktop/measurements/%1-%2/");
  measurementDirPath = measurementDirPath.arg(dt.toString("yyyyMMdd"));
  
  int i = 1;
  do {
    currentDir_.setPath( measurementDirPath.arg(i) );
    ++i;
  } while ( currentDir_.exists() );

  // CONRAD MODEL
  conradModel_ = new DefoConradModel(this);

  // JULABO MODEL
  julaboModel_ = new DefoJulaboModel(5);

  // KEITHLEY MODEL
  keithleyModel_ = new DefoKeithleyModel(60);

  // MEASUREMENT MODEL
  listModel_ = new DefoMeasurementListModel();
  selectionModel_ = new DefoMeasurementSelectionModel();

  // CANON CAMERA MODEL
  cameraModel_ = new DefoCameraModel(this);
  connect(cameraModel_, SIGNAL(newImage(QString,bool)), SLOT(newCameraImage(QString,bool)));

  // POINT MODEL
  pointModel_ = new DefoPointRecognitionModel(this);

  // SCRIPT MODEL
  scriptModel_ = new DefoScriptModel(
        conradModel_
      , cameraModel_
      , this
  );

  tabWidget_ = new QTabWidget(this);
  tabWidget_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  QBoxLayout *layout = new QHBoxLayout();
  QWidget *powerAndControlWidget = new QWidget(tabWidget_);
  powerAndControlWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  powerAndControlWidget->setLayout(layout);

  QGroupBox * groupbox = new QGroupBox("Power Supplies", powerAndControlWidget);
  // CONRAD MODEL
  DefoConradWidget* conradWidget = new DefoConradWidget(conradModel_);
  conradWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget(conradWidget);
  vbox->addStretch(1);
  groupbox->setLayout(vbox);
  layout->addWidget(groupbox);

  QBoxLayout * vlayout = new QVBoxLayout();
  QWidget *widget = new QWidget(powerAndControlWidget);
  widget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  widget->setLayout(vlayout);

  groupbox = new QGroupBox("Chiller", widget);
  // JULABO MODEL
  DefoJulaboWidget* julaboWidget = new DefoJulaboWidget(julaboModel_);
  julaboWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  vbox = new QVBoxLayout;
  vbox->addWidget(julaboWidget);
  vbox->addStretch(1);
  groupbox->setLayout(vbox);

  vlayout->addWidget(groupbox);

  groupbox = new QGroupBox("Script", widget);
  DefoScriptWidget* scriptWidget = new DefoScriptWidget(scriptModel_);
  scriptWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  vbox = new QVBoxLayout;
  vbox->addWidget(scriptWidget);
  vbox->addStretch(1);
  groupbox->setLayout(vbox);

  vlayout->addWidget(groupbox);

  layout->addWidget(widget);

  tabWidget_->addTab(powerAndControlWidget, "Power and Control");

  layout = new QHBoxLayout();
  QWidget *measurementWidget = new QWidget(tabWidget_);
  measurementWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  measurementWidget->setLayout(layout);

  layout->addWidget(
        new DefoCameraWidget(
          cameraModel_
          , conradModel_
          , listModel_
          , selectionModel_
          , this
          )
        );

  if (cameraModel_->getDeviceState() == READY) {
    cameraModel_->setOptionSelection(DefoCameraModel::APERTURE, 6);
    cameraModel_->setOptionSelection(DefoCameraModel::ISO, 1);
    cameraModel_->setOptionSelection(DefoCameraModel::SHUTTER_SPEED, 26);
  }

  // read default settings
  DefoConfigReader cfgReader( "defo.cfg" );
  pointModel_->setThresholdValue(
        DefoPointRecognitionModel::THRESHOLD_1
        , cfgReader.getValue<int>( "STEP1_THRESHOLD" )
        );
  pointModel_->setThresholdValue(
        DefoPointRecognitionModel::THRESHOLD_2
        , cfgReader.getValue<int>( "STEP2_THRESHOLD" )
        );
  pointModel_->setThresholdValue(
        DefoPointRecognitionModel::THRESHOLD_3
        , cfgReader.getValue<int>( "STEP3_THRESHOLD" )
        );
  pointModel_->setHalfSquareWidth(
        cfgReader.getValue<int>( "HALF_SQUARE_WIDTH" )
        );


  // POINT RECOGNITION TAB
  QTabWidget* tabs = new QTabWidget(this);
  //  tabs->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  layout->addWidget(tabs);

  // THRESHOLDS
  QWidget* thresholds = new QWidget(tabs);
  QVBoxLayout* thresholdsLayout = new QVBoxLayout(thresholds);
  thresholds->setLayout(thresholdsLayout);
  tabs->addTab(thresholds, "Thresholds");

  thresholdsLayout->addWidget(
      new DefoImageThresholdsWidget(selectionModel_, pointModel_, thresholds)
  );

  QWidget* thresholdSpinners = new QWidget(thresholds);
  thresholdsLayout->addWidget(thresholdSpinners);
  thresholdSpinners->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  QHBoxLayout* thresholdLayout = new QHBoxLayout(thresholdSpinners);
  thresholdSpinners->setLayout(thresholdLayout);

  QString format("Threshold %1");
  for (int i=0; i<3; i++) {
    thresholdLayout->addWidget(new QLabel(format.arg(i+1)));
    thresholdLayout->addWidget(new DefoThresholdSpinBox(
                                 pointModel_
                                 , static_cast<DefoPointRecognitionModel::Threshold>(i)
                                 ));
  }

  // POINT FINDING
  QWidget* points = new QWidget(tabs);
  tabs->addTab(points, "Points");
  QVBoxLayout* pointsLayout = new QVBoxLayout(points);
  points->setLayout(pointsLayout);

  DefoImagePointsWidget * pointsImage = new DefoImagePointsWidget(listModel_, selectionModel_, points);
  pointsLayout->addWidget(pointsImage);
  QPushButton* findPoints = new QPushButton("Find &points", points);
  pointsLayout->addWidget(findPoints);
  //  pointsLayout->addWidget(
  //        new DefoMeasurementListComboBox(listModel_, selectionModel_, points)
  //  );

  connect(findPoints, SIGNAL(clicked()), SLOT(pointButtonClicked()));

  QPushButton* savePoints = new QPushButton("Save points", points);
  pointsLayout->addWidget(savePoints);
  connect(savePoints, SIGNAL(clicked()), SLOT(writePoints()));

  tabWidget_->addTab(measurementWidget, "Measurement");

  layout = new QHBoxLayout();
  QWidget *temperatureWidget = new QWidget(tabWidget_);
  temperatureWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  temperatureWidget->setLayout(layout);

  // KEITHLEY MODEL
  layout->addWidget( new DefoKeithleyWidget(keithleyModel_) );

  tabWidget_->addTab(temperatureWidget, "Temperature");

////  photographer = new Photographer(listModel_, cameraModel_, 5);

  setCentralWidget(tabWidget_);
}
