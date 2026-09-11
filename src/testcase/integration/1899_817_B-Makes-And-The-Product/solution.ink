// Translated from solution.cpp.

var M: dynamic = (1e5 + 3);

var N: dynamic = 4;

var mo: dynamic = (1e9 + 7);

var inf: dynamic = (1e18 + 1);

var mp: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(M);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&x));
      mp[x] += 1;
      i += 1;
    }
  }
  var cnt: dynamic = 0;
  var sum: dynamic = 0;
  for (var e: dynamic in mp)
  {
    a[cpp_update(cnt, "++")] = e.second;
    sum += e.second;
    if ((sum >= 3))
    {
      break;
    }
  }
  var res: dynamic = 0;
  if ((cnt == 1))
  {
    res = (((a[0] * ((a[0] - 1))) * ((a[0] - 2))) / 6);
  } else if ((cnt == 2))
  {
    if ((a[0] == 1))
    {
      res = ((a[1] * ((a[1] - 1))) / 2);
    } else
    {
      res = a[1];
    }
  } else
  {
    res = a[2];
  }
  printf("%I64d\n", res);
  return 0;
}
