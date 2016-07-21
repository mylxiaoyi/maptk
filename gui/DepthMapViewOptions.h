/*ckwg +29
 * Copyright 2016 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither the name Kitware, Inc. nor the names of any contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MAPTK_DEPTHMAPVIEWOPTIONS_H_
#define MAPTK_DEPTHMAPVIEWOPTIONS_H_

#include <qtGlobal.h>

#include <QtGui/QButtonGroup>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class vtkActor;
class vtkPointData;

class DepthMapViewOptionsPrivate;

class DepthMapViewOptions : public QWidget
{
  Q_OBJECT

public:
  explicit DepthMapViewOptions(const QString& settingsGroup,
                               QWidget* parent = 0, Qt::WindowFlags flags = 0);
  virtual ~DepthMapViewOptions();

  void setActor(vtkActor*);

signals:
  void modified();

public slots:
  void updateRanges(vtkPointData*);
  void updateActor();

protected slots:
  void setDepthIcon(QIcon const&);
  void setBestCostValueIcon(QIcon const&);
  void setUniquenessRatioIcon(QIcon const&);

private:
  QTE_DECLARE_PRIVATE_RPTR(DepthMapViewOptions)
  QTE_DECLARE_PRIVATE(DepthMapViewOptions)

  QTE_DISABLE_COPY(DepthMapViewOptions)
};

#endif