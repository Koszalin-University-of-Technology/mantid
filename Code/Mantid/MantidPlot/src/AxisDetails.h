/**
    This class holds the widgets that hold the details for each axis so the contents are only filled once and switching axis only changes a pointer.

    @author Keith Brown, Placement Student at ISIS Rutherford Appleton Laboratory from the University of Derby
    @date 13/03/2009

    Copyright &copy; 2009 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://github.com/mantidproject/mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>
*/

#ifndef AXISDETAILS_H_
#define AXISDETAILS_H_

#include <QWidget>
class AxisAxisDetails: public QWidget
{
    Q_OBJECT
    //details for each axis in the Axis tab
  public:
    AxisAxisDetails(QWidget *parent = 0); // populate and fill in with existing data
    ~AxisAxisDetails();
};

class ScaleAxisDetails: public QWidget
{
    Q_OBJECT
    //details for each axis in the Scale Tab
  public:
    ScaleAxisDetails(QWidget *parent = 0); // populate and fill in with existing data
    ~ScaleAxisDetails();


  private:
    DoubleSpinBox *dspnEnd, *dspnStart, *dspnStep, *dspnBreakStart, *dspnBreakEnd, *dspnStepBeforeBreak, *dspnStepAfterBreak; //formerly  *boxEnd, *boxStart, *boxStep, *boxBreakStart, *boxBreakEnd, *boxStepBeforeBreak, *boxStepAfterBreak;

    QCheckBox *chkInvert, *chkLog10AfterBreak, *chkBreakDecoration; //formerly *btnInvert, *boxLog10AfterBreak, *boxBreakDecoration;

    QRadioButton *radStep, *radMajor; //formerly *btnStep,*btnMajor

    QSpinBox *spnMajorValue, *spnBreakPosition, *spnBreakWidth; //formerly *boxMajorValue, *boxBreakPosition, *boxBreakWidth;

    QGroupBox *grpAxesBreaks; //formerly *boxAxesBreaks;

    QComboBox *cmbMinorTicksBeforeBreak, *cmbMinorTicksAfterBreak, *cmbScaleType, *cmbMinorValue, *cmbUnit; //formerly *boxMinorTicksBeforeBreak, *boxMinorTicksAfterBreak, *boxScaleType, *boxMinorValue, *boxUnit;

    QLabel *lblScaleTypeLabel, *lblMinorBox; //formerly *boxScaleTypeLabel, *minorBoxLabel;

    QDateTimeEdit *dteStartDateTime, *dteEndDateTime; //formerly *boxStartDateTime, *boxEndDateTime;

    QTimeEdit *timStartTime, *timEndTime; //formerly *boxStartTime, *boxEndTime;
};

#endif /* AXISDETAILS_H_ */
