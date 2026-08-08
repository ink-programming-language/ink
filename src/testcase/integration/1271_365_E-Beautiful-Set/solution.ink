// Translated from solution.cpp.

var as_cpp: dynamic;

var a = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43];

func main()
{
  var i: dynamic;
  var j: dynamic;
  var n: dynamic;
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
