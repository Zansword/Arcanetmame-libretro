/* stvprot.h */

void install_standard_protection(running_machine *machine);
void install_twcup98_protection(running_machine *machine);
void install_decathlt_protection(running_machine *machine);
void install_astrass_protection(running_machine *machine);

void stv_register_protection_savestates(running_machine *machine);
void stv_reset_protection_state(running_machine *machine);

