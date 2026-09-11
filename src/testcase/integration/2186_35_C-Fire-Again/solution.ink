// Translated from solution.cpp.

func max(a: dynamic, b: dynamic) -> dynamic
{
  return  (((a > b))) ? a : b;
}

func min(a: dynamic, b: dynamic) -> dynamic
{
  return  (((a < b))) ? a : b;
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  if (fopen("input.txt", "r"))
  {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
  }
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var k: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(k);
  var a: dynamic = cpp_array(k);
  {
    i = 0;
    while ((i < k))
    {
      read(a[i].first, a[i].second);
      i += 1;
    }
  }
  var ans: dynamic = -1;
  var j: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= m))
        {
          var tmp: dynamic = 4000;
          {
            l = 0;
            while ((l < k))
            {
              tmp = min(tmp, (abs((i - a[l].first)) + abs((j - a[l].second))));
              l += 1;
            }
          }
          if ((ans < tmp))
          {
            ans = tmp;
            x = i;
            y = j;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(x, " ", y);
  return 0;
}
