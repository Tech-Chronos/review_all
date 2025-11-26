#include "Heap.h"

int main()
{
    Heap<int> hp;

    int arr[10] = {2,21,1,98,100,19,0,71,30,77};

    for (auto element : arr)
    {
        hp.Insert(element);
    }

    hp.Print();

    while (hp.size() != 0)
    {
        std::cout << hp.erase() << " ";
    }
    return 0;
}
