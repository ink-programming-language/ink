// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var i: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < n))
    {
      read(a[i], b[i]);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    i = (n - 1);
    while ((i >= 0))
    {
      a[i] += ans;
      if (((a[i] % b[i]) != 0))
      {
        ans += (b[i] - (a[i] % b[i]));
      }
      i -= 1;
    }
  }
  write(ans);
}
