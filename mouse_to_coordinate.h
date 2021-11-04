#pragma once

float XM2C(float WinX, int XM) {
	return (XM / (WinX / 2)) - 1;
}

float YM2C(float WinY, int YM) {
	return 1 - (YM / (WinY / 2));
}

float XC2M(float WinX, float XC) {
	return (1 + XC) * (WinX / 2);
}

float YC2M(float WinY, float YC) {
	return (1 - YC) * (WinY / 2);
}