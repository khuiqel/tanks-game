#pragma once
#include <vector>

#include "color-value-holder.h"

class GeneralizedLava {
protected:
	//note to anyone deciding to use nested classes: simply "struct LavaBubble;" is enough here (allowed because only pointers to this object are used)
	//the struct can then be given a header-like definition at the top of the source file, then actual code at the bottom, if that's what you want
	struct LavaBubble {
	protected:
		//floats, not doubles, since it's not like you can see the difference anyway (also less implicit casting)
		float r;
		float xStart; //[0,1] (rect) or [-1,-1] (circle)
		float yStart; //[0,1] (rect) or [-1,-1] (circle)
		float xEnd;   //[0,1] (rect) or [-1,-1] (circle)
		float yEnd;   //[0,1] (rect) or [-1,-1] (circle)
		//if circle, x and y are constrained by the circle's edges (sine and cosine are involved)

		int tickCount;
		float tickMax; //float just to avoid an explicit cast for finding the alpha when bubble is rising or falling

		float stateMultiplier[4];
		unsigned int state;
		// 0 = rising
		// 1 = moving
		// 2 = "popping" (falling? fading?)
		// 3 = dead (don't use)

	public:
		inline bool isDead() const noexcept { return (state >= 3); }
		void tick();
		float getAlpha() const noexcept;
		float getX() const noexcept;
		float getY() const noexcept;
		float getR() const noexcept;

		LavaBubble() { state = 3; }
		LavaBubble(float radius, float x0, float y0, float x1, float y1, float tickStart);
		LavaBubble(float radius, float x0, float y0, float x1, float y1, float tickStart, const float* tickMultiplier); //tickMultiplier length = 3
		//LavaBubble(const LavaBubble&); //no need
	};

protected:
	double tickCount;
	double tickCycle; //this is used to make the brightness of the lava vary sinusoidally

	unsigned int maxBubbles = 8; //there wasn't a limit in JS Tanks because the bubbles were rare (1/400 per tick, 100Hz)
	float bubbleChance; //= 1.0/400 (chance to spawn bubble this tick)
	std::vector<LavaBubble> bubbles; //size is fixed once created, dead bubbles remain where they are
	virtual void tryToPushNewBubble(float bubbleRadius) = 0; //write over the first dead bubble

public:
	ColorValueHolder getBackgroundColor() const;
	ColorValueHolder getBackgroundColor_Pose() const;
	ColorValueHolder getBubbleColor(const LavaBubble& bubble) const;
	ColorValueHolder getBubbleColor_Pose(const LavaBubble& bubble) const;

	virtual void tick();

	virtual ~GeneralizedLava();
};
