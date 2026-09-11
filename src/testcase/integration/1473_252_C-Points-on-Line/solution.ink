// Translated from solution.cpp.

var a: dynamic = cpp_array(100005);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(n, d);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var l: dynamic = 1;
  var r: dynamic = 2;
  var ans: dynamic = 0;
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
