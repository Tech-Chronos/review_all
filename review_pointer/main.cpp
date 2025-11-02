#include <iostream>
#include <cstdio>
using namespace std;

void test1()
{
    int a = 0;
    const int *p = &a;
    ///*p = 20;

    int* const q = &a;
    cout << *p << endl;
}

void test2()
{
    int arr[10] = {};

    // sizeof 数组名是数组的所有元素
    size_t  sz = sizeof arr / sizeof arr[0];
    for (int i = 0; i < sz; ++i)
    {
        cout << arr[i] << endl;
    }
}

void swap(int* left, int* right)
{
    int tmp = *left;
    *left = * right;
    *right = tmp;
}

void bubble_sort(int* arr, int sz)
{
    int* slow = arr;
    int* fast = arr + 1;
    for (int i = 0; i < sz - 1; ++i)
    {
        int flag = 1;
        while (fast < arr + sz - i - 1)
        {
            if (*slow > *fast)
            {
                flag = 0;
                swap(fast, slow);
            }
            ++fast;
            ++slow;
        }
        if (flag == 1)
        {
            break;
        }
        slow = arr;
        fast = arr + 1;
    }

}


// 指针数组
void test3()
{
    int arr1[] = {1,2,3,4,5};
    int arr2[] = {2,3,4,5,6};
    int arr3[] = {3,4,5,6,7};

    int* arr_ptr[3] = {arr1, arr2, arr3};

    //size_t size = sizeof(arr_ptr) / sizeof(arr_ptr[0]);

    for (size_t i = 0; i < 3; ++i)
    {
        for (size_t j = 0; j < 5; ++j)
        {
            cout << arr_ptr[i][j] << " ";
        }
        cout << endl;
    }
}

int main()
{
    test3();
    return 0;
}
//int main()
//{
//    int arr[] = {3,1,7,5,8,9,0,2,4,6};
//    int arr2[] = {4,1};
//    cout << "before sort: ";
//    for (auto e : arr2)
//    {
//        cout << e;
//    }
//    cout << endl;
//
//    bubble_sort(arr2, sizeof(arr2) / sizeof(arr2[0]));
//
//    cout << "after sort: ";
//    for (auto e : arr2)
//    {
//        cout << e;
//    }
//    cout << endl;
//
//    return 0;
//}