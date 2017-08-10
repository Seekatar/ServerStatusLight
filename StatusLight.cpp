#include "StatusLight.h"

bool StatusLight::initialize()
{
  _blue = _strip->Color(0, 0, 255);
  _green = _strip->Color(0, 255, 0);
  _red = _strip->Color(150, 0, 0);
  _brightred = _strip->Color(255, 0, 0);
  _yellow = _strip->Color(255, 255, 0);
  _orange = _strip->Color(200, 100, 0);
  _gray = _strip->Color(30,30,30);
  _cyan = _strip->Color(0,255,255);
  _purple = _strip->Color(255,0,255);
  
  return true;
}

uint32_t StatusLight::mapServerColor(SystemStatus::ServerStatus stat )
{
  switch ( stat )
  {
    case SystemStatus::ServerStatus::Blue:
      return _blue;
    case SystemStatus::ServerStatus::Green:
      return _green;
    case SystemStatus::ServerStatus::Yellow:
      return _yellow;
    case SystemStatus::ServerStatus::Orange:
      return _orange;
    case SystemStatus::ServerStatus::Red:
      return _red;
    case SystemStatus::ServerStatus::BrightRed:
      return _brightred;
    default:
      return 0;
  }
}

uint32_t StatusLight::mapBuildColor(SystemStatus::BuildStatus stat )
{
  switch ( stat )
  {
    case SystemStatus::BuildStatus::Failure:
      return _red;
    case SystemStatus::BuildStatus::Success:
      return _green;
    case SystemStatus::BuildStatus::Staged:
    case SystemStatus::BuildStatus::Processing:
      return _blue;
    case SystemStatus::BuildStatus::Canceled:
    case SystemStatus::BuildStatus::NotRunYet:
      return _gray;
    case SystemStatus::BuildStatus::Pending:
      return _purple;
    default:
      return 0;
  }
}

