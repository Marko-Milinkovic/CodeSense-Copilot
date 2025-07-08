#include "MainLogicController.h"

int main() {

    MainLogicController controller;

    // Step 1: Load dictionary
    controller.start_program();

    // Step 2: Start user interaction loop
    controller.interactive_loop();

    // Step 3: Save changes before exit
    controller.exit_program();

    return 0;
}
