// This file is part of slideio project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://slideio.com/license.html.
#include <gtest/gtest.h>
#include "slideio/core/imagedrivermanager.hpp"
#include "testtools.hpp"
#include "slideio/drivers/zvi/ZVIUtils.hpp"
#include <pole/polepp.hpp>



TEST(ZVIUtils, read_stream_int)
{
    std::string file_path = TestTools::getTestImagePath("zvi","Zeiss-1-Merged.zvi");
    ole::compound_document doc(file_path);
    ASSERT_TRUE(doc.good());
    auto begin = doc.begin();
    auto end = doc.end();
    auto storage = doc.find_storage("/Image");
    ASSERT_TRUE(storage != doc.end());
    auto contents = storage->find_stream("/Image/Contents");
    ASSERT_TRUE(contents != storage->end());

    ole::basic_stream stream = contents->stream();
    ZVIUtils::skipItems(stream, 4);

    int32_t width = ZVIUtils::readIntItem(stream);
    EXPECT_EQ(width, 1480);

    int32_t height = ZVIUtils::readIntItem(stream);
    EXPECT_EQ(height, 1132);

    int32_t depth = ZVIUtils::readIntItem(stream);
    EXPECT_EQ(depth, 0);

    int32_t pixelFormat = ZVIUtils::readIntItem(stream);
    EXPECT_EQ(pixelFormat, 4);

    int32_t rawCount = ZVIUtils::readIntItem(stream);
    EXPECT_EQ(rawCount, 3);
}

TEST(ZVIUtils, read_stream_double)
{
    std::string file_path = TestTools::getTestImagePath("zvi","Zeiss-1-Merged.zvi");
    ole::compound_document doc(file_path);
    ASSERT_TRUE(doc.good());
    auto begin = doc.begin();
    auto end = doc.end();
    auto scaling_storage = doc.find_storage("/Image/Scaling");
    ASSERT_TRUE(scaling_storage != doc.end());
    auto contents_stream = scaling_storage->find_stream("/Image/Scaling/Contents");
    ASSERT_TRUE(contents_stream != scaling_storage->end());
    ZVIUtils::skipItems(contents_stream->stream(), 3);
    double value = ZVIUtils::readDoubleItem(contents_stream->stream());
    ASSERT_DOUBLE_EQ(value, 0.0645);
    int scalingUnits = ZVIUtils::readIntItem(contents_stream->stream());
    ASSERT_EQ(scalingUnits, 76);
}

TEST(ZVIUtils, read_stream_string)
{
    std::string file_path = TestTools::getTestImagePath("zvi","Zeiss-1-Merged.zvi");
    ole::compound_document doc(file_path);
    ASSERT_TRUE(doc.good());
    auto begin = doc.begin();
    auto end = doc.end();
    auto scaling_storage = doc.find_storage("/Image/Scaling");
    ASSERT_TRUE(scaling_storage != doc.end());
    auto contents_stream = scaling_storage->find_stream("/Image/Scaling/Contents");
    ASSERT_TRUE(contents_stream != scaling_storage->end());
    ZVIUtils::skipItems(contents_stream->stream(), 1);
    std::string key = ZVIUtils::readStringItem(contents_stream->stream());
    ASSERT_EQ(key, std::string("Scaling124"));
}
