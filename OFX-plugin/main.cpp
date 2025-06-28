/*
MIT License

Copyright (c) 2025 Nikita Petrovich

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ofxCore.h"
#include "ofxsImageEffect.h"
#include "ofxsProcessing.h"
#include <ofxsCore.h>
#include <ofxsParam.h>

#include <cstddef>
#include <memory>

#define kPluginName "Nikita Blend"
#define kPluginGrouping "NP FX"
#define kPluginDescription "Original composite mode to blend 2 images"
#define kPluginIdentifier "ofx.NP.BlendMode"
#define kPluginVersionMajor 1
#define kPluginVersionMinor 0

#define kSupportsTiles true

//////////////////////////////////////////////////////////////////////////////
// PROCESSOR
//////////////////////////////////////////////////////////////////////////////

class Processor : public OFX::ImageProcessor {
public:
  explicit Processor(OFX::ImageEffect &p_Instance);

  virtual void multiThreadProcessImages(OfxRectI p_ProcWindow);
  virtual void preProcess();

  void setSrcImg(OFX::Image *p_srcFrame);
  void setTopLayer(OFX::Image *p_topLayer);
  void setParams(bool p_useAvgColor, float p_blend);
  float calcLuminance(const float *srcPix);
  void calcAvgColor();

private:
  OFX::Image *m_srcFrame;
  OFX::Image *m_topLayer;

  std::vector<float> m_avgColor{0.f, 0.f, 0.f};
  bool m_useAvgColor;
  float m_blend;
};

Processor::Processor(OFX::ImageEffect &p_Instance)
    : OFX::ImageProcessor(p_Instance) {}

void Processor::preProcess() {
  if (m_useAvgColor)
    calcAvgColor();
}

void Processor::multiThreadProcessImages(OfxRectI p_ProcWindow) {
  for (int y = p_ProcWindow.y1; y < p_ProcWindow.y2; ++y) {
    if (_effect.abort())
      break;

    float *dstPix =
        static_cast<float *>(_dstImg->getPixelAddress(p_ProcWindow.x1, y));

    for (int x = p_ProcWindow.x1; x < p_ProcWindow.x2; ++x) {

      float *srcPix = static_cast<float *>(
          m_srcFrame ? m_srcFrame->getPixelAddress(x, y) : nullptr);

      float *topLayerPix = static_cast<float *>(
          m_topLayer ? m_topLayer->getPixelAddress(x, y) : nullptr);

      if (srcPix && topLayerPix) {
        float srcPixLum = calcLuminance(srcPix);

        for (int c = 0; c < 3; ++c) {
          dstPix[c] = (srcPix[c] + srcPixLum -
                       (m_useAvgColor ? m_avgColor[c] : topLayerPix[c])) *
                          m_blend +
                      srcPix[c] * (1.f - m_blend);
        }
        dstPix[3] = srcPix[3];
      } else {
        // no src pixel here, be black and transparent
        for (int c = 0; c < 4; ++c) {
          dstPix[c] = 0.f;
        }
      }
      dstPix += 4;
    }
  }
}

void Processor::setSrcImg(OFX::Image *p_srcFrame) { m_srcFrame = p_srcFrame; }
void Processor::setTopLayer(OFX::Image *p_topLayer) { m_topLayer = p_topLayer; }

void Processor::setParams(bool p_useAvgColor, float p_blend) {
  m_useAvgColor = p_useAvgColor;
  m_blend = p_blend;
}

float Processor::calcLuminance(const float *srcPix) {
  return 0.2126f * srcPix[0] + 0.7152f * srcPix[1] + 0.0722f * srcPix[2];
}

void Processor::calcAvgColor() {
  int countPixels{0};

  for (int y = _renderWindow.y1; y < _renderWindow.y2; ++y) {
    if (_effect.abort())
      break;

    for (int x = _renderWindow.x1; x < _renderWindow.x2; ++x) {
      float *topLayerPix = static_cast<float *>(
          m_topLayer ? m_topLayer->getPixelAddress(x, y) : nullptr);

      if (topLayerPix) {
        for (int c = 0; c < 3; ++c) {
          m_avgColor[c] += topLayerPix[c];
        }
        ++countPixels;
      }
    }
  }

  for (int c = 0; c < 3; ++c) {
    m_avgColor[c] /= countPixels;
  }
}

//////////////////////////////////////////////////////////////////////////////
// THE PLUGIN
//////////////////////////////////////////////////////////////////////////////

class NikitaBlendPlugin : public OFX::ImageEffect {
public:
  explicit NikitaBlendPlugin(OfxImageEffectHandle p_Handle);

  virtual void render(const OFX::RenderArguments &p_Args);
  virtual bool isIdentity(const OFX::IsIdentityArguments &args,
                          OFX::Clip *&identityClip, double &identityTime);

private:
  // Does not own the following pointers
  OFX::Clip *m_dstClip;
  OFX::Clip *m_srcClip;
  OFX::Clip *m_topClip;

  OFX::BooleanParam *m_swapLayers;
  OFX::BooleanParam *m_useAvgColor;
  OFX::DoubleParam *m_blend;
};

NikitaBlendPlugin::NikitaBlendPlugin(OfxImageEffectHandle p_Handle)
    : ImageEffect(p_Handle) {
  m_dstClip = fetchClip(kOfxImageEffectOutputClipName);
  m_srcClip = fetchClip(kOfxImageEffectSimpleSourceClipName);
  getContext() == OFX::eContextGeneral ? m_topClip = fetchClip("TopLayer")
                                       : m_topClip = nullptr;

  m_swapLayers = fetchBooleanParam("swapLayers");
  m_useAvgColor = fetchBooleanParam("useAvgColor");
  m_blend = fetchDoubleParam("blend");
}

void NikitaBlendPlugin::render(const OFX::RenderArguments &p_Args) {
  if ((m_dstClip->getPixelDepth() == OFX::eBitDepthFloat) &&
      (m_dstClip->getPixelComponents() == OFX::ePixelComponentRGBA)) {

    Processor nikitaBlend(*this);

    const double currTime{p_Args.time};

    // Get the dst image
    std::unique_ptr<OFX::Image> dst{m_dstClip->fetchImage(currTime)};
    const OFX::BitDepthEnum dstBitDepth{dst->getPixelDepth()};
    const OFX::PixelComponentEnum dstComponents{dst->getPixelComponents()};

    // Get Top Layer Image
    std::unique_ptr<OFX::Image> topLayer{
        (getContext() == OFX::eContextGeneral && m_topClip->isConnected())
            ? m_topClip->fetchImage(currTime)
            : m_srcClip->fetchImage(currTime)};
    const OFX::BitDepthEnum topLayerBitDepth{topLayer->getPixelDepth()};
    const OFX::PixelComponentEnum topLayerComponents{
        topLayer->getPixelComponents()};

    if ((topLayerBitDepth != dstBitDepth) ||
        (topLayerComponents != dstComponents)) {
      OFX::throwSuiteStatusException(kOfxStatErrValue);
    }

    // Get Src Image
    std::unique_ptr<OFX::Image> src{m_srcClip->fetchImage(currTime)};
    const OFX::BitDepthEnum srcBitDepth{src->getPixelDepth()};
    const OFX::PixelComponentEnum srcComponents{src->getPixelComponents()};

    if ((srcBitDepth != dstBitDepth) || (srcComponents != dstComponents)) {
      OFX::throwSuiteStatusException(kOfxStatErrValue);
    }

    if (m_swapLayers->getValueAtTime(currTime)) {
      std::swap(src, topLayer);
    }

    nikitaBlend.setDstImg(dst.get());
    nikitaBlend.setSrcImg(src.get());
    nikitaBlend.setTopLayer(topLayer.get());

    nikitaBlend.setParams(m_useAvgColor->getValueAtTime(currTime),
                          m_blend->getValueAtTime(currTime));

    nikitaBlend.setRenderWindow(p_Args.renderWindow);

    // Call the base class process member, this will call the derived
    // templated process code
    nikitaBlend.process();

  } else {
    OFX::throwSuiteStatusException(kOfxStatErrUnsupported);
  }
}

bool NikitaBlendPlugin::isIdentity(const OFX::IsIdentityArguments &args,
                                   OFX::Clip *&identityClip,
                                   double &identityTime) {
  if (m_blend->getValueAtTime(args.time) == 0.) {
    identityClip = m_srcClip;
    identityTime = args.time;
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
// PLUGIN FACTORY
//////////////////////////////////////////////////////////////////////////////

class NikitaBlendPluginFactory
    : public OFX::PluginFactoryHelper<NikitaBlendPluginFactory> {
public:
  NikitaBlendPluginFactory();
  virtual void load() {}
  virtual void unload() {}
  virtual void describe(OFX::ImageEffectDescriptor &p_Desc);
  virtual void describeInContext(OFX::ImageEffectDescriptor &p_Desc,
                                 OFX::ContextEnum p_Context);
  virtual OFX::ImageEffect *createInstance(OfxImageEffectHandle p_Handle,
                                           OFX::ContextEnum p_Context);
};

NikitaBlendPluginFactory::NikitaBlendPluginFactory()
    : OFX::PluginFactoryHelper<NikitaBlendPluginFactory>(
          kPluginIdentifier, kPluginVersionMajor, kPluginVersionMinor) {}

void NikitaBlendPluginFactory::describe(OFX::ImageEffectDescriptor &p_Desc) {
  p_Desc.setLabels(kPluginName, kPluginName, kPluginName);
  p_Desc.setPluginGrouping(kPluginGrouping);
  p_Desc.setPluginDescription(kPluginDescription);

  p_Desc.addSupportedContext(OFX::eContextFilter);
  p_Desc.addSupportedContext(OFX::eContextGeneral);

  p_Desc.addSupportedBitDepth(OFX::eBitDepthFloat);

  p_Desc.setHostFrameThreading(true);
  p_Desc.setSupportsMultiResolution(false);
  p_Desc.setSupportsTiles(kSupportsTiles);
  p_Desc.setRenderTwiceAlways(false);
}

void NikitaBlendPluginFactory::describeInContext(
    OFX::ImageEffectDescriptor &p_Desc, OFX::ContextEnum p_Context) {
  // Create mandated source clip
  OFX::ClipDescriptor *srcClip{
      p_Desc.defineClip(kOfxImageEffectSimpleSourceClipName)};
  srcClip->addSupportedComponent(OFX::ePixelComponentRGBA);
  srcClip->setSupportsTiles(kSupportsTiles);

  // Create Top Layer clip
  if (p_Context == OFX::eContextGeneral) {
    OFX::ClipDescriptor *topLayer{p_Desc.defineClip("TopLayer")};
    topLayer->addSupportedComponent(OFX::ePixelComponentRGBA);
    topLayer->setSupportsTiles(kSupportsTiles);
    topLayer->setOptional(true);
  }

  // Create mandated output clip
  OFX::ClipDescriptor *dstClip{
      p_Desc.defineClip(kOfxImageEffectOutputClipName)};
  dstClip->addSupportedComponent(OFX::ePixelComponentRGBA);
  dstClip->setSupportsTiles(kSupportsTiles);

  // Page "Controls"
  {
    OFX::PageParamDescriptor *page{p_Desc.definePageParam("Contorls")};
    {
      OFX::BooleanParamDescriptor *param{
          p_Desc.defineBooleanParam("useAvgColor")};
      param->setLabel("Top Color Average");
      param->setHint("Use average color of the top layer");
      param->setDefault(false);
      page->addChild(*param);
    }
    {
      OFX::BooleanParamDescriptor *param{
          p_Desc.defineBooleanParam("swapLayers")};
      param->setLabel("Swap Layers");
      param->setHint("Swap Layers");
      param->setDefault(false);
      page->addChild(*param);
    }
    {
      OFX::DoubleParamDescriptor *param{p_Desc.defineDoubleParam("blend")};
      param->setLabel("Blend");
      param->setHint("Blend result with a bottom layer");
      param->setDefault(1.0);
      param->setRange(0., 1.);
      param->setIncrement(0.001);
      param->setDisplayRange(0., 1.);
      param->setDoubleType(OFX::eDoubleTypeScale);
      page->addChild(*param);
    }
  } // end of the Page "Controls"
}

OFX::ImageEffect *
NikitaBlendPluginFactory::createInstance(OfxImageEffectHandle p_Handle,
                                         OFX::ContextEnum /*p_Context*/) {
  return new NikitaBlendPlugin(p_Handle);
}

void OFX::Plugin::getPluginIDs(PluginFactoryArray &p_FactoryArray) {
  static NikitaBlendPluginFactory nikitaBlend;
  p_FactoryArray.push_back(&nikitaBlend);
}
