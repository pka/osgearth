/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
 * Copyright 2008-2010 Pelican Mapping
 * http://osgearth.org
 *
 * osgEarth is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#include "KML_Placemark"
#include "KML_Geometry"
#include <osgEarthFeatures/MarkerFactory>
#include <osgEarthUtil/Annotation>

using namespace osgEarth::Features;
using namespace osgEarth::Util::Annotation;

void 
KML_Placemark::build( const Config& conf, KMLContext& cx )
{
    Style style;
    if ( conf.hasValue("styleurl") )
    {
        const Style* ref_style = cx._sheet->getStyle( conf.value("styleurl"), false );
        if ( ref_style )
            style = *ref_style;
    }

    URI iconURI;
    MarkerSymbol* marker = style.get<MarkerSymbol>();
    if ( marker && marker->url().isSet() )
    {
        MarkerFactory mf;
        iconURI = mf.getRawURI( marker );
    }

    std::string text = 
        conf.hasValue("name") ? conf.value("name") :
        conf.hasValue("description") ? conf.value("description") :
        "Unnamed";

    // read in the geometry:
    osg::Vec3d position;
    KML_Geometry geometry;
    geometry.build(conf, cx, style);
    if ( geometry._geom.valid() && geometry._geom->size() > 0 )
    {
        Geometry* geom = geometry._geom.get();
        position = geom->getBounds().center();
    }

    // if we have a non-single-point geometry, render it.
    if ( geometry._geom.valid() && geometry._geom->size() != 1 )
    {
        const ExtrusionSymbol* ex = style.get<ExtrusionSymbol>();
        const AltitudeSymbol* alt = style.get<AltitudeSymbol>();

        bool draped =
            ex == 0L &&
            (alt && alt->clamping() == AltitudeSymbol::CLAMP_TO_TERRAIN);


        FeatureNode* fNode = new FeatureNode( cx._mapNode, new Feature(geometry._geom.get()), draped );
        fNode->setStyle( style );
        // drape if we're not extruding.
        fNode->setDraped( draped );
        cx._groupStack.top()->addChild( fNode );
    }

    PlacemarkNode* pmNode = new PlacemarkNode( cx._mapNode, position, iconURI, text, style );
    cx._groupStack.top()->addChild( pmNode );
}