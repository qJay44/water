#include "FFT.hpp"

#include "glm/common.hpp"
#include "glm/exponential.hpp"
#include "utils/utils.hpp"
#include <cassert>
#include <utility>

namespace water {

bool FFT::isCreated = false;

FFT::FFT() {
  if (std::exchange(isCreated, true))
    error("[FFT::FFT] FFT instance is already exist");

  ubo.spectrums.gen();
  ubo.spectrums.storage(nullptr, sizeof(spectrums), GL_DYNAMIC_STORAGE_BIT);
  rebuild();
}

void FFT::rebuild() {
  TextureDescriptor descR{.internalFormat = GL_R32F, .format = GL_RED, .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
  TextureDescriptor descRG{.internalFormat = GL_RG32F, .format = GL_RG, .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
  TextureDescriptor descRGBA{.internalFormat = GL_RGBA32F, .format = GL_RGBA, .wrapS = GL_REPEAT, .wrapT = GL_REPEAT};
  logSize = glm::log2((float)size);
  numWorkGroups = size / 8;

  texPrecomputedTwiddleFactorsAndInputIndices = Texture2D(ivec2(logSize, size), descRGBA);
  texNoise                                    = Texture2D{size, descRG};
  texInitialSpectrum                          = Texture2D{size, descRGBA};
  texPrecomputedData                          = Texture2D{size, descRGBA};
  texBuffer                                   = Texture2D{size, descRG};
  texDxDz                                     = Texture2D(size, descRG);
  texDyDxz                                    = Texture2D(size, descRG);
  texDyxDyz                                   = Texture2D(size, descRG);
  texDxxDzz                                   = Texture2D(size, descRG);
  texDisplacement                             = Texture2D{size, descRGBA};
  texDerivatives                              = Texture2D{size, descRGBA};
  texTurbulence                               = Texture2D{size, descR};

  generatePrecomputedTwiddleFactorsAndInputIndices();
  generateNoise();
  generateInitials();
}

float FFT::JonswapAlpha(float g, float fetch, float windSpeed) {
  return 0.076f * glm::pow(g * fetch / windSpeed / windSpeed, -0.22f);
}

float FFT::JonswapPeakFrequency(float g, float fetch, float windSpeed) {
  return 22.f * glm::pow(windSpeed * fetch / g / g, -0.33f);
}

void FFT::fillSettings(const SpectrumSettingsGUI& display, SpectrumSettings& settings) {
  settings.scale = display.scale;
  settings.angle = display.windDir;
  settings.spreadBlend = display.spreadBlend;
  settings.swell = glm::clamp(display.swell, 0.01f, 1.f);
  settings.alpha = JonswapAlpha(g, display.fetch, display.windSpeed);
  settings.peakOmega = JonswapPeakFrequency(g, display.fetch, display.windSpeed);
  settings.gamma = display.peakEnhancemnt;
  settings.shortWavesFade = display.shortWavesFade;
}

void FFT::generatePrecomputedTwiddleFactorsAndInputIndices() {
  shaderPrecomputedTwiddleFactorsAndInputIndices.use();
  shaderPrecomputedTwiddleFactorsAndInputIndices.setUniform1i("u_size", size);
  glBindImageTexture(0, texPrecomputedTwiddleFactorsAndInputIndices.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(logSize, numWorkGroups / 2, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FFT::generateNoise() {
  shaderNoise.use();
  shaderNoise.setUniform1f("u_seed1", seed1);
  shaderNoise.setUniform1f("u_seed2", seed2);
  glBindImageTexture(0, texNoise.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FFT::generateInitials() {
  fillSettings(local, spectrums[0]);
  fillSettings(swell, spectrums[1]);
  ubo.spectrums.updateSubData(spectrums, sizeof(spectrums));

  ubo.spectrums.bindBase(0);
  texNoise.bind(0);

  shaderInitialSpectrum.use();
  shaderInitialSpectrum.setUniform1f("u_g", g);
  shaderInitialSpectrum.setUniform1f("u_depth", depth);
  shaderInitialSpectrum.setUniform1f("u_lengthScale", lengthScale);
  glBindImageTexture(0, texNoise.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(1, texBuffer.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glBindImageTexture(2, texPrecomputedData.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  shaderConjugateSpectrum.use();
  glBindImageTexture(0, texBuffer.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(1, texInitialSpectrum.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FFT::generateWavesAtTime(float time) {
  shaderTimeSpectrum.use();
  shaderTimeSpectrum.setUniform1f("u_time", time);
  glBindImageTexture(0, texInitialSpectrum.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
  glBindImageTexture(1, texPrecomputedData.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
  glBindImageTexture(2, texDxDz  .getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glBindImageTexture(3, texDyDxz .getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glBindImageTexture(4, texDyxDyz.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glBindImageTexture(5, texDxxDzz.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FFT::generateIFFT(Texture2D& input, Texture2D& buffer) {
  GLuint ping = input.getId();
  GLuint pong = buffer.getId();

  shaderIFFT_horizontal.use();
  glBindImageTexture(0, texPrecomputedTwiddleFactorsAndInputIndices.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

  for (int i = 0; i < logSize; i++) {
    shaderIFFT_horizontal.setUniform1i("u_step", i);
    glBindImageTexture(1, ping, 0, GL_FALSE, 0, GL_READ_ONLY , GL_RG32F);
    glBindImageTexture(2, pong, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    glDispatchCompute(numWorkGroups, numWorkGroups, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    std::swap(ping, pong);
  }

  if (logSize & 1)
    std::swap(ping, pong);

  shaderIFFT_vertical.use();
  glBindImageTexture(0, texPrecomputedTwiddleFactorsAndInputIndices.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

  for (int i = 0; i < logSize; i++) {
    shaderIFFT_vertical.setUniform1i("u_step", i);
    glBindImageTexture(1, ping, 0, GL_FALSE, 0, GL_READ_ONLY , GL_RG32F);
    glBindImageTexture(2, pong, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);
    glDispatchCompute(numWorkGroups, numWorkGroups, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    std::swap(ping, pong);
  }

  if (logSize & 1)
    std::swap(ping, pong);

  shaderPermute.use();
  glBindImageTexture(0, ping, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RG32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FFT::generateMerge() {
  shaderMerge.use();
  shaderMerge.setUniform1f("u_lambda", lambda);
  shaderMerge.setUniform1f("u_dt", global::dt);
  glBindImageTexture(0, texDxDz  .getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(1, texDyDxz .getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(2, texDyxDyz.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(3, texDxxDzz.getId(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
  glBindImageTexture(4, texDisplacement.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY , GL_RGBA32F);
  glBindImageTexture(5, texDerivatives.getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY , GL_RGBA32F);
  glBindImageTexture(6, texTurbulence.getId(), 0, GL_FALSE, 0, GL_READ_WRITE , GL_R32F);
  glDispatchCompute(numWorkGroups, numWorkGroups, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void FFT::update() {
  generateWavesAtTime(global::time);
  generateIFFT(texDxDz, texBuffer);
  generateIFFT(texDyDxz, texBuffer);
  generateIFFT(texDyxDyz, texBuffer);
  generateIFFT(texDxxDzz, texBuffer);
  generateMerge();
}

} // namespace water

