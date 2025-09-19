

//
// Created by Kirby! 
//

#ifndef NEBULA_AUTOSPRINT_H
#define NEBULA_AUTOSPRINT_H

#include "Cheat.h"


class AutoSprint : public Cheat {
public:
  AutoSprint();
  void run(Minecraft *mc) override;

private:
  float delay;
};


#endif // PHANTOM_FASTPLACE_H
