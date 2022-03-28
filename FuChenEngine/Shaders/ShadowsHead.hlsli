//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************
/*"DescriptorTable(b0, visibility = SHADER_VISIBILITY_VERTEX), " \*/
#define FuChenSample_ShadowSig \
	"RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), " \
	"CBV(b0, visibility = SHADER_VISIBILITY_VERTEX), " \
	"CBV(b1, visibility = SHADER_VISIBILITY_VERTEX), " \
	"CBV(b2, visibility = SHADER_VISIBILITY_VERTEX), " \
	"RootConstants(b3, num32BitConstants = 4), " \
	"StaticSampler(s0," \
		"addressU = TEXTURE_ADDRESS_WRAP," \
		"addressV = TEXTURE_ADDRESS_WRAP," \
		"addressW = TEXTURE_ADDRESS_WRAP," \
		"filter = FILTER_MIN_MAG_MIP_POINT)"

