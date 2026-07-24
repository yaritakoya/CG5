#include "Test.hlsli"

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float32_t2 uv = input.texcoord;
    //https://learn.microsoft.com/ja-jp/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math
    //位置セットかカラーセットでアクセスできる
    output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return output;
}