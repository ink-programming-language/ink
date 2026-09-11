// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(val[i]);
      i += 1;
    }
  }
  sort(val.begin(), val.end());
  if ((n == 1))
  {
    write(val[0]);
    return 0;
  }
  var ans: dynamic = val[0];
  {
    var i: dynamic = 1;
    while ((i < (n - 1)))
    {
      if ((val[i] >= 0))
      {
        ans -= val[i];
      }
      i += 1;
    }
  }
  ans = (val.back() - ans);
  {
    var i: dynamic = 1;
    while ((i < (n - 1)))
    {
      if ((val[i] < 0))
      {
        ans -= val[i];
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
