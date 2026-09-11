// Translated from solution.cpp.

func main() -> dynamic
{
  var arr: dynamic = [];
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var m: dynamic = cpp_uninitialized();
      scanf("%d", (&m));
      arr[m] = 1;
      i += 1;
    }
  }
  var c: dynamic = 0;
  var ans: dynamic = 0;
  var i: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i <= 90))
    {
      c += 1;
      if ((arr[i] != 0))
      {
        c = 0;
      }
      if ((c == 15))
      {
        break;
      }
      i += 1;
    }
  }
  if ((i == 91))
  {
    i -= 1;
  }
  write(i);
}
