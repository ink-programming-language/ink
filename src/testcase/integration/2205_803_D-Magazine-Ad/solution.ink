// Translated from solution.cpp.

var maxn = (1e6 + 10);

var mod = (1e9 + 7);

var inf = 0x3f3f3f3f;

var INF = 0x3f3f3f3f3f3f3f3f;

var eps = 1e-7;

var n: dynamic;

var k: dynamic;

var m: dynamic;

var T: dynamic;

var cnt = 0;

var edge = cpp_array(maxn);

var mp = cpp_array(2000, 2000);

var judge = cpp_array(2000, 2000);

var arr = cpp_array(maxn);

var str = cpp_array(maxn);

func GCD(a: dynamic, b: dynamic)
{
  while (cpp_assign(b, "^=", cpp_assign(a, "^=", cpp_assign(b, "^=", cpp_assign(a, "%=", b)))))
  {
  }
  return a;
}

func check(mid: dynamic)
{
  var sum = 0;
  var res = 0;
  {
    var i = 0;
    while ((i < cnt))
    {
      if ((arr[i] > mid))
      {
        return 0;
      }
      if (((sum + arr[i]) > mid))
      {
        res += 1;
        sum = 0;
      }
      sum += arr[i];
      i += 1;
    }
  }
  if ((sum != 0))
  {
    res += 1;
    sum = 0;
  }
  return (res <= k);
}

func main()
{
  scanf("%d", (&k));
  getchar();
  gets(str);
  var len = strlen(str);
  var j = 0;
  {
    var i = 0;
    while ((i < len))
    {
      j += 1;
      if (((str[i] == cpp_char(" ")) || (str[i] == cpp_char("-"))))
      {
        arr[cpp_update(cnt, "++")] = j;
        j = 0;
      }
      i += 1;
    }
  }
  arr[cpp_update(cnt, "++")] = j;
  j = 0;
  var l = 0;
  var r = len;
  var ans = 0;
  while (((r - l) >= 0))
  {
    var mid = (((l + r)) / 2);
    if (check(mid))
    {
      r = (mid - 1);
      ans = mid;
    } else
    {
      l = (mid + 1);
    }
  }
  printf("%d\n", ans);
  return 0;
}
