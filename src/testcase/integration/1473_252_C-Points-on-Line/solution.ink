// Translated from solution.cpp.

var a = cpp_array(100005);

func main()
{
  var n: dynamic;
  var d: dynamic;
  read(n, d);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var l = 1;
  var r = 2;
  var ans = 0;
  while (((l <= n) || (r <= n)))
  {
    if ((l >= r))
    {
      r = (l + 1);
      continue;
    }
    if ((r > n))
    {
      ans += max(0, (((((r - l) - 1)) * (((r - l) - 2))) / 2));
      r -= 1;
      l += 1;
      continue;
    }
    if (((a[r] - a[l]) <= d))
    {
      r += 1;
    } else
    {
      ans += max(0, (((((r - l) - 1)) * (((r - l) - 2))) / 2));
      r -= 1;
      l += 1;
    }
  }
  write(ans, "\n");
}
