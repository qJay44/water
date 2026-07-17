#pragma once

#include "../engine/Shader.hpp"
#include "../engine/texture/Texture2D.hpp"
#include "../engine/mesh/BufferObject.hpp"
#include "general.hpp"

namespace water {

struct Tessendorf {
  struct SpectrumSettingsGUI {
    float scale;
    float windSpeed;
    float windDir;
    float fetch;
    float spreadBlend;
    float swell;
    float peakEnhancemnt;
    float shortWavesFade;
  };

  struct SpectrumSettings {
    float scale;
    float angle;
    float spreadBlend;
    float swell;
    float alpha;
    float peakOmega;
    float gamma;
    float shortWavesFade;
  };
  static_assert(sizeof(SpectrumSettings) % 16 == 0);

  struct {
    BufferObject spectrums{GL_UNIFORM_BUFFER};
  } ubo;

  static bool isCreated;
  float worldSize = 256.f;

  int& size = texResolution;
  int logSize;

  float seed1 = 13.37f;
  float seed2 = 42.f;

  SpectrumSettingsGUI local{
    .scale = 1.f,
    .windSpeed = 0.5f,
    .windDir = glm::radians(-29.81f),
    .fetch = 1e5f,
    .spreadBlend = 1.f,
    .swell = 0.198f,
    .peakEnhancemnt = 3.3,
    .shortWavesFade = 0.01f
  };

  SpectrumSettingsGUI swell{
    .scale = 0.f,
    .windSpeed = 1.f,
    .windDir = 0.f,
    .fetch = 3e5f,
    .spreadBlend = 1.f,
    .swell = 1.f,
    .peakEnhancemnt = 3.3,
    .shortWavesFade = 0.01f
  };

  SpectrumSettings spectrums[2];

  float g = 9.81f;
  float depth = 500.f; // D
  float lengthScale = 5;
  float lambda = 1.f;

  Shader shaderButterfly         { "water/fft/butterfly.comp"         };
  Shader shaderNoise             { "water/fft/noise.comp"             };
  Shader shaderInitialSpectrum   { "water/fft/initialSpectrum.comp"   };
  Shader shaderConjugateSpectrum { "water/fft/conjugateSpectrum.comp" };
  Shader shaderTimeSpectrum      { "water/fft/timeSpectrum.comp"      };
  Shader shaderIFFT_horizontal   { "water/fft/ifft_horizontal.comp"   };
  Shader shaderIFFT_vertical     { "water/fft/ifft_vertical.comp"     };
  Shader shaderPermute           { "water/fft/permute.comp"           };
  Shader shaderMerge             { "water/fft/merge.comp"             };

  Texture2D texButterfly;
  Texture2D texNoise;
  Texture2D texInitialSpectrum;
  Texture2D texPrecomputedData;
  Texture2D texBuffer;
  Texture2D texDxDz;
  Texture2D texDyDxz;
  Texture2D texDyxDyz;
  Texture2D texDxxDzz;
  Texture2D texDisplacement;
  Texture2D texDerivatives;
  Texture2D texTurbulence;

  Tessendorf();

  void rebuild();

  float JonswapAlpha(float g, float fetch, float windSpeed);
  float JonswapPeakFrequency(float g, float fetch, float windSpeed);

  void fillSettings(const SpectrumSettingsGUI& display, SpectrumSettings& settings);

  void generatePrecomputedTwiddleFactorsAndInputIndices();
  void generateNoise();
  void generateInitials();
  void generateWavesAtTime(float time);
  void generateIFFT(Texture2D& input, Texture2D& buffer);
  void generateMerge();

  void update();
};

} // namespace water

