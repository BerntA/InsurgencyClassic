#include "macros.vsh"

;-------------------------------------------------------------------------------
;	 $SHADER_SPECIFIC_CONST_0-$SHADER_SPECIFIC_CONST_1 = Texture 0 transform
;	 $SHADER_SPECIFIC_CONST_2-$SHADER_SPECIFIC_CONST_3 = Texture 1 transform
;	 $SHADER_SPECIFIC_CONST_4-$SHADER_SPECIFIC_CONST_5 = Texture 2 transform
;	 $SHADER_SPECIFIC_CONST_6-$SHADER_SPECIFIC_CONST_7 = Texture 3 transform
;                            OR
;    $SHADER_SPECIFIC_CONST_0-$SHADER_SPECIFIC_CONST_1 = Texture transform
;    $SHADER_SPECIFIC_CONST_2-$SHADER_SPECIFIC_CONST_3 = Blue points
;-------------------------------------------------------------------------------

sub Overlay
{
	local( $blur ) = shift;
	local( $tex1 ) = shift;
	local( $tex2 ) = shift;
	local( $tex3 ) = shift;
	local( $vertexcolor ) = shift;

	local( $wolrdPos, $projPos );

 	; ----------------------------------------
	;  Vertex blending
    ; ----------------------------------------
	&AllocateRegister( \$worldPos );
	&SkinPosition( $worldPos );

    ; ----------------------------------------
	;  Transform the position from world
	;  to projected space
	; ----------------------------------------
	&AllocateRegister( \$projPos );

	dp4 $projPos.x, $worldPos, $cViewProj0
	dp4 $projPos.y, $worldPos, $cViewProj1
	dp4 $projPos.z, $worldPos, $cViewProj2
	dp4 $projPos.w, $worldPos, $cViewProj3
	mov oPos, $projPos

    ; ----------------------------------------
	;  Texture Coordinates
	; ----------------------------------------

	if( $blur )
	{
		;
		;	Blurred texture coords
		;
 		mov $worldPos, $SHADER_SPECIFIC_CONST_2

		; Calculate center point
		dp4 $projPos.x, $vTexCoord0, $SHADER_SPECIFIC_CONST_0
		dp4 $projPos.y, $vTexCoord0, $SHADER_SPECIFIC_CONST_1

		; Calculate Coordinates based on center point
		mov oT0, $projPos
		add oT1, $SHADER_SPECIFIC_CONST_2, $projPos
		add oT2, $SHADER_SPECIFIC_CONST_3, $projPos
		add oT3, $SHADER_SPECIFIC_CONST_4, $projPos

	}
	else
	{
		;
		; Texture0
		;
		dp4 oT0.x, $vTexCoord0, $SHADER_SPECIFIC_CONST_0
		dp4 oT0.y, $vTexCoord0, $SHADER_SPECIFIC_CONST_1

		if( $tex1 )
		{
			;
			; Texture1
			;
			dp4 oT1.x, $vTexCoord0, $SHADER_SPECIFIC_CONST_2
			dp4 oT1.y, $vTexCoord0, $SHADER_SPECIFIC_CONST_3
		}

		if( $tex2 )
		{
		    ;
			; Texture2
			;
			dp4 oT2.x, $vTexCoord0, $SHADER_SPECIFIC_CONST_4
			dp4 oT2.y, $vTexCoord0, $SHADER_SPECIFIC_CONST_5
		}

		if( $tex3 )
		{
			;
			; Texture3
			;
			dp4 oT3.x, $vTexCoord0, $SHADER_SPECIFIC_CONST_6
			dp4 oT3.y, $vTexCoord0, $SHADER_SPECIFIC_CONST_7
		}
	}

	if( $vertexcolor )
	{
		;
		; Modulation Color
		;
		mul oD0, $vColor, $cModulationColor
	}
	else
	{
		;
		; Modulation Color
		;
		mov oD0, $cModulationColor
	}

	; ----------------------------------------
	;  Free unused registers
	; ----------------------------------------
	&FreeRegister( \$worldPos );
	&FreeRegister( \$projPos );

}


