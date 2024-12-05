#include "basevsshader.h"
#include "insshader-constants.h"
#include "vshtmp9/overlay_vs11.inc"
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER( screeneffect_pain_dx8, "xENO's Pain Shader" )

    // Constant variables
    const float m_flPoint0[] = {  0.00f,  1.00f,  0.00f,  0.00f };
    const float m_flPoint1[] = { -0.87f, -0.50f,  0.00f,  0.00f };
    const float m_flPoint2[] = {  0.87f, -0.50f,  0.00f,  0.00f };

    const float m_flHaze[]   = { -0.25f, -1.25f, -1.00f,  0.00f };

    const float m_flBias[]   = {  1.00f,  1.00f,  1.00f,  0.00f };


    // Temporary storage
          float m_flTemp[4];

    // Scalar multiplication of float[4]
    void scale_float4( float *out, const float *in, const float in2 )
    {
        for( int i = 3; i >= 0; i-- )
        {
            out[i] = in[i] * in2;
        }
    }

    BEGIN_SHADER_PARAMS
    	SHADER_PARAM( blur,     SHADER_PARAM_TYPE_FLOAT, "0.0",
                      "Blur amount. 0.0 = none, 1.0 = blurry." )
		SHADER_PARAM( haze,     SHADER_PARAM_TYPE_FLOAT, "0.0", 
                      "Haze amount. 0.0 = none, 1.0 = opaque." )
	END_SHADER_PARAMS

    SHADER_INIT_PARAMS()
    {
        if( !params[blur]->IsDefined() ) {
            params[blur]->SetFloatValue( 0.0f );
        } else {
            params[blur]->SetFloatValue( clamp( params[blur]->GetFloatValue(), 0.0, 1.0 ) );
        }
        if( !params[haze]->IsDefined() ) {
            params[haze]->SetFloatValue( 0.0f );
        } else {
            params[haze]->SetFloatValue( clamp( params[haze]->GetFloatValue(), 0.0, 1.0 ) );
        }
    }

    SHADER_INIT
    {
    }

    SHADER_FALLBACK
    {
        if( !g_pHardwareConfig->SupportsVertexAndPixelShaders() )
        {
            return "UnlitGeneric";
        }
        return 0;
    }

    SHADER_DRAW
    {
        SHADOW_STATE
		{
            SetInitialShadowState( );

            // Setup texturing and coordinate transform
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
            pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );
            pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
            pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
#ifdef _XBOX
			pShaderShadow->SetColorSign( SHADER_TEXTURE_STAGE0, true );
#endif
            pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION | VERTEX_FORMAT_VERTEX_SHADER, 1, 0, 0, 0 );

            // Disable depth writes and depth testing!
            pShaderShadow->EnableDepthWrites( false );

            // Setup alpha-blending
	        s_pShaderShadow->EnableBlending( true );
	        s_pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
            pShaderShadow->EnableDepthWrites( false );

            // Setup shaders
            overlay_vs11_Static_Index vsIndex;
            vsIndex.SetBLUR( 1 );
            pShaderShadow->SetVertexShader( "overlay_vs11", vsIndex.GetIndex() );
			pShaderShadow->SetPixelShader( "pain_ps11", 0 );           
            DisableFog();
        }
        DYNAMIC_STATE
		{ 
            pShaderAPI->SetDefaultState();
#ifndef _XBOX
			pShaderAPI->SetTextureTransformDimension( 1, 0, true );
#endif

            // Set dynamic shader index
            s_pShaderAPI->SetVertexShaderIndex( 0 );

            // Setup basetexture transform
            SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, BASETEXTURETRANSFORM );

            // Setup blur offsets
            scale_float4( m_flTemp, m_flPoint0, params[blur]->GetFloatValue() );
            pShaderAPI->SetVertexShaderConstant( SHADER_SPECIFIC_CONST_2, m_flTemp );
            scale_float4( m_flTemp, m_flPoint1, params[blur]->GetFloatValue() );
            pShaderAPI->SetVertexShaderConstant( SHADER_SPECIFIC_CONST_3, m_flTemp );
            scale_float4( m_flTemp, m_flPoint2, params[blur]->GetFloatValue() );
            pShaderAPI->SetVertexShaderConstant( SHADER_SPECIFIC_CONST_4, m_flTemp );

            // Setup blur weights
            if( params[blur]->GetFloatValue() > 0.001 )
            {
                scale_float4( m_flTemp, m_flBias, 0.7f );
                pShaderAPI->SetPixelShaderConstant( 1, m_flTemp );
                scale_float4( m_flTemp, m_flBias, 0.1f );
                pShaderAPI->SetPixelShaderConstant( 2, m_flTemp );
            }
            else
            {
                scale_float4( m_flTemp, m_flBias, 1.0f );
                pShaderAPI->SetPixelShaderConstant( 1, m_flTemp );
                scale_float4( m_flTemp, m_flBias, 0.0f );
                pShaderAPI->SetPixelShaderConstant( 2, m_flTemp );
            }


            // Setup haze
            scale_float4( m_flTemp, m_flHaze, params[haze]->GetFloatValue() );
            m_flTemp[3] = GetAlpha( params );
            pShaderAPI->SetPixelShaderConstant( 0, m_flTemp );

            // Bind textures.
            BindTexture( SHADER_TEXTURE_STAGE0, BASETEXTURE, FRAME );
            BindTexture( SHADER_TEXTURE_STAGE1, BASETEXTURE, FRAME );
            BindTexture( SHADER_TEXTURE_STAGE2, BASETEXTURE, FRAME );
            BindTexture( SHADER_TEXTURE_STAGE3, BASETEXTURE, FRAME );
        }
        Draw();
    }
    
END_SHADER