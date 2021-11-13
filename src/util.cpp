#include "util.h"

void quickSort(doorDayAlarm_t arr[], int left, int right) {
      int i = left, j = right;
      doorDayAlarm_t tmp;
      int pivot = arr[(left + right) / 2].DoW*24*60+arr[(left + right) / 2].hour*60+arr[(left + right) / 2].minute;
 
      /* partition */
      while (i <= j) {
            while (arr[i].DoW*24*60+arr[i].hour*60+arr[i].minute < pivot)
                  i++;
            while (arr[j].DoW*24*60+arr[j].hour*60+arr[j].minute > pivot)
                  j--;
            if (i <= j) {
                  tmp = arr[i];
                  arr[i] = arr[j];
                  arr[j] = tmp;
                  i++;
                  j--;
            }
      };
 
      /* recursion */
      if (left < j)
            quickSort(arr, left, j);
      if (i < right)
            quickSort(arr, i, right);
}