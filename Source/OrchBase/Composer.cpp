/* A file of stubs. */
class Composer;
#include "Composer.h"
#warning "Composer: Structure with no name (UnnamedStruct) can't be compiled."
Composer::Composer(Placer*){}
Composer::Composer(){}
int Composer::compose(GraphI_t*){return 0;}
int Composer::generate(GraphI_t*){return 0;}
int Composer::compile(GraphI_t*){return 0;}
void Composer::decompose(GraphI_t*){}
void Composer::clean(GraphI_t*){}
void Composer::reset(){}
void Composer::setOutputPath(std::string){}
void Composer::setPlacer(Placer*){}
