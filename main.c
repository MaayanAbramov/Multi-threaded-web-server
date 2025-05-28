#include "stdlib.h"
#include "queue.h"
// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {

    Queue  q= create();
    assert(dequeue(q) == -1 );
    assert(dequeue(q) == -1 );

    for (int i = 1; i <= 1000; i++) {
        struct timeval time={i,i};
        enqueue(q,i,time,time);
        assert(getSize(q) == i);
    }
    for (int i = 1; i <= 1000; ++i) {
        assert(dequeue(q) == i);
        assert(getSize(q) == 1000-i);
    }
    assert(getSize(q) == 0);
    return 0;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.