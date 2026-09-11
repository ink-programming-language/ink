// Translated from solution.cpp.

var as_cpp: dynamic = cpp_uninitialized();

var a: dynamic = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43];

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  as_cpp.push_back(1);
  scanf("%d", (&n));
  {
    j = 0;
    while ((as_cpp.size() <= n))
    {
      {
        i = 0;
        while ((i < as_cpp.size()))
        {
          if (((as_cpp[i] * a[j]) <= ((n * n) * 2)))
          {
            as_cpp.push_back((as_cpp[i] * a[j]));
          }
          i += 1;
        }
      }
      j += 1;
    }
  }
  sort(as_cpp.begin(), as_cpp.end());
  {
    i = (as_cpp.size() - n);
    while ((i < as_cpp.size()))
    {
      printf("%d ", as_cpp[i]);
      i += 1;
    }
  }
  return 0;
}
