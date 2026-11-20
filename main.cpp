#include "WarehouseSystem.h"

int main() {
    WarehouseSystem warehouse;
    warehouse.showWelcome();

    if (!warehouse.loadData("."))
        return 1;

    warehouse.run();
    return 0;
}
