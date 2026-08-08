// Translated from solution.cpp.

func max(a: dynamic, b: dynamic)
{
  return if (((a > b))) a else b;
}

func min(a: dynamic, b: dynamic)
{
  return if (((a < b))) a else b;
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  if (fopen("input.txt", "r"))
  {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
  }
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var k: dynamic;
  var i: dynamic;
  var x: dynamic;
  var y: dynamic;
  read(k);
  var a = cpp_array(k);
  {
    i = 0;
    while ((i < k))
    {
      read(a[i].first, a[i].second);
      i += 1;
    }
  }
  var ans = -1;
  var j: dynamic;
  var l: dynamic;
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= m))
        {
          var tmp = 4000;
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
