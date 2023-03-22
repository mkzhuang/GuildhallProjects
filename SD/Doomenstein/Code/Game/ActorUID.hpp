#pragma once

struct ActorUID
{
public:
	ActorUID();
	ActorUID(int index, int salt);

	void Invalidate();
	bool IsValid() const;
	int GetIndex() const;
	bool operator==(ActorUID const& other) const;
	bool operator!=(ActorUID const& other) const;

	static const ActorUID INVALID;

private:
	int m_data = 0xffffffff;
};

