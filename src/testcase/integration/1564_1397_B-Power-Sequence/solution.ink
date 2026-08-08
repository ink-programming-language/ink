// Translated from solution.cpp.

func solve(a: dynamic, n: dynamic)
{
  var big = cpp_cast(1e15);
  var ans = big;
  var c = 1;
  var k = 0;
  while (1)
  {
    var val = 0;
    var pw = 1;
    {
      var i = 0;
      while ((i < n))
      {
        if ((pw >= big))
        {
          val = -1;
          break;
        }
        val += abs((pw - a[i]));
        i += 1;
        pw *= c;
      }
    }
    if ((val == -1))
    {
      break;
    }
    ans = min(ans, val);
    c += 1;
  }
  write(ans);
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  sort(arr.begin(), arr.end());
  solve(arr, n);
  return 0;
}
