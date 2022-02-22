#include "common.hlsl"

struct VS_OUTPUT {
	float4 pos       : POSITION;
	half2  texCoord  : TEXCOORD1;
	half2  maskCoord : TEXCOORD2;
	half2  texCoordR : TEXCOORD6;
	half2  texCoordB : TEXCOORD7;
	float4 viewVec   : TEXCOORD3;
	float3 lightVec  : TEXCOORD4;
	float3 hpos      : TEXCOORD5;
};

#ifdef VERTEX
VS_OUTPUT main(VS_INPUT In) {
	VS_OUTPUT Out;

	float3 coord = In.aCoord.xyz * INV_SHORT_HALF;

	float4 uv = float4(coord.x, coord.y, coord.x, -coord.y) * 0.5 + 0.5;
	Out.maskCoord = uv.xy * uRoomSize.zw;
	Out.texCoord  = uv.zw * uTexParam.zw;

#ifdef _GAPI_D3D9
	Out.texCoord += 0.5 * uTexParam.xy;
#endif

	coord           = float3(coord.x, 0.0, coord.y) * uPosScale[1].xyz + uPosScale[0].xyz;

	Out.pos         = mul(uViewProj, float4(coord, 1.0));
	Out.hpos        = Out.pos.xyw;
	Out.viewVec.xyz = uViewPos.xyz - coord;
	Out.lightVec    = uLightPos[0].xyz - coord;
	Out.viewVec.y   = abs(Out.viewVec.y);
	Out.lightVec.y  = abs(Out.lightVec.y);

	Out.viewVec.w   = step(uPosScale[0].y, uViewPos.y);

	Out.texCoordR = Out.texCoord + float2(uTexParam.x, 0.0);
	Out.texCoordB = Out.texCoord + float2(0.0, uTexParam.y);

	return Out;
}

#else // PIXEL

half4 main(VS_OUTPUT In) : COLOR0 {
	float3 viewVec = normalize(In.viewVec.xyz);

	float  value  = SAMPLE_2D_LINEAR(sNormal, In.texCoord).x;
	float3 normal = calcHeightMapNormal(In.texCoordR, In.texCoordB, value);

	float2 dudv = mul(uViewProj, float4(normal.x, 0.0, normal.z, 0.0)).xy;
	float3 rv = reflect(-viewVec, normal);
	float3 lv = normalize(In.lightVec);

	half spec = pow(max(0.0, dot(rv, lv)), 64.0) * 0.5;

	float2 tc = In.hpos.xy / In.hpos.z * 0.5 + 0.5;

	half4 refrA = SAMPLE_2D_LINEAR(sDiffuse, clamp(tc + dudv * uParam.z, 0.0, 0.999));
	half4 refrB = SAMPLE_2D_POINT(sDiffuse, tc);
	half3 refr  = lerp(refrA.xyz, refrB.xyz, refrA.w);

#ifdef _GAPI_D3D11_9_3 // TODO scale reflection matrix
	tc.x = 1.0 - tc.x;
	tc.y = 1.0 - tc.y;
#endif

	half3 refl  = SAMPLE_2D_LINEAR(sReflect, float2(tc.x, tc.y) + dudv * uParam.w).xyz;

	half fresnel = calcFresnel(abs(dot(normal, viewVec)), 0.12);

	half  mask  = SAMPLE_2D_POINT(sMask, In.maskCoord).r;
	half4 color = half4(lerp(refr, refl, fresnel), mask);
	color.xyz += spec * 1.5;

	float dist = (In.viewVec.y / viewVec.y) * In.viewVec.w;
	color.xyz *= lerp((half3)1.0, UNDERWATER_COLOR_H, (half)clamp(dist * WATER_COLOR_DIST, 0.0, 2.0));

	half fog = saturate(1.0h / exp(dist * WATER_FOG_DIST));
	color.xyz = lerp(UNDERWATER_COLOR_H * 0.2, color.xyz, fog);

	return color;
}

#endif
