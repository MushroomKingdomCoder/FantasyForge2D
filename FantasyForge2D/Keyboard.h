#pragma once
#include <bitset>
#include <queue>
#include <optional>

class Keyboard
{
	friend class Window;
public:
	static constexpr unsigned char nKeys = 255u;
	static constexpr unsigned char BufferSize = 32u;
public:
	class Event
	{
	public:
		enum class Type
		{
			KeyDown,
			KeyUp,
			Char
		};
	private:
		unsigned char keycode = 0u;
		std::optional<Type> type = std::optional<Type>();
	public:
		Event() = default;
		Event(unsigned char keycode, Type type)
			:
			keycode(keycode),
			type(type)
		{}
		unsigned char GetKeycode() const
		{
			return keycode;
		}
		Type GetType() const
		{
			return *type;
		}
		bool isKeyDown() const
		{
			return *type == Type::KeyDown;
		}
		bool isKeyUp() const
		{
			return *type == Type::KeyUp;
		}
		bool isChar() const
		{
			return *type == Type::Char;
		}
		bool isEndOfQueue() const
		{
			return !type;
		}
	};
private:
	std::bitset<nKeys> keystates = std::bitset<nKeys>();
	std::queue<Event> eventQueue = std::queue<Event>();
	std::queue<unsigned char> charBuffer = std::queue<unsigned char>();
	bool autorepeat = false;
private:
	void OnKeyDown(unsigned char keycode);
	void OnKeyUp(unsigned char keycode);
	void OnChar(unsigned char character);
	template <typename buffer>
	void TrimBuffer(buffer& buf)
	{
		while (buf.size() >= BufferSize)
		{
			buf.pop();
		}
	}
public:
	Keyboard() = default;
	bool KeyIsPressed(unsigned char key) const;
	bool isInUse() const;
	void ClearKeystates();
	Event Read();
	bool isActive() const;
	void EmptyEventQueue();
	unsigned char Get();
	bool isEmpty() const;
	void EmptyCharacterBuffer();
	bool AutorepeatIsEnabled() const;
	void EnableAutorepeat();
	void DisableAutorepeat();
};

