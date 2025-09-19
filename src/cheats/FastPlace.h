//
// Created by somepineaple on 4/29/22.
//

#ifndef PHANTOM_FASTPLACE_H
#define PHANTOM_FASTPLACE_H

#include "Cheat.h"

class FastPlace : public Cheat {
public:
    FastPlace();
void render(); 
    void run(Minecraft *mc) override;
private:
	float delay;
};


#endif //PHANTOM_FASTPLACE_H
