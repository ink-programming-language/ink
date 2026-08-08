// Translated from solution.cpp.

var M = (1e5 + 3);

var N = 4;

var mo = (1e9 + 7);

var inf = (1e18 + 1);

var mp: dynamic;

var a = cpp_array(M);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      scanf("%d", (&x));
      mp[x] += 1;
      i += 1;
    }
  }
  var cnt = 0;
  var sum = 0;
  for (var e in mp)
  {
    a[cpp_update(cnt, "++")] = e.second;
    sum += e.second;
    if ((sum >= 3))
    {
      break;
    }
  }
  var res = 0;
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
