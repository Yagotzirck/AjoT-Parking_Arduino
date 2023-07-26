#include "LcdController.h"

// Static variables' definitions
constexpr const char LcdController::numFreeStallsMsg[numRows][numColumns] = {
  "AjoT - Free",
  "stalls: %u"
};

 constexpr const char LcdController::enterOpenMsg[numRows][numColumns] = {
  "Your assigned",
  "stall is #%u"
};

constexpr const char LcdController::enterFullMsg[numRows][numColumns] = {
  "The parking lot",
  "is full!"
};

 constexpr const char LcdController::enterClosedMsg[numRows][numColumns] = {
  "The parking lot",
  "is closed!"
};

constexpr const char LcdController::exitMsg[numRows][numColumns] = {
  "Thank you for",
  "choosing AjoT!"
};
