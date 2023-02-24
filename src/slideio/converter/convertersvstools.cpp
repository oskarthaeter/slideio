// This file is part of slideio project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://slideio.com/license.html.

#include "convertersvstools.hpp"

#include "convertertools.hpp"
#include "slideio/base/exceptions.hpp"

void slideio::ConverterSVSTools::checkSVSRequirements(const CVScenePtr& scene)
{
    const DataType dt = scene->getChannelDataType(0);
    if(dt != DataType::DT_Byte) {
        RAISE_RUNTIME_ERROR << "Converter: Only 8bit images are supported now!";
    }
    const int numChannels = scene->getNumChannels();
    if (numChannels != 1 && numChannels !=3) {
        RAISE_RUNTIME_ERROR << "Converter: Only 1 and 3 channels are supported now!";
    }
    for (int channel = 1; channel < numChannels; ++channel) {
        if (dt != scene->getChannelDataType(channel)) {
            RAISE_RUNTIME_ERROR << "Converter: Cannot convert scene with different channel types to SVS!";
        }
    }
}

std::string slideio::ConverterSVSTools::createDescription(const CVScenePtr& scene)
{
    auto rect = scene->getRect();
    std::stringstream buff;
    buff << "SlideIO Library 2.0" << std::endl;
    buff << rect.width << "x" << rect.height << std::endl;
    double magn = scene->getMagnification();
    if (magn > 0) {
        buff << "AppMag = " << magn;
    }
    return buff.str();
}

void slideio::ConverterSVSTools::createZoomLevel(TIFFKeeperPtr& file, int zoomLevel, const CVScenePtr& scene, const cv::Size& tileSize)
{
    cv::Rect sceneRect = scene->getRect();
    sceneRect.x = sceneRect.y = 0;
    cv::Rect levelRect = ConverterTools::computeZoomLevelRect(sceneRect, tileSize, zoomLevel);


    slideio::TiffDirectory dir;
    dir.channels = scene->getNumChannels();
    dir.dataType = scene->getChannelDataType(0);
    dir.slideioCompression = slideio::Compression::Jpeg;
    dir.width = levelRect.width;
    dir.height = levelRect.height;
    dir.tileWidth = tileSize.width;
    dir.tileHeight = tileSize.height;
    if (zoomLevel == 0) {
        dir.description = createDescription(scene);
    }
    else {
        dir.description = "";
    }
    dir.res = scene->getResolution();
    file->setTags(dir, zoomLevel > 0);

    cv::Size sceneTileSize = slideio::ConverterTools::scaleSize(tileSize, zoomLevel, false);

    cv::Mat tile;
    for (int y = 0; y < sceneRect.height; y += sceneTileSize.height) {
        for (int x = 0; x < sceneRect.width; x += sceneTileSize.width) {
            cv::Rect blockRect(x, y, sceneTileSize.width, sceneTileSize.height);
            ConverterTools::readTile(scene, zoomLevel, blockRect, tile);
        }
    }
}
