﻿#include "pyscene.hpp"
#include <pybind11/numpy.h>
#include <boost/format.hpp>
#include "pyslide.hpp"

namespace py = pybind11;

PyScene::PyScene(std::shared_ptr<slideio::Scene> scene, std::shared_ptr<slideio::Slide> slide):
    m_scene(std::move(scene)), m_slide(slide)
{
}

std::string PyScene::getFilePath() const
{
    return m_scene->getFilePath();
}

std::string PyScene::getName() const
{
    return m_scene->getName();
}

std::tuple<int, int, int, int> PyScene::getRect() const
{
    return m_scene->getRect();
}

int PyScene::getNumChannels() const
{
    return m_scene->getNumChannels();
}

int PyScene::getNumZSlices() const
{
    return m_scene->getNumZSlices();
}

int PyScene::getNumTFrames() const
{
    return m_scene->getNumTFrames();
}

std::string PyScene::getChannelName(int channel) const
{
    return m_scene->getChannelName(channel);
}

std::tuple<double, double> PyScene::getResolution() const
{
    return m_scene->getResolution();
}

double PyScene::getZSliceResolution() const
{
    return m_scene->getZSliceResolution();
}

double PyScene::getTFrameResolution() const
{
    return m_scene->getTFrameResolution();
}

double PyScene::getMagnification() const
{
    return m_scene->getMagnification();
}

slideio::Compression PyScene::getCompression() const
{
    return m_scene->getCompression();
}

py::dtype PyScene::getChannelDataType(int channel) const
{
    auto dt = m_scene->getChannelDataType(channel);
    switch(dt)
    {
        case slideio::DataType::DT_Byte:
            return py::detail::npy_format_descriptor<uint8_t>::dtype();
        case slideio::DataType::DT_Int8:
            return py::detail::npy_format_descriptor<int8_t>::dtype();
        case slideio::DataType::DT_Int16:
            return py::detail::npy_format_descriptor<int16_t>::dtype();
        case slideio::DataType::DT_Int32:
            return py::detail::npy_format_descriptor<int32_t>::dtype();
        case slideio::DataType::DT_Float32:
            return py::detail::npy_format_descriptor<float>::dtype();
        case slideio::DataType::DT_Float64:
            return py::detail::npy_format_descriptor<double>::dtype();
        case slideio::DataType::DT_UInt16:
            return py::detail::npy_format_descriptor<uint16_t>::dtype();
        default:
            throw std::runtime_error("Cannot convert data to numpy: Unsupported data type");
    }
}

pybind11::array PyScene::readBlock(std::tuple<int, int, int, int> rect,
    std::tuple<int, int> size, std::vector<int> channelIndices,
    std::tuple<int,int> sliceRange, std::tuple<int,int> tframeRange) const
{
    const int imageChannels = getNumChannels();
    const std::tuple<int,int,int,int> imageRect = m_scene->getRect();
    const int imageWidth = std::get<2>(imageRect);
    const int imageHeight = std::get<3>(imageRect);

    const int startSlice = std::max(0,std::get<0>(sliceRange));
    const int stopSlice = std::get<1>(sliceRange);
    const int startFrame = std::max(0,std::get<0>(tframeRange));
    const int stopFrame = std::get<1>(tframeRange);
    const int numSlices = std::max(1,stopSlice - startSlice);
    const int numFrames = std::max(1,stopFrame - startFrame);

    const int refChannel = channelIndices.empty()?0:channelIndices[0];
    const int numChannels = channelIndices.empty()?imageChannels:static_cast<int>(channelIndices.size());

    const py::dtype dtype = getChannelDataType(refChannel);

    PyRect blockRect = adjustSourceRect(rect);
    PySize blockSize = adjustTargetSize(blockRect, size);

    // source block parameters

    const int memSize = m_scene->getBlockSize(blockSize, refChannel, numChannels, numSlices, numFrames);

    py::array::ShapeContainer shape;
    const int planes = numChannels*numSlices*numFrames;

    if(planes==1)
    {
        shape = {blockSize.height(), blockSize.width()};
    }
    else
    {
        shape = {blockSize.height(), blockSize.width(), planes};
    }

    py::array numpy_array(dtype, shape);

    if(startSlice==0 && stopSlice<=1 && startFrame==0 && stopFrame<=1)
    {
        m_scene->readResampledBlockChannels(blockRect, blockSize, channelIndices, numpy_array.mutable_data(), memSize);
    }
    else
    {
        if(stopSlice<=startSlice)
        {
            throw std::runtime_error(
                (boost::format("Invalid slice range (%1%,%2%)") %startSlice %stopSlice).str()
            );
        }
        if(stopFrame<=startFrame)
        {
            throw std::runtime_error(
                (boost::format("Invalid time frame range (%1%,%2%)") %startFrame %stopFrame).str()
            );
        }
        m_scene->readResampled4DBlockChannels(blockRect, blockSize, channelIndices, sliceRange, tframeRange, numpy_array.mutable_data(), memSize);
    }

    return numpy_array;
}

PyRect PyScene::adjustSourceRect(const PyRect& rect) const
{
    PyRect srcRect(rect);
    const PyRect imageRect = m_scene->getRect();
    if(srcRect.width()==0)
    {
        srcRect.width() = imageRect.width() - srcRect.x();
    }
    if(srcRect.height()==0)
    {
        srcRect.height() = imageRect.height() - srcRect.y();
    }
    return srcRect;
}

PySize PyScene::adjustTargetSize(const PyRect& rect, const PySize& size) const
{
    PySize trgSize(size);
    if(trgSize.width()<=0 && trgSize.height()<=0)
    {
        // both width and height are invalid
        // set them to the size of the source 
        trgSize.width() = rect.width();
        trgSize.height() = rect.height();
    }
    else if(trgSize.width()<=0)
    {
        // only target height is set.
        // compute target width from the height
        double coef = double(trgSize.height())/double(rect.height());
        double width = coef*double(rect.width());
        trgSize.width() = static_cast<int>(std::lround(width));
    }
    else if(trgSize.height()<=0)
    {
        // only target width is set.
        // compute target width from the height
        double coef = double(trgSize.width())/double(rect.width());
        double height = coef*double(rect.height());
        trgSize.height() = static_cast<int>(std::lround(height));
    }
    return trgSize;
}

