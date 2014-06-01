/*
#include <windows.h> // for FLOAT definition
#include <xnamath.h>
#include <iostream>
using namespace std;

int main()
{
	cout.precision(8);

	// Check support for SSE2 (Pentium4, AMD K8, and above).
	if( !XMVerifyCPUSupport() )
	{
		cout << "xna math not supported" << endl;
		return 0;
	}
 
	XMVECTOR u = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	XMVECTOR n = XMVector3Normalize(u);

	float LU = XMVectorGetX(XMVector3Length(n));
	
	// Mathematically, the length should be 1.  Is it numerically?
	cout << LU << endl;
	if( LU == 1.0f )
		cout << "Length 1" << endl;
	else
		cout << "Length not 1" << endl;

	// Raising 1 to any power should still be 1.  Is it?
	float powLU = powf(LU, 1.0e6f);
	cout << "LU^(10^6) = " << powLU << endl;
}
*/