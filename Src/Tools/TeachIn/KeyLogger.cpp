#include "KeyLogger.h"
#include "Representations/BehaviorControl/TI/TIData.h"

KeyLogger *KeyLogger::keyLoggerInstance = 0;

KeyLogger *KeyLogger::getInstance()
{
    if (!keyLoggerInstance)
    {
        keyLoggerInstance = new KeyLogger();
    }
    return keyLoggerInstance;
}

int64_t KeyLogger::now() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t KeyLogger::getKeyHitTime(int key) const
{
    switch (key)
    {
    case Keys::R1:
        return r1_lastHit;
    case Keys::R2:
        return r2_lastHit;
    case Keys::R3:
        return r3_lastHit;
    case Keys::L1:
        return l1_lastHit;
    case Keys::L2:
        return l2_lastHit;
    case Keys::L3:
        return l3_lastHit;
    case Keys::Cross:
        return cross_lastHit;
    case Keys::Square:
        return square_lastHit;
    case Keys::Circle:
        return circle_lastHit;
    case Keys::Triangle:
        return triangle_lastHit;
    case Keys::Share:
        return share_lastHit;
    case Keys::Home:
        return home_lastHit;
    case Keys::Start:
        return start_lastHit;
    default:   // default shoudn't trigger
        break; 
    }
    return 0;
}

void KeyLogger::setKeyHitTime(int key, int64_t time)
{
    switch (key)
    {
    case Keys::R1:
        r1_lastHit = time;
			  return;
    case Keys::R2:
        r2_lastHit = time;
			  return;
    case Keys::R3:
        r3_lastHit = time;
			  return;
    case Keys::L1:
        l1_lastHit = time;
			  return;
    case Keys::L2:
        l2_lastHit = time;
			  return;
    case Keys::L3:
        l3_lastHit = time;
			  return;
    case Keys::Cross:
        cross_lastHit = time;
			  return;
    case Keys::Square:
        square_lastHit = time;
			  return;
    case Keys::Circle:
        circle_lastHit = time;
			  return;
    case Keys::Triangle:
        triangle_lastHit = time;
			  return;
    case Keys::Share:
        share_lastHit = time;
			  return;
    case Keys::Home:
        home_lastHit = time;
			  return;
    case Keys::Start:
        start_lastHit = time;
			  return;
    default:  //default shoudn't trigger
			  return;
			   
    }
}

bool KeyLogger::keyPressedJustNow(int usedKey, int timePeriod)
{
    if (now() < getKeyHitTime(usedKey) + timePeriod)
    {
        setKeyHitTime(usedKey, 0);
        return true;
    }
    return false;
}

int KeyLogger::getKey() const
{
    //obvious
    return mKey;
}

Vector2f KeyLogger::getLeftCursor() const
{
    return leftCursor;
}

Vector2f KeyLogger::getRightCursor() const
{
    return rightCursor;
}

bool KeyLogger::isFastMode() const
{
    return fastMode;
}

void KeyLogger::toggleFastMode(int usedKey)
{
    mtx.lock();
    if (keyPressedJustNow(usedKey, 500))
    {
        fastMode = !fastMode;
    }
    mtx.unlock();
}

bool KeyLogger::isHeadUp() const
{
    return headUp;
}

void KeyLogger::toggleHead()
{
    mtx.lock();
    if (keyPressedJustNow(Keys::Home, 500))
    {
        headUp = !headUp;
    }
    mtx.unlock();
}

void KeyLogger::setKey(int key)
{
    mtx.lock();
    //remember the time of the button hit in ms
    setKeyHitTime(key, now());
    // OUTPUT_TEXT("Key " << key << " pressed." // set output with key number
    
    if (keyEvents.size() >= 10) keyEvents.pop();
    keyEvents.push(key);

    mKey = key;
    if (key == Keys::Start)
    {
        toggleStart();
    }
    mtx.unlock();
}

bool KeyLogger::hasEvent() const 
{
    return !keyEvents.empty();
}

int KeyLogger::nextEvent() 
{
    auto ret = keyEvents.front();
    keyEvents.pop();
    return ret;
}

void KeyLogger::clearEvents()
{
    while (!keyEvents.empty())
    {
        keyEvents.pop();
    }
    
}

bool KeyLogger::isStartPressed() const
{
    return startGamepadControl;
}

void KeyLogger::toggleStart()
{
    if (keyPressedJustNow(Keys::Start, 500))
    {
      startGamepadControl = !startGamepadControl;
    }
}

void KeyLogger::forceToToggleStart()
{
    startGamepadControl = !startGamepadControl;
}

void KeyLogger::setLeftCursor(float x, float y)
{
    mtx.lock();
    leftCursor = Vector2f(x, y);
    mtx.unlock();
}

void KeyLogger::setLeftCursor(Vector2f left)
{
    mtx.lock();
    leftCursor = left;
    mtx.unlock();
}

void KeyLogger::setRightCursor(float x, float y)
{
    mtx.lock();
    rightCursor = Vector2f(x, y);
    mtx.unlock();
}

void KeyLogger::setRightCursor(Vector2f right)
{
    mtx.lock();
    rightCursor = right;
    mtx.unlock();
}

void KeyLogger::protocol(PlaybackAction* playbackAction)
{
    // add data to the protocol
    mtx.lock();
    if (lastGamePlayTime + protocolIntervals <= playbackAction->maxTime)
    {
        if (recordingStartTime == 0)
        {
					recordingStartTime = playbackAction->maxTime;
        }
        /* ToDo: rapidCSV
        */
    }

    mtx.unlock();
}

void KeyLogger::saveProtocol()
{
    /* ToDo: rapidCSV
    saving stuff
    */
  OUTPUT_TEXT("saving stuff");
}

void KeyLogger::setRobotName(std::string name)
{
    mtx.lock();
    robotName = name;
    mtx.unlock();
}

std::string KeyLogger::getRobotName() const
{
    return robotName;
}

int KeyLogger::getRobotNumber() const
{
    std::string numberOutName = robotName.substr(5, robotName.length() - 1);
    return std::stoi(numberOutName);
}

float KeyLogger::getProtocolInterval() const{
    return protocolIntervals; 
}

void KeyLogger::toggleSaving()
{
    mtx.lock();
    if (keyPressedJustNow(Keys::Share, 500))
    {
        saving = !saving;
    }
    mtx.unlock();
}