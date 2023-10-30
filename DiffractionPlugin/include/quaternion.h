#pragma once

#pragma region quaternion
class quaternion
{
public:
	quaternion(const float w_ = 0, const float x_ = 0, const float y_ = 0, const float z_ = 0) : w(w_), x(x_), y(y_), z(z_) {}
	~quaternion() {}

	float w;
	float x;
	float y;
	float z;

private:
};
#pragma endregion