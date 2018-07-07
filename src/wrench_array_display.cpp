//from https://github.com/ros-visualization/rviz/blob/kinetic-devel/src/rviz/default_plugin/wrench_display.cpp

/*
Upstream Authors (2005-2009):

    Ulisse Perusin <uli.peru@gmail.com>
    Steven Garrity <sgarrity@silverorange.com>
    Lapo Calamandrei <calamandrei@gmail.com>
    Ryan Collier <rcollier@novell.com>
    Rodney Dawes <dobey@novell.com>
    Andreas Nilsson <nisses.mail@home.se>
    Tuomas Kuosmanen <tigert@tigert.com>
    Garrett LeSage <garrett@novell.com>
    Jakub Steiner <jimmac@novell.com>

Other icons and graphics contained in this package are released into the Public Domain as well.

Authors (2012-2017):

    David Gossow
    Chad Rockey
    Kei Okada
    Julius Kammerl
    Acorn Pooley

Copyright notice for all icons and graphics in this package:

Public Domain Dedication

Copyright-Only Dedication (based on United States law) or Public Domain
Certification

The person or persons who have associated work with this document (the
"Dedicator" or "Certifier") hereby either (a) certifies that, to the best
of his knowledge, the work of authorship identified is in the public
domain of the country from which the work is published, or (b)
hereby dedicates whatever copyright the dedicators holds in the work
of authorship identified below (the "Work") to the public domain. A
certifier, moreover, dedicates any copyright interest he may have in
the associated work, and for these purposes, is described as a
"dedicator" below.

A certifier has taken reasonable steps to verify the copyright
status of this work. Certifier recognizes that his good faith efforts
may not shield him from liability if in fact the work certified is not
in the public domain.

Dedicator makes this dedication for the benefit of the public at
large and to the detriment of the Dedicator's heirs and successors.
Dedicator intends this dedication to be an overt act of relinquishment
in perpetuity of all present and future rights under copyright law,
whether vested or contingent, in the Work. Dedicator understands that
such relinquishment of all rights includes the relinquishment of all
rights to enforce (by lawsuit or otherwise) those copyrights in the
Work.

Dedicator recognizes that, once placed in the public domain, the Work
may be freely reproduced, distributed, transmitted, used, modified,
built upon, or otherwise exploited by anyone for any purpose, commercial
or non-commercial, and in any way, including by methods that have not
yet been invented or conceived.

 */

#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

#include <rviz/visualization_manager.h>
#include <rviz/frame_manager.h>
#include <rviz/properties/color_property.h>
#include <rviz/properties/float_property.h>
#include <rviz/properties/int_property.h>
#include <rviz/properties/parse_color.h>
#include <rviz/validate_floats.h>

#include <boost/foreach.hpp>

#include <rviz/default_plugin/wrench_visual.h>

#include "wrench_array_display.h"

namespace my_rviz_plugin
{

WrenchStampedArrayDisplay::WrenchStampedArrayDisplay()
{
    force_color_property_ =
            new rviz::ColorProperty( "Force Color", QColor( 204, 51, 51 ),
                                     "Color to draw the force arrows.",
                                     this, SLOT( updateColorAndAlpha() ));

    torque_color_property_ =
            new rviz::ColorProperty( "Torque Color", QColor( 204, 204, 51),
                                     "Color to draw the torque arrows.",
                                     this, SLOT( updateColorAndAlpha() ));

    alpha_property_ =
            new rviz::FloatProperty( "Alpha", 1.0,
                                     "0 is fully transparent, 1.0 is fully opaque.",
                                     this, SLOT( updateColorAndAlpha() ));

    force_scale_property_ =
            new rviz::FloatProperty( "Force Arrow Scale", 2.0,
                                     "force arrow scale",
                                     this, SLOT( updateColorAndAlpha() ));

    torque_scale_property_ =
            new rviz::FloatProperty( "Torque Arrow Scale", 2.0,
                                     "torque arrow scale",
                                     this, SLOT( updateColorAndAlpha() ));

    width_property_ =
            new rviz::FloatProperty( "Arrow Width", 0.5,
                                     "arrow width",
                                     this, SLOT( updateColorAndAlpha() ));


    history_length_property_ =
            new rviz::IntProperty( "History Length", 1,
                                   "Number of prior measurements to display.",
                                   this, SLOT( updateHistoryLength() ));

    history_length_property_->setMin( 1 );
    history_length_property_->setMax( 100000 );
}

void WrenchStampedArrayDisplay::onInitialize()
{
    MFDClass::onInitialize();
    updateHistoryLength( );
}

WrenchStampedArrayDisplay::~WrenchStampedArrayDisplay()
{
}

// Override rviz::Display's reset() function to add a call to clear().
void WrenchStampedArrayDisplay::reset()
{
    MFDClass::reset();
    visuals_.clear();
}

void WrenchStampedArrayDisplay::updateColorAndAlpha()
{
    float alpha = alpha_property_->getFloat();
    float force_scale = force_scale_property_->getFloat();
    float torque_scale = torque_scale_property_->getFloat();
    float width = width_property_->getFloat();
    Ogre::ColourValue force_color = force_color_property_->getOgreColor();
    Ogre::ColourValue torque_color = torque_color_property_->getOgreColor();

    for( size_t i = 0; i < visuals_.size(); i++ )
    {
      for(size_t j = 0; j< visuals_[i]->size(); j++){
        (*visuals_[i])[j]->setForceColor( force_color.r, force_color.g, force_color.b, alpha );
	(*visuals_[i])[j]->setTorqueColor( torque_color.r, torque_color.g, torque_color.b, alpha );
	(*visuals_[i])[j]->setForceScale( force_scale );
        (*visuals_[i])[j]->setTorqueScale( torque_scale );
	(*visuals_[i])[j]->setWidth( width );
      }
    }
}

// Set the number of past visuals to show.
void WrenchStampedArrayDisplay::updateHistoryLength()
{
  //visuals_.rset_capacity(history_length_property_->getInt());
  if (visuals_.size()>history_length_property_->getInt()){
    for(int i=0;i<history_length_property_->getInt()-visuals_.size();i++){
      visuals_.pop_front();
    }
  }
}

// bool validateFloats( const geometry_msgs::WrenchStamped& msg )
// {
//     return rviz::validateFloats(msg.wrench.force) && rviz::validateFloats(msg.wrench.torque) ;
// }

// This is our callback to handle an incoming message.
void WrenchStampedArrayDisplay::processMessage( const my_rviz_plugin::WrenchStampedArray::ConstPtr& msg )
{
  boost::shared_ptr<std::vector<boost::shared_ptr<rviz::WrenchVisual> > > visuals;
  if( visuals_.size()>=history_length_property_->getInt() )
    {
      visuals = visuals_.front();
      visuals_.pop_front();
      visuals->clear();
    }
  else
    {
      visuals.reset(new std::vector<boost::shared_ptr<rviz::WrenchVisual> >{});
    }
  for (int i=0;i<msg->wrenchstampeds.size();i++){
    if( !validateFloats( msg->wrenchstampeds[i] ))
      {
        setStatus( rviz::StatusProperty::Error, "Topic", "Message contained invalid floating point values (nans or infs)" );
        continue;
      }
    
    // Here we call the rviz::FrameManager to get the transform from the
    // fixed frame to the frame in the header of this Imu message.  If
    // it fails, we can't do anything else so we return.
    Ogre::Quaternion orientation;
    Ogre::Vector3 position;
    if( !context_->getFrameManager()->getTransform( msg->wrenchstampeds[i].header.frame_id,
                                                    msg->wrenchstampeds[i].header.stamp,
                                                    position, orientation ))
      {
	//ROS_ERROR( "Error transforming from frame '%s' to frame '%s'",
	ROS_DEBUG( "Error transforming from frame '%s' to frame '%s'",
                   msg->wrenchstampeds[i].header.frame_id.c_str(), qPrintable( fixed_frame_ ));
        continue;
      }
    
    if ( position.isNaN() )
      {
        ROS_ERROR_THROTTLE(1.0, "Wrench position contains NaNs. Skipping render as long as the position is invalid");
        continue;
    }

    boost::shared_ptr<rviz::WrenchVisual> visual{new rviz::WrenchVisual{context_->getSceneManager(), scene_node_ }};
    // Now set or update the contents of the chosen visual.
    visual->setWrench( msg->wrenchstampeds[i].wrench );
    //std::this_thread::sleep_for(std::chrono::seconds(3));
    visual->setFramePosition( position );
    visual->setFrameOrientation( orientation );
    float alpha = alpha_property_->getFloat();
    float force_scale = force_scale_property_->getFloat();
    float torque_scale = torque_scale_property_->getFloat();
    float width = width_property_->getFloat();
    Ogre::ColourValue force_color = force_color_property_->getOgreColor();
    Ogre::ColourValue torque_color = torque_color_property_->getOgreColor();
    visual->setForceColor( force_color.r, force_color.g, force_color.b, alpha );
    visual->setTorqueColor( torque_color.r, torque_color.g, torque_color.b, alpha );
    visual->setForceScale( force_scale );
    visual->setTorqueScale( torque_scale );
    visual->setWidth( width );
    //visuals->push_back(std::move(visual));
    visuals->push_back(visual);
    //std::cerr<<"$$$$$$$$$$$$$$$"<<std::endl;
  }
  // And send it to the end of the circular buffer
  visuals_.push_back(visuals);
}

} // end namespace my_rviz_plugin

// Tell pluginlib about this class.  It is important to do this in
// global scope, outside our package's namespace.
#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS( my_rviz_plugin::WrenchStampedArrayDisplay, rviz::Display )

