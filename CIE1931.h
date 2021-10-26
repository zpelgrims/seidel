#pragma once
class CIE1931
{
public:
	static float3 WavelengthRGB( const float wavelength );
	static float3 WavelengthXYZ( const float wavelength );
	static float3 XYZtoCIELAB( const float3 XYZ );
	static float CIEDE2000( const float3& lab1, const float3& lab2 );
private:
	static float f( const float t );
	static constexpr double deg2Rad( const double deg );
	static constexpr double rad2Deg( const double rad );
};
