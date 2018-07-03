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

#include "wrench_display.h"

namespace my_rviz_plugin
{

WrenchStampedDisplay::WrenchStampedDisplay()
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

void WrenchStampedDisplay::onInitialize()
{
    MFDClass::onInitialize();
    updateHistoryLength( );
}

WrenchStampedDisplay::~WrenchStampedDisplay()
{
}

// Override rviz::Display's reset() function to add a call to clear().
void WrenchStampedDisplay::reset()
{
    MFDClass::reset();
    visuals_.clear();
}

void WrenchStampedDisplay::updateColorAndAlpha()
{
    float alpha = alpha_property_->getFloat();
    float force_scale = force_scale_property_->getFloat();
    float torque_scale = torque_scale_property_->getFloat();
    float width = width_property_->getFloat();
    Ogre::ColourValue force_color = force_color_property_->getOgreColor();
    Ogre::ColourValue torque_color = torque_color_property_->getOgreColor();

    for( size_t i = 0; i < visuals_.size(); i++ )
    {
        visuals_[i]->setForceColor( force_color.r, force_color.g, force_color.b, alpha );
        visuals_[i]->setTorqueColor( torque_color.r, torque_color.g, torque_color.b, alpha );
        visuals_[i]->setForceScale( force_scale );
        visuals_[i]->setTorqueScale( torque_scale );
        visuals_[i]->setWidth( width );
    }
}

// Set the number of past visuals to show.
void WrenchStampedDisplay::updateHistoryLength()
{
  std::cerr<<">>UPDATEHISTORYLENGTH"<<std::endl;
  //下の行を、visuals_のsizeが1以上のときに呼ぶと、警告なくrvizがcrashする。
  //rviz_default_pluginでは発生しない。
  visuals_.rset_capacity(history_length_property_->getInt());
  std::cerr<<"<<UPDATEHISTORYLENGTH"<<std::endl;
}

bool validateFloats( const geometry_msgs::WrenchStamped& msg )
{
    return rviz::validateFloats(msg.wrench.force) && rviz::validateFloats(msg.wrench.torque) ;
}

// This is our callback to handle an incoming message.
void WrenchStampedDisplay::processMessage( const geometry_msgs::WrenchStamped::ConstPtr& msg )
{
  std::cerr<<">>PROCESSMESSAGE"<<std::endl;
    if( !validateFloats( *msg ))
    {
        setStatus( rviz::StatusProperty::Error, "Topic", "Message contained invalid floating point values (nans or infs)" );
        return;
    }

    // Here we call the rviz::FrameManager to get the transform from the
    // fixed frame to the frame in the header of this Imu message.  If
    // it fails, we can't do anything else so we return.
    Ogre::Quaternion orientation;
    Ogre::Vector3 position;
    if( !context_->getFrameManager()->getTransform( msg->header.frame_id,
                                                    msg->header.stamp,
                                                    position, orientation ))
    {
        ROS_DEBUG( "Error transforming from frame '%s' to frame '%s'",
                   msg->header.frame_id.c_str(), qPrintable( fixed_frame_ ));
        return;
    }

    if ( position.isNaN() )
    {
        ROS_ERROR_THROTTLE(1.0, "Wrench position contains NaNs. Skipping render as long as the position is invalid");
        return;
    }

    // We are keeping a circular buffer of visual pointers.  This gets
    // the next one, or creates and stores it if the buffer is not full
    boost::shared_ptr<rviz::WrenchVisual> visual;
    if( visuals_.full() )
    {
        visual = visuals_.front();
    }
    else
    {
      visual.reset(new rviz::WrenchVisual( context_->getSceneManager(), scene_node_ ));
    }

    // Now set or update the contents of the chosen visual.
    visual->setWrench( msg->wrench );
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

    // And send it to the end of the circular buffer
    visuals_.push_back(visual);

    std::cerr<<"<<PROCESSMESSAGE"<<std::endl;
}

} // end namespace my_rviz_plugin

// Tell pluginlib about this class.  It is important to do this in
// global scope, outside our package's namespace.
#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS( my_rviz_plugin::WrenchStampedDisplay, rviz::Display )

