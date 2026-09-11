// Translated from solution.cpp.

var maxn: dynamic = (1e6 + 10);

var mod: dynamic = (1e9 + 7);

var inf: dynamic = 0x3f3f3f3f;

var INF: dynamic = 0x3f3f3f3f3f3f3f3f;

var eps: dynamic = 1e-7;

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var cnt: dynamic = 0;

var edge: dynamic = cpp_array(maxn);

var mp: dynamic = cpp_array(2000, 2000);

var judge: dynamic = cpp_array(2000, 2000);

var arr: dynamic = cpp_array(maxn);

var str: dynamic = cpp_array(maxn);

func GCD(a: dynamic, b: dynamic) -> dynamic
{
  while (cpp_assign(b, "^=", cpp_assign(a, "^=", cpp_assign(b, "^=", cpp_assign(a, "%=", b)))))
  {
  }
  return a;
}

func check(mid: dynamic) -> dynamic
{
  var sum: dynamic = 0;
  var res: dynamic = 0;
  {
    var i: dynamic = 0;
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

func main() -> dynamic
{
  scanf("%d", (&k));
  getchar();
  gets(str);
  var len: dynamic = strlen(str);
  var j: dynamic = 0;
  {
    var i: dynamic = 0;
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
  var l: dynamic = 0;
  var r: dynamic = len;
  var ans: dynamic = 0;
  while (((r - l) >= 0))
  {
    var mid: dynamic = (((l + r)) / 2);
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
