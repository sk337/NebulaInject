
//
// Created by somepineaple on 4/29/22.
//

#ifndef NEBULA_NOHITDELAY_H
#define NEBULA_NOHITDELAY_H

#include "Cheat.h"

class NoHitDelay : public Cheat {
public:
  NoHitDelay();

  void run(Minecraft *mc) override;

private:
  float delay;
};

#endif 
