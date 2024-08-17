#ifndef QUICK
#define QUICK

template <typename T>
T median(T a, T b, T c)
{
   if ((a > b) ^ (a > c))
      return a;
   else if ((b < a) ^ (b < c))
      return b;
   else
      return c;
}

template <typename T>
void insertionSortForHybrid(T arr[], int left, int right)
{
   for (int i = left + 1; i <= right; i++)
   {
      T key = arr[i];
      int j = i - 1;
      while (j >= left && arr[j] > key)
      {
         arr[j + 1] = arr[j];
         j--;
      }
      arr[j + 1] = key;
   }
}

template <typename T>
void partition3(T arr[], int left, int right, int *i, int *j)
{

   int mid = (left + right) / 2;
   T pivot = median(arr[left], arr[mid], arr[right]);

   *i = left;
   *j = right;
   T aux;

   while (*i <= *j)
   {

      while (arr[*i] < pivot)
         (*i)++;

      while (arr[*j] > pivot)
         (*j)--;

      if (*i <= *j)
      {

         aux = arr[*i];
         arr[*i] = arr[*j];
         arr[*j] = aux;

         (*i)++;
         (*j)--;
      }
   }
}

template <typename T>
void MedianInsQuickSort(T arr[], int left, int right)
{

   int i, j;
   partition3(arr, left, right, &i, &j);

   if (left < j)
   {
      if (j - left > 50)
         MedianInsQuickSort(arr, left, j);
      else
         insertionSortForHybrid(arr, left, j);
   }

   if (i < right)
   {
      if (right - i > 50)
         MedianInsQuickSort(arr, i, right);
      else
         insertionSortForHybrid(arr, i, right);
   }
}

#endif