#pragma once
#include "color-value-holder.h"

class ColorMixer {
public:
	static inline ColorValueHolder mix(const ColorValueHolder& a, const ColorValueHolder& b) {
		return ColorValueHolder((a.getRf() + b.getRf())/2, (a.getGf() + b.getGf())/2, (a.getBf() + b.getBf())/2);
	}
	static inline ColorValueHolder mix(const ColorValueHolder& a, const ColorValueHolder& b, float amt) {
		return ColorValueHolder((a.getRf() + (b.getRf() - a.getRf())*amt), (a.getGf() + (b.getGf() - a.getGf())*amt), (a.getBf() + (b.getBf() - a.getBf())*amt));
	}
	static inline ColorValueHolder mix(const ColorValueHolder* c, unsigned int num) {
		float colors[3] = { 0, 0, 0 };
		for (unsigned int i = 0; i < num; i++) {
			colors[0] += c[i].getRf();
			colors[1] += c[i].getGf();
			colors[2] += c[i].getBf();
		}
		return ColorValueHolder(colors[0]/num, colors[1]/num, colors[2]/num);
	}

	static ColorValueHolder HSVtoRGB(float hue, float saturation, float value); //[0,360], [0,1], [0,1]
	static ColorValueHolder HSVtoRGB_int(unsigned int hue, float saturation, float value); //fast path for integer hue
	static ColorValueHolder HSLtoRGB(float hue, float saturation, float light); //[0,360], [0,1], [0,1]
	static ColorValueHolder CMYKtoRGB(float cyan, float magenta, float yellow, float black); //[0,1]

private:
	ColorMixer() = delete;
	ColorMixer(const ColorMixer&) = delete;
};
