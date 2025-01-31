#ifndef VULKANO_OFFSCREENRENDERING_H
#define VULKANO_OFFSCREENRENDERING_H

class voDeviceContext;
class voBuffer;
struct voRenderModel;


bool InitOffscreen( voDeviceContext * device, int width, int height );
bool CleanupOffscreen( voDeviceContext * device );

void DrawOffscreen( voDeviceContext * device, int cmdBufferIndex, voBuffer * uniforms, const voRenderModel * renderModels, const int numModels );

void Resize( voDeviceContext * device, int width, int height );

#endif //VULKANO_OFFSCREENRENDERING_H
