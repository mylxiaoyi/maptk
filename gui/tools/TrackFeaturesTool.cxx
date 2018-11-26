/*ckwg +29
 * Copyright 2017 by Kitware, Inc.
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

#include "TrackFeaturesTool.h"
#include "GuiCommon.h"

#include <maptk/colorize.h>
#include <maptk/version.h>

#include <vital/algo/image_io.h>
#include <vital/algo/convert_image.h>
#include <vital/algo/track_features.h>
#include <vital/algo/video_input.h>

#include <vital/types/metadata.h>
#include <vital/types/metadata_traits.h>

#include <QMessageBox>

using kwiver::vital::algo::image_io;
using kwiver::vital::algo::convert_image;
using kwiver::vital::algo::convert_image_sptr;
using kwiver::vital::algo::track_features;
using kwiver::vital::algo::track_features_sptr;
using kwiver::vital::algo::video_input;
using kwiver::vital::algo::video_input_sptr;

namespace
{
static char const* const BLOCK_CI = "image_converter";
static char const* const BLOCK_TF = "feature_tracker";
static char const* const BLOCK_VR = "video_reader";
}

//-----------------------------------------------------------------------------
class TrackFeaturesToolPrivate
{
public:
  convert_image_sptr image_converter;
  track_features_sptr feature_tracker;
  video_input_sptr video_reader;
};

QTE_IMPLEMENT_D_FUNC(TrackFeaturesTool)

//-----------------------------------------------------------------------------
TrackFeaturesTool::TrackFeaturesTool(QObject* parent)
  : AbstractTool(parent), d_ptr(new TrackFeaturesToolPrivate)
{
  this->setText("&Track Features Dense");
  this->setToolTip(
    "<nobr>Detect feature points in the images, compute feature descriptors, "
    "</nobr>and track the features across images.  Also run loop closure if "
    "configured to do so.  This is slower than the primary tracking algorithm.");
}

//-----------------------------------------------------------------------------
TrackFeaturesTool::~TrackFeaturesTool()
{
}

//-----------------------------------------------------------------------------
AbstractTool::Outputs TrackFeaturesTool::outputs() const
{
  return Tracks | ActiveFrame;
}

//-----------------------------------------------------------------------------
bool TrackFeaturesTool::execute(QWidget* window)
{
  QTE_D();

  if (!this->hasVideoSource())
  {
    QMessageBox::information(
      window, "Insufficient data", "This operation requires a video source.");
    return false;
  }

  // Merge project config with default config file
  auto const config = readConfig("gui_track_features.conf");

  // Check configuration
  if (!config)
  {
    QMessageBox::critical(
      window, "Configuration error",
      "No configuration data was found. Please check your installation.");
    return false;
  }

  config->merge_config(this->data()->config);
  if (!convert_image::check_nested_algo_configuration(BLOCK_CI, config) ||
      !track_features::check_nested_algo_configuration(BLOCK_TF, config) ||
      !video_input::check_nested_algo_configuration(BLOCK_VR, config))

  {
    QMessageBox::critical(
      window, "Configuration error",
      "An error was found in the algorithm configuration.");
    return false;
  }

  // Create algorithm from configuration
  convert_image::set_nested_algo_configuration(BLOCK_CI, config, d->image_converter);
  track_features::set_nested_algo_configuration(BLOCK_TF, config, d->feature_tracker);
  video_input::set_nested_algo_configuration(BLOCK_VR, config, d->video_reader);

  return AbstractTool::execute(window);
}

//-----------------------------------------------------------------------------
void TrackFeaturesTool::run()
{
  QTE_D();

  unsigned int frame = this->activeFrame();
  auto tracks = this->tracks();
  kwiver::vital::timestamp currentTimestamp;

  d->video_reader->open(this->data()->videoPath);

  // Seek to just before active frame TODO: check status?
  if (frame > 1)
  {
    d->video_reader->seek_frame(currentTimestamp, frame-1);
  }

  while (d->video_reader->next_frame(currentTimestamp))
  {
    auto const image = d->video_reader->frame_image();
    auto const converted_image = d->image_converter->convert(image);

    auto const mdv = d->video_reader->frame_metadata();
    if( !mdv.empty() )
    {
      converted_image->set_metadata( mdv[0] );
    }

    frame = currentTimestamp.get_frame();
    tracks = d->feature_tracker->track(tracks, frame, converted_image);
    if (tracks)
    {
      tracks = kwiver::maptk::extract_feature_colors(tracks, *image, frame);
    }

    // make a copy of the tool data
    auto data = std::make_shared<ToolData>();
    data->copyTracks(tracks);
    data->activeFrame = frame;

    emit updated(data);
    if( this->isCanceled() )
    {
      d->video_reader->close();
      break;
    }
  }
  d->video_reader->close();
  this->updateTracks(tracks);
  this->setActiveFrame(frame);
}
